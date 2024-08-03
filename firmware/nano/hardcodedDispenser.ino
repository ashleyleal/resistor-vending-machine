/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/11/2024
 *
 * File Name   : hardcodedDispenser.ino
 * Purpose     :
 *
 */

#include <Servo.h>
#include <Stepper.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

// define pins
const int irOnPin = 2;
const int buttonPin = 3;
const int irSensorPin = 4;
const int servoPin1 = 5;
const int servoPin2 = 6;
const int ledPin = 9;

// initialize variables
volatile bool ledState = LOW;
int resistorCount = 0;           // count of resistors dispensed
int requestedResistorCount = 1;  // count of resistors requested, hardcoded to 1 for now

// debounce prevents the push button from being spammed
const unsigned long debounceDelay = 250;  // in ms
volatile unsigned long lastDebounceTime = 0;

// stepper motor configuration
const int stepPin = 15; 
const int dirPin = 16; 
const int enablePin = 17;                // active low
const int microstepRes = 32;             // 1 / 32 microstep resolution
const int stepsPerRevolution = 200;      // from datasheet: 360 deg / 1.8 deg = 200
int stepsPerResistor = stepsPerRevolution * microstepRes * 0.5;

// servo motor configuration
Servo leftServo;
Servo rightServo;
int servoPos = 0;

// define states for the dispenser FSM
enum DispenserState {
    IDLE,
    REELING,
    CUTTING,
    COUNTING,
    COMPLETE
};

DispenserState dispenserState = IDLE;  // set initial state of the dispenser

void setup() {
    // initialize serial communication for debugging
    Serial.begin(9600);
    Serial.println("Setup started");

    // initialize pins
    pinMode(buttonPin, INPUT_PULLUP);  // set the button pin as input with pullup resistor to prevent floating
    pinMode(ledPin, OUTPUT);
    
    pinMode(irSensorPin, INPUT);
    pinMode(irOnPin, OUTPUT);
    digitalWrite(irOnPin, LOW);

    leftServo.attach(servoPin1);
    rightServo.attach(servoPin2);

  leftServo.write(170);
  rightServo.write(10);

    // stepper motor pins
    pinMode(stepPin, OUTPUT); 
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, HIGH); // stepper driver off to start

    // calling ISR on negedge of button press
    attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

    // enable global interrupts
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
            Serial.println("Entering sleep mode");
            delay(100);
            enterSleepMode();
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

            digitalWrite(irOnPin, HIGH);
            dispenserState = REELING;
        }
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
    // move the motor enough for one resistor
    digitalWrite(enablePin, LOW);
    digitalWrite(dirPin, HIGH); // Enables the motor to move forward

    for(int x = 0; x < stepsPerResistor; x++) { // pulse stepper motor
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500); 
    }
  
  delay(1000); // One second delay
  digitalWrite(enablePin, HIGH); // shutdown stepper driver 
    dispenserState = COUNTING;
}

void counting() {
    Serial.println("Counting state");
    int sensorStatus = digitalRead(irSensorPin);
    if (sensorStatus == LOW)  // read sensor status (is this active low?)
    {
        resistorCount++;
    }
    // now check if enough resistors have been reeled out
    if (resistorCount == requestedResistorCount) {
        dispenserState = CUTTING;
    } else {
        dispenserState = REELING;
    }
}

void cutting() {
    Serial.println("Cutting state");
  leftServo.write(10);
  rightServo.write(170);  

  delay(2000);

  leftServo.write(170);
  rightServo.write(10);

  delay(2000);
    dispenserState = COMPLETE;
}

void complete() {
    Serial.println("Complete state");
    // reset the servo positions
    servoPos = 0;
    // reset the resistor count
    resistorCount = 0;
    // toggle the LED
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    digitalWrite(irOnPin, LOW);
    // reset the dispenser state
    dispenserState = IDLE;
}
