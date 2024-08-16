// use with controller.ino

#include <SPI.h>

// Pin connected to the LED
const int ledPin = 9;

void setup() {
    pinMode(ledPin, OUTPUT);
    SPI.begin();
    // Set the SPI in Slave mode
    SPCR |= _BV(SPE);
    // Enable SPI interrupt
    SPI.attachInterrupt();
}

volatile int resistorQuantity = 0;

ISR (SPI_STC_vect) {
    // When data is received, store it in resistorQuantity
    resistorQuantity = SPDR;
}

void loop() {
    if (resistorQuantity > 0) {
        // Flash the LED resistorQuantity times
        for (int i = 0; i < resistorQuantity; i++) {
            digitalWrite(ledPin, HIGH);   // Turn the LED on
            delay(500);                    // Wait for 500 ms
            digitalWrite(ledPin, LOW);    // Turn the LED off
            delay(500);                    // Wait for 500 ms
        }
        resistorQuantity = 0;  // Reset the quantity after flashing
    }
}
