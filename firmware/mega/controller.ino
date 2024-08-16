#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

// KEYPAD CONFIGURATION

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {31, 29, 27, 25};
byte colPins[COLS] = {39, 37, 35, 33};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD CONFIGURATION
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // 16 chars x 2 lines display

// SPI CONFIGURATION - Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

// TIMEOUT CONFIGURATION
const unsigned long TIME_LIMIT = 300000; // 5 minutes in milliseconds

// STATE DEFINITIONS FOR MASTER FSM
enum MasterState {
    MS_IDLE,
    SELECT_RESISTOR,
    CONFIRM_SELECTION,
    SELECT_QUANTITY,
    DISPENSE_SIGNAL,
    COMPLETE,
    TIMEOUT
};

MasterState masterState = MS_IDLE;  // Initial state of the master

// ENUM FOR SELECTED RESISTOR
enum SelectedResistor {
    RESISTOR_A,
    RESISTOR_B,
    RESISTOR_C,
    RESISTOR_D,
    NONE
};

SelectedResistor selectedResistor = NONE;
int resistorQuantity = 0;

// BUFFER VARIABLES
String textBuffer = "";
char key = '\0';  // Initialize key with '\0' (null character)
char savedKey = '\0';  // Initialize savedKey with '\0' (null character)

// TIMING VARIABLES
unsigned long lastActionTime = 0;
unsigned long previousMillis = 0; // Store the last time the display was updated
const int interval = 300; // Interval for updating the display (in milliseconds)

// FUNCTION PROTOTYPES
void sweepLCD(String text0, String text1);
void idle();
void selectResistor();
void confirmSelection();
void selectQuantity();
void dispenseSignal();
void complete();
void timeout();
void sendSignal(int ssPin, int quantity);
void handleBuffer();
bool verifyQuantity(int min, int max);

void setup() {
    Serial.begin(9600);
    lcd.init();           
    lcd.clear();          
    lcd.backlight();      

    // Initialize SPI
    SPI.begin();
  
    // Set SS pins as OUTPUT and set them HIGH
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
    key = keypad.getKey();  // Get keypress from keypad

    if (key != '\0') {
        lastActionTime = millis();  // Reset timer
        Serial.print("Key Pressed: ");
        Serial.println(key);
    }

    // Check for timeout
    if (masterState != MS_IDLE && millis() - lastActionTime > TIME_LIMIT) {
        masterState = TIMEOUT;
    }

    switch (masterState) {
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
        case TIMEOUT:
            timeout();
            break;
    }
    
    // Update LCD display
    sweepLCD(" ", " ");  // Pass empty strings to avoid blocking
}

void sweepLCD(String text0, String text1) {
    static int i = 0; // Current position in the scrolling text
    int len0 = text0.length();
    int len1 = text1.length();
    int maxLen = max(len0, len1); // Maximum length of text0 and text1

    unsigned long currentMillis = millis();

    // Check if it's time to update the display
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Save the last update time

        lcd.setCursor(0, 0);
        if (i <= len0 - 16) {
            lcd.print(text0.substring(i, i + 16));
        } else if (i <= len0) {
            lcd.print(text0.substring(len0 - 16));
        } else {
            lcd.print("                "); // Blank line
        }

        lcd.setCursor(0, 1);
        if (masterState != SELECT_QUANTITY) {
            if (i <= len1 - 16) {
                lcd.print(text1.substring(i, i + 16));
            } else if (i <= len1) {
                lcd.print(text1.substring(len1 - 16));
            } else {
                lcd.print("                "); // Blank line
            }
        } else {
            lcd.print(textBuffer);  // Always show the buffer
        }

        // Increment the position and reset if it goes beyond the text length
        i++;
        if (i > maxLen - 16) {
            i = 0;
        }
    }
}

void idle() {
    sweepLCD("Welcome to the IEEE Resistor Vending Machine!", "Please select a value (A-D)");
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '*') {
        savedKey = key;
        masterState = SELECT_RESISTOR;
        lastActionTime = millis();  // Reset timer
        key = '\0';  // Reset key to null character
    }
}

void selectResistor() {
    switch (savedKey) {
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
            masterState = MS_IDLE;
            selectedResistor = NONE;
            return;
    }
    masterState = CONFIRM_SELECTION;
    lastActionTime = millis();  // Reset timer
}

void confirmSelection() {
    sweepLCD("You selected dispenser " + String(savedKey), "Press # to confirm or * to choose again");
    if (key == '#') {
        masterState = SELECT_QUANTITY;
        lastActionTime = millis();  // Reset timer
        lcd.clear();
    } else if (key == '*') {
        masterState = MS_IDLE;
        selectedResistor = NONE;
    }
}

void selectQuantity() {
    lcd.setCursor(0,1);   // Set cursor to the beginning
    lcd.print(textBuffer); // Display the buffer on the LCD
    sweepLCD("Enter quantity between 4 and 10. Then press # to continue", textBuffer);

    handleBuffer();

    if (key == '#') {
        if (verifyQuantity(4, 10)) {
            masterState = DISPENSE_SIGNAL;
            lastActionTime = millis();  // Reset timer
        } else {
            sweepLCD("Invalid quantity!", "");
            delay(2000);
            textBuffer = "";  // Clear buffer after invalid entry
            lcd.clear();
        }
    }
}

void dispenseSignal() {
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
    sweepLCD("Dispensing " + String(resistorQuantity) + " resistors", "Thank you for using the IEEE Resistor Vending Machine!");
    delay(5000);
    masterState = MS_IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0; 
}

void timeout() {
    sweepLCD("Session timed out!", "Restarting...");
    delay(3000);  // Show timeout message for 3 seconds
    masterState = MS_IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0;
    lastActionTime = millis();  // Reset timer
    lcd.clear();
}

void sendSignal(int ssPin, int quantity) {
    digitalWrite(ssPin, LOW);  
    SPI.transfer(quantity);  
    digitalWrite(ssPin, HIGH);
}

void handleBuffer() {
    if (key == '*') { 
        textBuffer = "";        
    } else if (isDigit(key)) {
        textBuffer += key;  
        Serial.println(textBuffer);  // Debugging: Print buffer to Serial Monitor
    }
}

bool verifyQuantity(int min, int max) {
    int quantity = textBuffer.toInt();
    return (quantity >= min && quantity <= max);
}
