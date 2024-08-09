#include <SPI.h>

// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 50
#define SS_NANO2 51
#define SS_NANO3 52

// Define Button Pins
#define BUTTON1 4
#define BUTTON2 5
#define BUTTON3 6

void setup() {
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
}

void loop() {
  // Check Button 1 and control Nano 1 LED
  if (digitalRead(BUTTON1) == LOW) {
    controlNanoLED(SS_NANO1, HIGH);  // Turn on LED on Nano 1
  } else {
    controlNanoLED(SS_NANO1, LOW);   // Turn off LED on Nano 1
  }
  
  // Check Button 2 and control Nano 2 LED
  if (digitalRead(BUTTON2) == LOW) {
    controlNanoLED(SS_NANO2, HIGH);  // Turn on LED on Nano 2
  } else {
    controlNanoLED(SS_NANO2, LOW);   // Turn off LED on Nano 2
  }
  
  // Check Button 3 and control Nano 3 LED
  if (digitalRead(BUTTON3) == LOW) {
    controlNanoLED(SS_NANO3, HIGH);  // Turn on LED on Nano 3
  } else {
    controlNanoLED(SS_NANO3, LOW);   // Turn off LED on Nano 3
  }
}

void controlNanoLED(int ssPin, int ledState) {
  digitalWrite(ssPin, LOW);            // Select the Nano
  SPI.transfer(ledState);              // Send LED state (HIGH or LOW)
  digitalWrite(ssPin, HIGH);           // Deselect the Nano
}
