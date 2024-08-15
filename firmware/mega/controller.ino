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

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
String textBuffer = "";
char key = "";

// LCD
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

// define states for the master FSM
enum MasterState {
    IDLE,
    SELECT_RESISTOR,
    CONFIRM_SELECTION,
    SELECT_QUANTITY,
    DISPENSE_SIGNAL,
    COMPLETE
};

MasterState masterState = IDLE;  // set initial state of the master

enum SelectedResistor {
    A,
    B,
    C,
    D,
    NONE
};

SelectedResistor selectedResistor = NONE;
int resistorQuantity = 0;

void setup(){
  Serial.begin(9600);

  // LCD SETUP
  lcd.init();           // Initialize the LCD
  lcd.clear();          // Clear the display
  lcd.backlight();      // Turn on the backlight
  lcd.setCursor(0,0);   // Set cursor to the beginning
  
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

void loop(){
    switch(masterState) {
        case IDLE:
            idle();
            break;
        case SELECT_RESISTOR:
            selectResistor();
            break;
        case CONFIRM_SELECTION:
            confirmSelection();
            break;
        case SELECT_QUANTITY:

            break;
        case DISPENSE_SIGNAL:
            dispenseSignal();
            break;
        case COMPLETE:

            break;
    }
}

void idle() {
    printLCD("Welcome to the IEEE Resistor Vending Machine", 0);
    printLCD("Please select a value (A-D)", 1);
    key = keypad.getKey();
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '*') {
        masterState = SELECT_RESISTOR;
    }
}

void selectResistor() {
    // set selectedResistor
    switch (key)
    {
    case 'A':
        selectedResistor = A;
        break;
    case 'B':
        selectedResistor = B;
        break;
    case 'C':
        selectedResistor = C;
        break;
    case 'D':
        selectedResistor = D;
        break;
    case '*':
        masterState = IDLE; // go back to IDLE state
        selectedResistor = NONE;
        break;
    }
    masterState = CONFIRM_SELECTION;
}

void confirmSelection() {
    // print selected resistor and quantity
    printLCD("You have selected dispenser" + selectedResistor, 0);
    printLCD("Press '#' to confirm or '*' to choose again", 1);
    
    key = keypad.getKey();
    if (key == '#') {
        masterState = SELECT_QUANTITY;
    } else if (key == '*') {
        masterState = IDLE;
    }
}

void selectQuantity() {
    // prompt quantity (5-10)
    printLCD("How many resistors do you need?", 0);
    printLCD("Enter a number between 4 and 10 and press '#' to continue", 1);

    key = keypad.getKey();

    if (isDigit(key) || key == '*') {
        handle_buffer();
    }

    if (key == '#') {
        if (verifyQuantity(4, 10)) {
            masterState = DISPENSE_SIGNAL;
        } else {
            printLCD("Invalid quantity. Please enter a number between 4 and 10", 0);
            
            delay(2000);

            printLCD("How many resistors do you need?", 0);
            printLCD("Enter a number between 4 and 10 and press '#' to continue", 1);
        }
    }
}

void dispenseSignal() {

    // send signal to selected Nano and print resistor value 
    switch (selectedResistor)
    {
    case A:
        sendSignal(SS_NANO1, selectQuantity);
        break;
    case B:
        sendSignal(SS_NANO2, selectQuantity);
        break;
    case C:
        sendSignal(SS_NANO3, selectQuantity);
        break;
    case D:
        sendSignal(SS_NANO4, selectQuantity);
        break;
    }
    masterState = COMPLETE;
}

void complete() {
    printLCD("Dispensing " + resistorQuantity + " resistors", 0);
    printLCD("Thank you for using the IEEE Resistor Vending Machine! :D", 1);
    delay(5000);
    masterState = IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0; 
}

void sendSignal(int ssPin, int quantity) {
    digitalWrite(ssPin, LOW);            // Select the Nano
    SPI.transfer(quantity);              // Send LED state (HIGH or LOW)
    digitalWrite(ssPin, HIGH);           // Deselect the Nano
}

void printLCD(String text, int row) {
    lcd.clear();  
    lcd.setCursor(0, row);   
    lcd.print(text); 
}

void handleBuffer() {
  if (key == '*') { 
    textBuffer = "";  // Clear the text buffer if '*' is pressed
    lcd.clear();      // Clear the LCD display
  } else {
    textBuffer += key;  // Append the pressed key to the buffer
    printLCD(textBuffer, 1);
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
