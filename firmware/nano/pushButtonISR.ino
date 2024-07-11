/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/11/2024
 * 
 * File Name   : pushButtonISR.ino
 * Purpose     : Push button interrupt service routine (ISR) to toggle on-board LED
 * 
 */

const int buttonPin = 3;    
const int ledPin = 13; 

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