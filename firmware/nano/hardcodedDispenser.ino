/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/11/2024
 * 
 * File Name   : hardcodedDispenser.ino
 * Purpose     : 
 * 
 */

#include <Stepper.h>
#include <Servo.h>

const int buttonPin = 3;    
const int ledPin = 9; 
const int irSensorPin = 4; 
const int servoPin1 = 5;
const int servoPin2 = 6;

volatile bool ledState = LOW;  

// debounce prevents the button from being spammed
const unsigned long debounceDelay = 250; // in ms
volatile unsigned long lastDebounceTime = 0;  

int resistorCount = 0; // count of resistors dispensed
int requestedResistorCount = 1; // count of resistors requested, hardcoded to 1 for now

int stepsPerRevolution = 2048; // can be found in the motor's datasheet (this is for a 28BYJ-48 motor)
const int revolutionsPerResistor = 1; // number of revolutions needed to dispense one resistor (Need to be calibrated)
int motSpeed = 10; // speed in rpm 

// IN1 to A4 (18), IN2 to A3 (17), IN3 to A2 (16), IN4 to A1 (15)
Stepper myStepper(stepsPerRevolution, 18, 16, 17, 15);

Servo myServo1;
Servo myServo2;
int servo1Pos = 0; 
int servo2Pos = 180;

enum DispenserState {
  IDLE,
  REELING,
  CUTTING,
  COUNTING,
  COMPLETE
};

DispenserState dispenserState = IDLE; // set initial state 

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  
  pinMode(irSensorPin, INPUT); 
  pinMode(ledPin, OUTPUT);  

  myServo1.attach(servoPin1);
  myServo2.attach(servoPin2);    

  // set the speed of the motor
  myStepper.setSpeed(motSpeed);

  // calling ISR on negedge of button press
  attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

}

void loop() {
    switch (dispenserState)
    {
    case IDLE:
      // do nothing, wait for button press
      break;
    case REELING:
       reeling();
       break;
    case COUNTING:
        counting();
        break;
    case CUTTING:
       cutting();
       break;
    case COMPLETE:
       complete();
       break;
    default:
        break;
    }
}

// Interrupt Service Routine (ISR) for the button press
void pushButtonISR() {
  unsigned long currentTime = millis();

  // if the current time is larger than the debounce delay, update the state to start reeling
  if (currentTime - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentTime; // update debounce
    if (dispenserState == IDLE) {
      dispenserState = REELING;
    }
  }
}

void reeling() {
  // move the motor enough for one resistor
  myStepper.step(stepsPerRevolution * revolutionsPerResistor);
  dispenserState = COUNTING;
}

void counting() {
  int sensorStatus = digitalRead(irSensorPin);
   if (sensorStatus == HIGH) // read sensor status
   {
    resistorCount++;
   }
   // now check if enough resistors have been reeled out
    if (resistorCount == requestedResistorCount) {
     dispenserState = CUTTING;
    }
    else {
      dispenserState = REELING;
    }
}

void cutting() {
  // move the servos in opposite directions to cut the resistor
    while (servo1Pos < 90 && servo2Pos > 90) {
      myServo1.write(servo1Pos);
      myServo2.write(servo2Pos);
      servo1Pos++;
      servo2Pos--;
    }
    dispenserState = COMPLETE;
}

void complete() {
  // reset the servo positions
   servo1Pos = 0;
   servo2Pos = 180;
  // reset the resistor count
  resistorCount = 0;
  // reset the dispenser state
  dispenserState = IDLE;
}