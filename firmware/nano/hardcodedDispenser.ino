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
const int buttonPin = 3;
const int ledPin = 9;
const int irSensorPin = 4;
const int servoPin1 = 5;
const int servoPin2 = 6;

// initialize variables
volatile bool ledState = LOW;
int resistorCount = 0;           // count of resistors dispensed
int requestedResistorCount = 1;  // count of resistors requested, hardcoded to 1 for now

// debounce prevents the push button from being spammed
const unsigned long debounceDelay = 250;  // in ms
volatile unsigned long lastDebounceTime = 0;

// stepper motor configuration
int stepsPerRevolution = 2048;         // can be found in the motor's datasheet (this is for a 28BYJ-48 motor)
const int revolutionsPerResistor = 1;  // number of revolutions needed to dispense one resistor (Need to be calibrated)
int motSpeed = 10;                     // speed in rpm
// IN1 to A4 (18), IN2 to A3 (17), IN3 to A2 (16), IN4 to A1 (15)
Stepper myStepper(stepsPerRevolution, 18, 16, 17, 15);

// servo motor configuration
Servo myServo1;
Servo myServo2;
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
    // initialize pins
    pinMode(buttonPin, INPUT_PULLUP);  // set the button pin as input with pullup resistor to prevent floating
    pinMode(irSensorPin, INPUT);
    pinMode(ledPin, OUTPUT);

    myServo1.attach(servoPin1);
    myServo2.attach(servoPin2);

    // set the speed of the motor
    myStepper.setSpeed(motSpeed);

    // calling ISR on negedge of button press
    attachInterrupt(digitalPinToInterrupt(buttonPin), pushButtonISR, FALLING);

    // enable global interrupts
    sei();
}

// in the main loop, we will check the state of the dispenser and act accordingly

/*
Finite State Machine (FSM):  model used to design algorithms that can be in one of a finite number of states at any given time.
The FSM transitions from one state to another based on inputs or events, and it can perform specific actions during these transitions or while in a particular state.
*/

void loop() {
    switch (dispenserState) {
        case IDLE:
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
            ledState = !ledState;
            digitalWrite(ledPin, ledState);

            dispenserState = REELING;
        }
    }
}

// Function to put the Arduino into sleep mode
void enterSleepMode() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_disable();
}

// The following functions are used to define the behavior of the dispenser in each state
// They also transition the dispenser to the next state based on the FSM

void reeling() {
    // move the motor enough for one resistor
    myStepper.step(stepsPerRevolution * revolutionsPerResistor);
    dispenserState = COUNTING;
}

void counting() {
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
    for (servoPos = 0; servoPos <= 180; servoPos += 1) {
        myServo1.write(servoPos);
        myServo2.write(servoPos);
    }
    delay(500);
    for (servoPos = 180; servoPos >= 0; servoPos -= 1) {
        myServo1.write(servoPos);
        myServo2.write(servoPos);
    }
    dispenserState = COMPLETE;
}

void complete() {
    // reset the servo positions
    servoPos = 0;
    // reset the resistor count
    resistorCount = 0;
    // reset the dispenser state
    dispenserState = IDLE;
}