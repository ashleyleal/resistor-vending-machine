#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

// KEYPAD
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte rowPins[ROWS] = {37, 35, 33, 31};
byte colPins[COLS] = {29, 27, 25, 23};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String textBuffer = "";
char key = '\0';  // Initialize key with '\0' (null character)
char verifiedKey = '\0';  // Initialize verifiedKey with '\0' (null character)

// LCD
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

// define states for the master FSM
enum MasterState {
    MS_IDLE,
    SELECT_RESISTOR,
    CONFIRM_SELECTION,
    SELECT_QUANTITY,
    DISPENSE_SIGNAL,
    COMPLETE
};

MasterState masterState = MS_IDLE;  // set initial state of the master

enum SelectedResistor {
    RESISTOR_A,
    RESISTOR_B,
    RESISTOR_C,
    RESISTOR_D,
    NONE
};

SelectedResistor selectedResistor = NONE;
int resistorQuantity = 0;

void sweepLCD(String text, int row, int delayTime = 100) {
    Serial.println(text);
    int len = text.length();  
    if (len <= 16) {  // If text fits on the screen, just print it
        lcd.setCursor(0, row);   
        lcd.print(text); 
    } else {  // Text is longer than 16 characters, sweep it
        for (int i = 0; i <= len - 16; i++) {
            lcd.setCursor(0, row);
            lcd.print(text.substring(i, i + 16));  // Display 16 characters at a time
            delay(delayTime);  // Pause for a short period to create the sweeping effect
        }
    }
}

void setup() {
  Serial.begin(9600);

  // LCD SETUP
  lcd.init();           // Initialize the LCD
  lcd.clear();          // Clear the display
  lcd.backlight();      // Turn on the backlight
  lcd.setCursor(0, 0);  // Set cursor to the beginning

  // SPI SETUP
  SPI.begin();
  // Set SS pins as OUTPUT and set them HIGH because SS is active low
  pinMode(SS_NANO1, OUTPUT);
  pinMode(SS_NANO2, OUTPUT);
  pinMode(SS_NANO3, OUTPUT);
  pinMode(SS_NANO4, OUTPUT);
  digitalWrite(SS_NANO1, HIGH);
  digitalWrite(SS_NANO2, HIGH);
  digitalWrite(SS_NANO3, HIGH);
  digitalWrite(SS_NANO4, HIGH);
}

void loop() {
    switch(masterState) {
        case MS_IDLE:
            idle();
            break;
        case SELECT_RESISTOR:
            selectResistor();
            break;
        case CONFIRM_SELECTION:
            confirmSelection();
            break;
        case SELECT_QUANTITY:
            selectQuantity();
            break;
        case DISPENSE_SIGNAL:
            dispenseSignal();
            break;
        case COMPLETE:
            complete();
            break;
    }
}

void idle() {
    sweepLCD("Welcome to the IEEE Resistor Vending Machine", 0);
    sweepLCD("Choose A-D", 1);
    key = keypad.getKey();
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '*') {
    
        masterState = SELECT_RESISTOR;
    }
}

void selectResistor() {
    // set selectedResistor
    switch (key) {
        case 'A':
            selectedResistor = RESISTOR_A;
            break;
        case 'B':
            selectedResistor = RESISTOR_B;
            break;
        case 'C':
            selectedResistor = RESISTOR_C;
            break;
        case 'D':
            selectedResistor = RESISTOR_D;
            break;
        case '*':
            masterState = MS_IDLE;  // go back to IDLE state
            selectedResistor = NONE;
            return;
    }
    masterState = CONFIRM_SELECTION;
}

void confirmSelection() {
    // print selected resistor and quantity
    sweepLCD("You have selected dispenser " + String(key), 0);
    sweepLCD("Press '#' to confirm or '*' to choose again", 1);
    
    key = keypad.getKey();
    if (key == '#') {
        masterState = SELECT_QUANTITY;
    } else if (key == '*') {
        masterState = MS_IDLE;
    }
}

void selectQuantity() {
    // prompt quantity (4-10)
    sweepLCD("How many resistors do you need?", 0);
    sweepLCD("Enter a number between 4 and 10 and press '#' to continue", 1);

    key = keypad.getKey();

    if (isDigit(key) || key == '*') {
        handleBuffer();
    }

    if (key == '#') {
        if (verifyQuantity(4, 10)) {
            masterState = DISPENSE_SIGNAL;
        } else {
            sweepLCD("Invalid quantity. Please enter a number between 4 and 10", 0);
            
            delay(2000);

            sweepLCD("How many resistors do you need?", 0);
            sweepLCD("Enter a number between 4 and 10 and press '#' to continue", 1);
        }
    }
}

void dispenseSignal() {
    // send signal to selected Nano and print resistor value 
    switch (selectedResistor) {
        case RESISTOR_A:
            sendSignal(SS_NANO1, resistorQuantity);
            break;
        case RESISTOR_B:
            sendSignal(SS_NANO2, resistorQuantity);
            break;
        case RESISTOR_C:
            sendSignal(SS_NANO3, resistorQuantity);
            break;
        case RESISTOR_D:
            sendSignal(SS_NANO4, resistorQuantity);
            break;
    }
    masterState = COMPLETE;
}

void complete() {
    sweepLCD("Dispensing " + String(resistorQuantity) + " resistors", 0);
    sweepLCD("Thank you for using the IEEE Resistor Vending Machine! :D", 1);
    delay(5000);
    masterState = MS_IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0; 
}

void sendSignal(int ssPin, int quantity) {
    digitalWrite(ssPin, LOW);            // Select the Nano
    SPI.transfer(quantity);              // Send LED state (HIGH or LOW)
    digitalWrite(ssPin, HIGH);           // Deselect the Nano
}

void handleBuffer() {
    if (key == '*') { 
        textBuffer = "";  // Clear the text buffer if '*' is pressed
        lcd.clear();      // Clear the LCD display
    } else {
        textBuffer += key;  // Append the pressed key to the buffer
        sweepLCD(textBuffer, 1);
        Serial.print(textBuffer);
    }
}

bool verifyQuantity(int min, int max) {
    int quantity = textBuffer.toInt();
    if (quantity >= min && quantity <= max) {
        resistorQuantity = quantity;
        return true;
    } else {
        return false;
    }
}


