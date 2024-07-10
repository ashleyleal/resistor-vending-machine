/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/10/2024
 * 
 * File Name   : stepperUnitTest.ino
 * Purpose     : Test the stepper motor by moving it one full revolution in both directions
 * 
 */

#include <Stepper.h>

int stepsPerRevolution = 2048; // can be found in the motor's datasheet (this is for a 28BYJ-48 motor)
int motSpeed = 10; // speed in rpm 
int dt = 500; // delay time in ms

// note: on the Arduino Nano, analog pins used as digital pins are numbered from 14 to 19 (A0 to A5)

// IN1 to A4 (18), IN2 to A3 (17), IN3 to A2 (16), IN4 to A1 (15)
Stepper myStepper(stepsPerRevolution, 18, 16, 17, 15);

void setup() {
  // set the speed of the motor
  myStepper.setSpeed(motSpeed);
}

void loop() {
  // move the motor one full revolution in one direction
  myStepper.step(stepsPerRevolution);
  delay(dt);
  // move the motor one full revolution in the opposite direction
  myStepper.step(-stepsPerRevolution);
  delay(dt);
}