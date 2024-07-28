#include <Servo.h>

Servo leftServo;
Servo rightServo;

void setup() {
  leftServo.attach(5); 
  rightServo.attach(6); 

  leftServo.write(90);
  rightServo.write(0);

  delay(1000);
}

void loop() {
  leftServo.write(0);
  rightServo.write(90);  

  delay(1000);

  leftServo.write(90);
  rightServo.write(0);

  delay(1000);
}