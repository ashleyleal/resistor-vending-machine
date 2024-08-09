#include <Servo.h>

Servo leftServo;
Servo rightServo;

void setup() {
  leftServo.attach(5); 
  rightServo.attach(6); 

  leftServo.write(180);
  rightServo.write(0);

  delay(2000);
}

void loop() {
  leftServo.write(0);
  rightServo.write(180);  

  delay(2000);

  leftServo.write(180);
  rightServo.write(0);

  delay(2000);
}