// DRV8825 Motor Driver + Nema 17 Stepper Motor (17HS4401S) Test
// Working w/ 12V 5A Power Supply

// defines pins numbers
const int stepPin = 15; 
const int dirPin = 16; 
const int enablePin = 17;                // active low

const int microstepRes = 32;             // 1 / 32 microstep resolution
const int stepsPerRevolution = 200;      // from datasheet: 360 deg / 1.8 deg = 200

int stepsFullRotation = stepsPerRevolution * microstepRes;

void setup() {
  
  // Sets the two pins as Outputs
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
}
void loop() {
  digitalWrite(dirPin, HIGH); // Enables the motor to move in a particular direction

  for(int x = 0; x < stepsFullRotation; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500); 
  }
  
  delay(1000); // One second delay
  
}
