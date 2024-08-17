/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 08/17/2024
 *
 * File Name   : dispenser.ino
 * Purpose     : Dispenser firmware for the Resistor Vending Machine project. Loaded onto the Arduino Nano.
 *
 */

#include <Servo.h>
#include <Stepper.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <SPI.h>

/* STATUS LED */
volatile bool ledState = LOW;
const int ledPin = 9;

/* RESISTOR STATES */
int resistorCount = 0;           // count of resistors dispensed
int requestedResistorCount = 5;  // count of resistors requested, hardcoded to 5 for now

/* SPI COMMUNICATION */
#define SS_PIN 10  
volatile int receivedQuantity = 0; // Integer quantity received from the Master
volatile bool newQuantityReceived = false; // Flag to indicate a new quantity has been received

/* BUTTON DEBOUNCE CONFIG */
const int buttonPin = 3;
const unsigned long debounceDelay = 250;  // in ms
volatile unsigned long lastDebounceTime = 0;

/* STEPPER MOTOR CONFIG */
const int stepPin = 16;
const int dirPin = 15;
const int enablePin = 17;            // active low
const int microstepRes = 32;         // 1 / 32 microstep resolution
const int stepsPerRevolution = 200;  // from datasheet: 360 deg / 1.8 deg = 200
int stepsPerResistor = 2593;

/* SERVO MOTOR CONFIG */
const int servoPin1 = 5;
const int servoPin2 = 6;
Servo leftServo;
Servo rightServo;
int servoPos = 0;

/* STATE DEFINITIONS FOR THE DISPENSER FSM */
enum DispenserState {
  IDLE,
  REELING,
  CUTTING,
  COMPLETE
};

DispenserState dispenserState = IDLE;  // set initial state of the dispenser

void setup() {
  // initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Setup started");

  // initialize pins
  pinMode(buttonPin, INPUT);  // set the button pin as input with pullup resistor to prevent floating
  pinMode(ledPin, OUTPUT);

  leftServo.attach(servoPin1);
  rightServo.attach(servoPin2);

  leftServo.write(180);
  rightServo.write(0);

  // stepper motor pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);  // stepper driver off to start

  // calling ISR on negedge of button press
  attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

  // SPI configuration
  pinMode(MISO, OUTPUT); // Set MISO pin as output
  
  // Set MISO high by default
  digitalWrite(MISO, HIGH);

  // Initialize SPI in slave mode
  SPI.begin();  // Initialize SPI peripheral

  // Set SPI Slave select pin as input
  pinMode(SS_PIN, INPUT);
  
  // Enable SPI in Slave mode
  SPCR |= _BV(SPE);  // Enable SPI in Slave mode

  // Enable SPI interrupt
  SPCR |= _BV(SPIE);

  // Enable global interrupts
  sei();
  Serial.println("Setup complete");
}

// in the main loop, we will check the state of the dispenser and act accordingly

/*
Finite State Machine (FSM):  model used to design algorithms that can be in one of a finite number of states at any given time.
The FSM transitions from one state to another based on inputs or events, and it can perform specific actions during these transitions or while in a particular state.
*/

void loop() {

  switch (dispenserState) {
    case IDLE:
      // Serial.println("Entering sleep mode");
      // delay(100);
      // enterSleepMode();
      break;
    case REELING:
      reeling();
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

/*
Interrupt Service Routine (ISR): a callback function that is called when an interrupt occurs.
Interrupts are used to handle time-sensitive tasks, such as reading sensor data or responding to user input.
As opposed to polling, interrupts allow the microcontroller to respond immediately to events without wasting processing power.
*/

/*
Debounce: a technique used to prevent mechanical switches or buttons from generating multiple signals when pressed or released.
When a button is pressed, the contacts can bounce back and forth before settling into a stable state.
*/

// Interrupt Service Routine (ISR) for the button press
void pushButtonISR() {
  unsigned long currentTime = millis();

  // if the current time is larger than the debounce delay, update the state to start reeling
  if (currentTime - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentTime;  // update debounce
    if (dispenserState == IDLE) {

      // toggle the LED
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      Serial.println("Button pressed, waking up");
      dispenserState = REELING;

    } 
    
    // else if (dispenserState == REELING) {
    //   Serial.println("Button pressed, stopping reeling, starting cutting");
    //   dispenserState = CUTTING;
    // }
  }
}

ISR(SPI_STC_vect) {
  static byte highByte = 0;
  static byte lowByte = 0;
  static byte byteCount = 0;

  byte receivedData = SPDR;

  // Handle data reception
  if (byteCount == 0) {
    highByte = receivedData;
    byteCount++;
  } else if (byteCount == 1) {
    lowByte = receivedData;
    byteCount = 0;

    // Combine highByte and lowByte to reconstruct the integer
    receivedQuantity = (highByte << 8) | lowByte;
    newQuantityReceived = true;

    // Debug message
    Serial.print("SPI Interrupt: Received quantity: ");
    Serial.println(receivedQuantity);
  }

    if (receivedQuantity < 10 && receivedQuantity > 0 && dispenserState == IDLE) {
    flashLED(receivedQuantity);
    requestedResistorCount = receivedQuantity;
    dispenserState = REELING;
    newQuantityReceived = false; // Reset the flag
  }
}

// Function to put the Arduino into sleep mode
void enterSleepMode() {
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  sleep_enable();
  // allow wake up from pin change interrupt
  attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);
  sleep_mode();
  // execution goes here after wake up
  sleep_disable();
}

// The following functions are used to define the behavior of the dispenser in each state
// They also transition the dispenser to the next state based on the FSM

void reeling() {
  Serial.println("Reeling state");

  while (resistorCount < requestedResistorCount) {
    // move the motor enough for one resistor
    digitalWrite(enablePin, LOW);
    digitalWrite(dirPin, HIGH);  // Enables the motor to move forward

    for (int x = 0; x < stepsPerResistor; x++) {  // pulse stepper motor for one resistor
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }

    resistorCount++;
  }

  delay(1000);                    // One second delay
  digitalWrite(enablePin, HIGH);  // shutdown stepper driver

  dispenserState = CUTTING;
}

void cutting() {
  Serial.println("Cutting state");

  leftServo.write(0);
  rightServo.write(180);

  delay(2000);

  leftServo.write(180);
  rightServo.write(0);

  delay(2000);

  dispenserState = COMPLETE;
}

void complete() {
  Serial.println("Complete state");
  // reset the resistor count
  resistorCount = 0;
  // toggle the LED
  ledState = LOW;
  digitalWrite(ledPin, ledState);
  // reset the dispenser state
  dispenserState = IDLE;
}

void flashLED(int quantity) {
  for (int i = 0; i < quantity; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500); // LED on for 500 milliseconds
    digitalWrite(ledPin, LOW);
    delay(500); // LED off for 500 milliseconds
  }
}