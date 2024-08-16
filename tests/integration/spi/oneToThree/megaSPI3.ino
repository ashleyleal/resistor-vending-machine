#include <Keypad.h>
#include <SPI.h>

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
int redPin = 13;    // Red RGB pin -> D13
int greenPin = 12;  // Green RGB pin -> D12
int bluePin = 11;   // Blue RGB pin -> D11

volatile bool ledStateNano1 = LOW;
volatile bool ledStateNano2 = LOW;
volatile bool ledStateNano3 = LOW;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);

  // Initialize SPI
  SPI.begin();

  // Set SS pins as OUTPUT and set them HIGH
  pinMode(SS_NANO1, OUTPUT);
  pinMode(SS_NANO2, OUTPUT);
  pinMode(SS_NANO3, OUTPUT);
  digitalWrite(SS_NANO1, HIGH);
  digitalWrite(SS_NANO2, HIGH);
  digitalWrite(SS_NANO3, HIGH);

  // Set RGB LED pins as OUTPUT
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Start with white color (all LEDs on)
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
  delay(500);

  Serial.println("Setup complete.");
}

void loop() {
  char key = keypad.getKey(); // Read the key

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);  // Process the key input to control the LEDs
  }
}

void handleKey(char key) {
  switch (key) {
    case 'A': // Toggle LED on Nano 1 (RED) and flash RED on RGB
      ledStateNano1 = !ledStateNano1;
      controlNanoLED(SS_NANO1, ledStateNano1);
      flashColor(redPin, greenPin, bluePin, HIGH, LOW, LOW);  // Flash RED
      break;

    case 'B': // Toggle LED on Nano 2 (BLUE) and flash BLUE on RGB
      ledStateNano2 = !ledStateNano2;
      controlNanoLED(SS_NANO2, ledStateNano2);
      flashColor(redPin, greenPin, bluePin, LOW, LOW, HIGH);  // Flash BLUE
      break;

    case 'C': // Toggle LED on Nano 3 (YELLOW) and flash YELLOW on RGB
      ledStateNano3 = !ledStateNano3;
      controlNanoLED(SS_NANO3, ledStateNano3);
      flashColor(redPin, greenPin, bluePin, HIGH, HIGH, LOW); // Flash YELLOW (Red + Green)
      break;

    case '*': // Clear buffer or any other desired action
      Serial.println("Buffer Cleared");
      break;

    default: // Handle other keys if necessary
      break;
  }
}

void controlNanoLED(int ssPin, int ledState) {
  digitalWrite(ssPin, LOW);            // Select the Nano
  SPI.transfer(ledState);              // Send LED state (HIGH or LOW)
  digitalWrite(ssPin, HIGH);           // Deselect the Nano

  Serial.print("Control LED on Nano with SS Pin ");
  Serial.print(ssPin);
  Serial.print(". LED state set to: ");
  Serial.println(ledState ? "ON" : "OFF");
}

void flashColor(int redPin, int greenPin, int bluePin, int redState, int greenState, int blueState) {
  // Set the RGB LED to the specified color
  digitalWrite(redPin, redState);
  digitalWrite(greenPin, greenState);
  digitalWrite(bluePin, blueState);

}
