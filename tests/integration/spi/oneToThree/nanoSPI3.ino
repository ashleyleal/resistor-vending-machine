#include <SPI.h>

// Define Slave Select (SS) Pin
#define SS_PIN 10 // This should match the SS pin set in the master code
#define LED_PIN 9

volatile bool ledState = LOW; // LED state received from the Master

void setup() {
  Serial.begin(9600);
  Serial.println("Nano setup starting...");

  pinMode(LED_PIN, OUTPUT);
  pinMode(MISO, OUTPUT);
  
  // Set MISO high by default
  digitalWrite(MISO, HIGH);

  // Initialize SPI in slave mode
  SPCR |= _BV(SPE); // Enable SPI

  // Enable SPI interrupt
  SPCR |= _BV(SPIE); 

  // Enable global interrupts
  sei();
}

void loop() {
  // Nothing to do here
}

ISR(SPI_STC_vect) {
  // Read data from master
  byte receivedData = SPDR;

  // Set the state of the LED to copy the master's LED state
  ledState = receivedData;

  // Update the LED state
  digitalWrite(LED_PIN, ledState);

  // Debug message
  Serial.print("SPI Interrupt: Received LED state: ");
  Serial.println(ledState);
}
