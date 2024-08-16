#include <Keypad.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

// Keypad Configuration
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

// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42

// Define RGB LED Pins
int redPin = 13;
int greenPin = 12;
int bluePin = 11;

// LCD Configuration
LiquidCrystal_I2C lcd(0x3F, 16, 2);

volatile bool ledStateNano1 = LOW;
volatile bool ledStateNano2 = LOW;
volatile bool ledStateNano3 = LOW;

// Buffer to store key sequence
char keyBuffer[17];  // 16 characters + null terminator
int bufferIndex = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();

  pinMode(SS_NANO1, OUTPUT);
  pinMode(SS_NANO2, OUTPUT);
  pinMode(SS_NANO3, OUTPUT);
  digitalWrite(SS_NANO1, HIGH);
  digitalWrite(SS_NANO2, HIGH);
  digitalWrite(SS_NANO3, HIGH);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
  delay(500);

  // Initialize key buffer
  memset(keyBuffer, 0, sizeof(keyBuffer));

  Serial.println("Setup complete.");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);

    // Update and display the key buffer, excluding '*'
    if (key != '*') {
      updateBuffer(key);
    }
  }
}

void handleKey(char key) {
  lcd.clear();
  switch (key) {
    case 'A':
      ledStateNano1 = !ledStateNano1;
      controlNanoLED(SS_NANO1, ledStateNano1);
      flashColor(redPin, greenPin, bluePin, HIGH, LOW, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Nano 1");
      break;

    case 'B':
      ledStateNano2 = !ledStateNano2;
      controlNanoLED(SS_NANO2, ledStateNano2);
      flashColor(redPin, greenPin, bluePin, LOW, LOW, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Nano 2");
      break;

    case 'C':
      ledStateNano3 = !ledStateNano3;
      controlNanoLED(SS_NANO3, ledStateNano3);
      flashColor(redPin, greenPin, bluePin, HIGH, HIGH, LOW);
      lcd.setCursor(0, 0);
      lcd.print("Nano 3");
      break;

    case '*':
      lcd.setCursor(0, 0);
      lcd.print("Buffer Cleared");
      Serial.println("Buffer Cleared");
      clearBuffer();  // Clear the buffer when '*' is pressed
      break;

    default:
      lcd.setCursor(0, 0);
      lcd.print("Invalid Key");
      break;
  }
}

void updateBuffer(char key) {
  if (bufferIndex < 16) {
    keyBuffer[bufferIndex++] = key;
    keyBuffer[bufferIndex] = '\0';  // Null-terminate the string
  } else {
    // Shift the buffer left if it's full
    for (int i = 1; i < 16; i++) {
      keyBuffer[i-1] = keyBuffer[i];
    }
    keyBuffer[15] = key;
  }
  lcd.setCursor(0, 1);  // Set cursor to second line
  lcd.print(keyBuffer); // Display buffer
}

void clearBuffer() {
  memset(keyBuffer, 0, sizeof(keyBuffer)); // Clear buffer content
  bufferIndex = 0;  // Reset index
  lcd.setCursor(0, 1);
  lcd.print("                "); // Clear the second line on the LCD
}

void controlNanoLED(int ssPin, int ledState) {
  digitalWrite(ssPin, LOW);
  SPI.transfer(ledState);
  digitalWrite(ssPin, HIGH);

  Serial.print("Control LED on Nano with SS Pin ");
  Serial.print(ssPin);
  Serial.print(". LED state set to: ");
  Serial.println(ledState ? "ON" : "OFF");
}

void flashColor(int redPin, int greenPin, int bluePin, int redState, int greenState, int blueState) {
  digitalWrite(redPin, redState);
  digitalWrite(greenPin, greenState);
  digitalWrite(bluePin, blueState);
}
