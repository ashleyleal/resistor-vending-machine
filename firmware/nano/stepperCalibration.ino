#include <Stepper.h>

const int buttonPin = 3;
const int ledPin = 9;

// initialize variables
volatile bool ledState = LOW;
int resistorCount = 0;           // count of resistors dispensed
int requestedResistorCount = 1;  // count of resistors requested, hardcoded to 1 for now

// debounce prevents the push button from being spammed
const unsigned long debounceDelay = 250;  // in ms
volatile unsigned long lastDebounceTime = 0;

// stepper motor configuration
const int stepPin = 16;
const int dirPin = 15;
const int enablePin = 17;            // active low
const int microstepRes = 32;         // 1 / 32 microstep resolution
const int stepsPerRevolution = 200;  // from datasheet: 360 deg / 1.8 deg = 200
int stepsPerResistor = stepsPerRevolution * microstepRes * 0.405;

void setup() {
  // initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Setup started");

  // initialize pins
  pinMode(buttonPin, INPUT);  // set the button pin as input with pullup resistor to prevent floating
  pinMode(ledPin, OUTPUT);

  // stepper motor pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);  // stepper driver off to start
  digitalWrite(dirPin, LOW); // Enables the motor to move forward

  // calling ISR on negedge of button press
  attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

  // enable global interrupts
  sei();
}


void loop() {

}


void pushButtonISR() {
  unsigned long currentTime = millis();

  if (currentTime - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentTime;  // update debounce

    ledState = HIGH;
    digitalWrite(ledPin, ledState);

    digitalWrite(enablePin, LOW); 

    for(int x = 0; x < stepsPerResistor; x++) { // pulse stepper motor
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
    }

    digitalWrite(enablePin, HIGH); 

    ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
}

