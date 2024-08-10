#include <Keypad.h>
#include <SPI.h>

// KEYPAD
const byte ROWS = 4; 
const byte COLS = 3; 

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {35, 33, 31, 29}; 
byte colPins[COLS] = {27, 25, 23}; 

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// making the keypad unspammable
const unsigned long debounceDelay = 250; // in ms
volatile unsigned long lastDebounceTime = 0;

// SPI


// Define Slave Select (SS) Pins for each Nano
#define SS_NANO1 40
#define SS_NANO2 41
#define SS_NANO3 42
#define SS_NANO4 43

// define states for the master FSM
enum MasterState {
    IDLE,
    SELECT_RESISTOR,
    SELECT_QUANTITY,
    DISPENSE_SIGNAL,
    COMPLETE
};

MasterState masterState = IDLE;  // set initial state of the master

enum SelectedResistor {
    RESISTOR1,
    RESISTOR2,
    RESISTOR3,
    RESISTOR4
};

SelectedResistor selectedResistor;
int resistorQuantity = 0;

void setup(){
  Serial.begin(9600);
  SPI.begin();
  
    // Set SS pins as OUTPUT and set them HIGH because SS is active low
  pinMode(SS_NANO1, OUTPUT);
  pinMode(SS_NANO2, OUTPUT);
  pinMode(SS_NANO3, OUTPUT);
  pinMode(SS_NANO4, OUTPUT);
  digitalWrite(SS_NANO1, HIGH);
  digitalWrite(SS_NANO2, HIGH);
  digitalWrite(SS_NANO3, HIGH);
  digitalWrite(SS_NANO4, HIGH);


}

void loop(){
    switch(masterState) {
        case IDLE:

            break;
        case SELECT_RESISTOR:

            break;
        case SELECT_QUANTITY:

            break;
        case DISPENSE_SIGNAL:
            dispenseSignal();
            break;
        case COMPLETE:

            break;
    }
}


void idle() {

}

void selectResistor() {
    // prompt user to select between 1-4

    // read the key
    char key = keypad.getKey();

    // print selected resistor on screen

    // set selectedResistor
    switch (key)
    {
    case '1':
        selectedResistor = RESISTOR1;
        break;
    case '2':
        selectedResistor = RESISTOR2;
        break;
    case '3':
        selectedResistor = RESISTOR3;
        break;
    case '4':
        selectedResistor = RESISTOR4;
        break;
    }
}

void selectQuantity() {
    // prompt quantity (5-10)


}

void dispenseSignal() {

    // send signal to selected Nano and print resistor value (TBD)
    switch (selectedResistor)
    {
    case RESISTOR1:
        sendSignal(SS_NANO1, selectQuantity);
        break;
    case RESISTOR2:
        sendSignal(SS_NANO2, selectQuantity);
        break;
    case RESISTOR3:
        sendSignal(SS_NANO3, selectQuantity);
        break;
    case RESISTOR4:
        sendSignal(SS_NANO4, selectQuantity);
        break;
    }
    masterState = COMPLETE;
}

void complete() {
    // reset selected resistor, resistor quantity, make sure no chips are selsected, 
}

void sendSignal(int ssPin, int quantity) {
    digitalWrite(ssPin, LOW);            // Select the Nano
    SPI.transfer(quantity);              // Send LED state (HIGH or LOW)
    digitalWrite(ssPin, HIGH);           // Deselect the Nano
}