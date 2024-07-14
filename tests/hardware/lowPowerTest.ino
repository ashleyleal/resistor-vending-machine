#include <avr/sleep.h>
#include <avr/interrupt.h> 


// define pins
const int buttonPin = 3;
const int ledPin = 9;

volatile bool ledState = LOW;  

// debounce prevents the button from being spammed
const unsigned long debounceDelay = 250; // in ms
volatile unsigned long lastDebounceTime = 0;  

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  
  pinMode(ledPin, OUTPUT);          

  // calling ISR on negedge of button press
  attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

  digitalWrite(ledPin, ledState);
}

void loop() {
    enterSleepMode();
}


// Interrupt Service Routine (ISR) for the button press
void pushButtonISR() {
  unsigned long currentTime = millis();

  // if the current time is larger than the debounce delay, toggle the LED
  if (currentTime - lastDebounceTime > debounceDelay) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    lastDebounceTime = currentTime;
  }
}

void enterSleepMode() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_disable();
}