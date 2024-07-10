/*
 * Project Name: Resistor Vending Machine
 * Author      : Ashley Leal
 * Date        : 07/10/2024
 * 
 * File Name   : nanoSPI.ino
 * Purpose     : Test SPI communication on the slave device (Arduino Nano)
 * 
 */

#include <SPI.h>

const int ledPin = 9;
bool ledState;

void setup() {
  pinMode(ledPin, OUTPUT);

  // initialize SPI in slave mode
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);

  // enable interrupts from SPI
  SPCR |= _BV(SPIE);

  // enable global interrupts
  sei();
}

ISR(SPI_STC_vect) { // SPI interrupt routine
  // read data from master
  byte receivedData = SPDR;

  // set the state of the LED to copy the master's LED state
  ledState = receivedData;
  digitalWrite(ledPin, ledState);

  // send the state of the slave's LED back to the master
  SPDR = ledState;
}

void loop() {
  // main loop does nothing; SPI is handled by the ISR in slave mode 
}