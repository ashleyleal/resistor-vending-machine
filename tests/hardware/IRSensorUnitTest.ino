int irSensorPin = 4; 
int ledPin = 9;

void setup(){
  pinMode(irSensorPin, INPUT); 
  pinMode(ledPin, OUTPUT); 
}

void loop(){
  int sensorStatus = digitalRead(irSensorPin);
  if (sensorStatus == HIGH) // read sensor status
  {
    // if the pin is high turn off the onboard Led
    digitalWrite(ledPin, LOW);
  }
  else  {
    //else turn on the onboard LED
    digitalWrite(ledPin, HIGH); 
}
}