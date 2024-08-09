/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/10/2024
 * 
 * File Name   : megaSPI.ino
 * Purpose     : Test SPI communication on the master device (Arduino Mega)
 * 
 */

#include <SPI.h>

const int ledPin = 22;
bool ledState = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  // initialize SPI as master
  SPI.begin(); 

  // set SPI clock speed
  SPI.setClockDivider(SPI_CLOCK_DIV8); 

  // set SS pin as output
  pinMode(SS, OUTPUT); // 
}

void loop() {
  // send data to the slave and receive data
  digitalWrite(SS, LOW);

  // send current state of LED to slave
  byte receivedData = SPI.transfer(ledState); 
  
  // write high to SS to stop communication (ss is active low)
  digitalWrite(SS, HIGH);
 
  // toggle master's LED state opposite to the slave's LED state
  ledState = !receivedData; 
  digitalWrite(ledPin, ledState);

  // delay to make the toggling visible
  delay(1000);
}