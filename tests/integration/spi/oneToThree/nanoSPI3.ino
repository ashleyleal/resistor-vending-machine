#include <SPI.h>

void setup() {
  pinMode(MISO, OUTPUT);        // Set MISO as OUTPUT for SPI communication
  SPCR |= _BV(SPE);             // Enable SPI in Slave mode
  pinMode(LED_BUILTIN, OUTPUT); // Set the built-in LED as OUTPUT
}

void loop() {
  if (SPI.transfer(0x00) == HIGH) {  // If the received byte is HIGH
    digitalWrite(LED_BUILTIN, HIGH); // Turn on the LED
  } else {
    digitalWrite(LED_BUILTIN, LOW);  // Turn off the LED
  }
}