#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

// KEYPAD CONFIGURATION
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {37, 35, 33, 31};
byte colPins[COLS] = {29, 27, 25, 23};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD CONFIGURATION
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // 16 chars x 2 lines display

// SPI CONFIGURATION - Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

// STATE DEFINITIONS FOR MASTER FSM
enum MasterState {
    MS_IDLE,
    SELECT_RESISTOR,
    CONFIRM_SELECTION,
    SELECT_QUANTITY,
    DISPENSE_SIGNAL,
    COMPLETE
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

// FUNCTION PROTOTYPES
void sweepLCD(String text, int row, int delayTime = 200);
void idle();
void selectResistor();
void confirmSelection();
void selectQuantity();
void dispenseSignal();
void complete();
void sendSignal(int ssPin, int quantity);
void handleBuffer();
bool verifyQuantity(int min, int max);

void setup() {
    Serial.begin(9600);
    lcd.init();           
    lcd.clear();          
    lcd.backlight();      
    lcd.setCursor(0, 0);

    SPI.begin();
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
    Serial.print("Key Pressed: ");
    Serial.println(key);  // Debugging: Print key to Serial Monitor
    lcd.clear();  // Clear LCD display

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
    }
}

void sweepLCD(String text, int row, int delayTime) {
    Serial.println(text);  // Debugging: Print text to Serial Monitor
    int len = text.length();  
    if (len <= 16) {  
        lcd.setCursor(0, row);   
        lcd.print(text); 
    } else {  
        for (int i = 0; i <= len - 16; i++) {
            lcd.setCursor(0, row);
            lcd.print(text.substring(i, i + 16));
            delay(delayTime);  
            key = keypad.getKey();  
            if (key != '\0') {  
                break;
            }
        }
    }
}

void idle() {
    sweepLCD("Welcome to the IEEE Resistor Vending Machine", 0);
    sweepLCD("Choose A-D", 1);
    
    if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '*') {
        savedKey = key;
        masterState = SELECT_RESISTOR;
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
}

void confirmSelection() {
    sweepLCD("You have selected dispenser " + String(savedKey), 0);
    sweepLCD("Press '#' to confirm or '*' to choose again", 1);
    
    if (key == '#') {
        masterState = SELECT_QUANTITY;
    } else if (key == '*') {
        masterState = MS_IDLE;
    }
}

void selectQuantity() {
    lcd.clear();
    sweepLCD("How many resistors do you need?", 0);
    delay(2000);
    lcd.clear();
    sweepLCD("Enter a number between 4 and 10", 0);
    
    handleBuffer();

    if (key == '#') {
        if (verifyQuantity(4, 10)) {
            masterState = DISPENSE_SIGNAL;
        } else {
            sweepLCD("Invalid quantity. Enter a number between 4 and 10", 0);
            delay(2000);
            textBuffer = "";  // Clear buffer after invalid entry
            sweepLCD("How many resistors do you need?", 0);
            sweepLCD("Enter a number between 4 and 10", 1);
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
    sweepLCD("Dispensing " + String(resistorQuantity) + " resistors", 0);
    sweepLCD("Thank you for using the IEEE Resistor Vending Machine! :D", 1);
    delay(5000);
    masterState = MS_IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0; 
}

void sendSignal(int ssPin, int quantity) {
    digitalWrite(ssPin, LOW);  
    SPI.transfer(quantity);  
    digitalWrite(ssPin, HIGH);
}

void handleBuffer() {
    if (key == '*') { 
        textBuffer = "";  
        lcd.clear();      
    } else if (isDigit(key)) {
        textBuffer += key;  
        sweepLCD(textBuffer, 1);
        Serial.print("Text Buffer: ");
        Serial.println(textBuffer);  // Debugging: Print buffer to Serial Monitor
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
