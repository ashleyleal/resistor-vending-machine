#include <SPI.h>

// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42

// Define Button Pins
#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 18

volatile bool ledStateNano1 = LOW;
volatile bool ledStateNano2 = LOW;
volatile bool ledStateNano3 = LOW;

const unsigned long debounceDelay = 250; // in ms
volatile unsigned long lastDebounceTimeNano1 = 0;
volatile unsigned long lastDebounceTimeNano2 = 0;
volatile unsigned long lastDebounceTimeNano3 = 0;

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
  
  // Set button pins as INPUT with internal pull-up resistors
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  
  // Attach interrupts to buttons
  attachInterrupt(digitalPinToInterrupt(BUTTON1), pushButtonISR1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), pushButtonISR2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON3), pushButtonISR3, FALLING);

  Serial.println("Setup complete.");
}

void loop() {
  // Main loop does nothing, everything handled by ISRs
}

// ISR for Button 1
void pushButtonISR1() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTimeNano1 > debounceDelay) {
    ledStateNano1 = !ledStateNano1;
    controlNanoLED(SS_NANO1, ledStateNano1); // Toggle LED on Nano 1
    lastDebounceTimeNano1 = currentTime;

    Serial.print("Button 1 pressed. Sending LED state: ");
    Serial.println(ledStateNano1 ? "ON" : "OFF");
  }
}

// ISR for Button 2
void pushButtonISR2() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTimeNano2 > debounceDelay) {
    ledStateNano2 = !ledStateNano2;
    controlNanoLED(SS_NANO2, ledStateNano2); // Toggle LED on Nano 2
    lastDebounceTimeNano2 = currentTime;

    Serial.print("Button 2 pressed. Sending LED state: ");
    Serial.println(ledStateNano2 ? "ON" : "OFF");
  }
}

// ISR for Button 3
void pushButtonISR3() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTimeNano3 > debounceDelay) {
    ledStateNano3 = !ledStateNano3;
    controlNanoLED(SS_NANO3, ledStateNano3); // Toggle LED on Nano 3
    lastDebounceTimeNano3 = currentTime;

    Serial.print("Button 3 pressed. Sending LED state: ");
    Serial.println(ledStateNano3 ? "ON" : "OFF");
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
