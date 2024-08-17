/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 08/17/2024
 *
 * File Name   : controller.ino
 * Purpose     : This file contains the code for the master controller (Arduino Mega) of the IEEE Resistor Vending Machine.
 *               The master controller is responsible for handling user input, displaying information on the LCD, and sending 
 *               signals to the slave controllers (Arduino Nanos) to dispense resistors.
 *
 */

// This file may be used with dispenser.ino (main dispenser code) or LEDFlash.ino to test the SPI communication 
// between the master and slave controllers.

#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

/* KEYPAD CONFIG */
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

/* STATUS LED */
int redPin = 13;
int greenPin = 12;
int bluePin = 11;

/* LCD CONFIG */
LiquidCrystal_I2C lcd(0x3F, 16, 2);  // 16 chars x 2 lines display

/* SPI CONFIG (SS) */
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

/* LED STATES */
// (for testing SPI signal)
volatile bool ledStateNano1 = LOW;
volatile bool ledStateNano2 = LOW;
volatile bool ledStateNano3 = LOW;
volatile bool ledStateNano4 = LOW;

/* TIMEOUT CONFIG */
const unsigned long TIME_LIMIT = 300000; // 5 minutes in milliseconds

/* STATE DEFINITIONS FOR MASTER FSM */
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

/* ENUM FOR SELECTED RESISTOR */
enum SelectedResistor {
    RESISTOR_A,
    RESISTOR_B,
    RESISTOR_C,
    RESISTOR_D,
    NONE
};

SelectedResistor selectedResistor = NONE; // Initial state of the selected resistor

/* ENUM FOR LED COLORS */
enum LEDcolor {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    WHITE
};

int resistorQuantity = 0; // Store the quantity of resistors to dispense

/* GLOBAL VARIABLES */
String textBuffer = "";
char key = '\0';  // Initialize key with '\0' (null character)
char savedKey = '\0';  // Initialize savedKey with '\0' (null character)

/* TIMING CONFIG */
unsigned long lastActionTime = 0;
unsigned long previousMillis = 0; // Store the last time the display was updated
const int interval = 300; // Interval for updating the display (in milliseconds)

/* FUNCTION PROTOTYPES */
void sweepLCD(String text0, String text1);
void idle();
void selectResistor();
void confirmSelection();
void selectQuantity();
void dispenseSignal();
void complete();
void timeout();
void sendSignal(int ssPin, int quantity);
void controlNanoLED(int ssPin, int ledState);  
void handleBuffer();
bool verifyQuantity(int min, int max);
void reset();
void updateStatusLED(LEDcolor color);

void setup() {
    Serial.begin(9600);
    lcd.init();           
    lcd.clear();          
    lcd.backlight();      

    // Initialize SPI
    SPI.begin();

    //status LED
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    updateStatusLED(WHITE);
  
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
}

void sweepLCD(String text0, String text1) {
    static int i = 0; // Current position in the scrolling text
    int len0 = text0.length(); 
    int len1 = text1.length();
    int maxLen = max(len0, len1); // Maximum length of text0 and text1

    unsigned long currentMillis = millis(); // Get the current time

    // Implement based on timing instead of delays to allow for non-blocking behavior
    // Check if it's time to update the display (every interval milliseconds)
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
        // When in SELECT_QUANTITY state, always show the buffer
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
    updateStatusLED(WHITE);
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
            reset();
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
        reset();
    }
}

void selectQuantity() {
    lcd.setCursor(0,1);   // Set cursor to the beginning
    lcd.print(textBuffer); // Display the buffer on the LCD
    sweepLCD("Enter quantity between 4 and 10. Then press # to continue", textBuffer);

    handleBuffer(); // Handle the buffer based on keypress (add to buffer or clear)

    if (key == '#') {
        if (verifyQuantity(4, 10)) {
            resistorQuantity = textBuffer.toInt();
            masterState = DISPENSE_SIGNAL;
            lastActionTime = millis();  // Reset timer
        } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Invalid quantity!");
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
            // controlNanoLED(SS_NANO1, ledStateNano1);
            updateStatusLED(RED);
            break;
        case RESISTOR_B:
            sendSignal(SS_NANO2, resistorQuantity);
            // controlNanoLED(SS_NANO2, ledStateNano2);
            updateStatusLED(BLUE);
            break;
        case RESISTOR_C:
            sendSignal(SS_NANO3, resistorQuantity);
            // controlNanoLED(SS_NANO3, ledStateNano3);
            updateStatusLED(YELLOW);
            break;
        case RESISTOR_D:
            sendSignal(SS_NANO4, resistorQuantity);
            // controlNanoLED(SS_NANO4, ledStateNano4);
            updateStatusLED(GREEN);
            break;
    }
    masterState = COMPLETE;
}

void complete() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dispensing...");   
    delay(7000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Thank you! :D");
    delay(2000);
    reset();
}

void timeout() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout!");   
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Restarting...");   
    delay(3000);  // Show timeout message for 3 seconds
    reset();
}

void sendSignal(int ssPin, int quantity) {
    Serial.print("Sending" + String(quantity) + " to Dispenser " + String(ssPin) + "...");
    // Cannot just send the quantity as integer, must split into two bytes
    byte highByte = (quantity >> 8) & 0xFF;
    byte lowByte = quantity & 0xFF;

    digitalWrite(ssPin, LOW); // Select the dispenser

    // Send the two bytes over SPI
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(highByte);
    SPI.transfer(lowByte);
    SPI.endTransaction();

    // Deselect the dispenser
    digitalWrite(ssPin, HIGH);
}

void controlNanoLED(int ssPin, int ledState) {
  ledState = !ledState;  // Toggle the LED state
  digitalWrite(ssPin, LOW);
  SPI.transfer(ledState);
  digitalWrite(ssPin, HIGH);

  Serial.print("Control LED on Nano with SS Pin ");
  Serial.print(ssPin);
  Serial.print(". LED state set to: ");
  Serial.println(ledState ? "ON" : "OFF");
}

void handleBuffer() {
    if (key == '*') { // Clear the buffer when '*' is pressed
        textBuffer = "";        
        lcd.clear();
        Serial.println("Buffer Cleared");
    } else if (isDigit(key)) {
        textBuffer += key;  
        Serial.println(textBuffer);  
    }
}

bool verifyQuantity(int min, int max) {
    int quantity = textBuffer.toInt();
    return (quantity >= min && quantity <= max);
}


void reset() {
    // Reset all variables
    masterState = MS_IDLE;
    selectedResistor = NONE;
    resistorQuantity = 0;
    textBuffer = "";
    lcd.clear();
    lastActionTime = millis();  // Reset timer
}

void updateStatusLED(LEDcolor color) {
    switch (color) {
        case RED:
            digitalWrite(redPin, HIGH);
            digitalWrite(greenPin, LOW);
            digitalWrite(bluePin, LOW);
            break;
        case GREEN:
            digitalWrite(redPin, LOW);
            digitalWrite(greenPin, HIGH);
            digitalWrite(bluePin, LOW);
            break;
        case BLUE:
            digitalWrite(redPin, LOW);
            digitalWrite(greenPin, LOW);
            digitalWrite(bluePin, HIGH);
            break;
        case YELLOW:
            digitalWrite(redPin, HIGH);
            digitalWrite(greenPin, HIGH);
            digitalWrite(bluePin, LOW);
            break;
        case WHITE:
            digitalWrite(redPin, HIGH);
            digitalWrite(greenPin, HIGH);
            digitalWrite(bluePin, HIGH);
            break;
    }
}