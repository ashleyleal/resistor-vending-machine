#include <SPI.h>

// Use this with controller.ino to test if the Nano can receive the correct quantity from the controller

// Define Slave Select (SS) Pin
#define SS_PIN 10  // This should match the SS pin set in the master code
#define LED_PIN 9

volatile int receivedQuantity = 0; // Integer quantity received from the Master
volatile bool newQuantityReceived = false; // Flag to indicate a new quantity has been received

void setup() {
  Serial.begin(9600);
  Serial.println("Nano setup starting...");

  pinMode(LED_PIN, OUTPUT);
  pinMode(MISO, OUTPUT); // Set MISO pin as output
  
  // Set MISO high by default
  digitalWrite(MISO, HIGH);

  // Initialize SPI in slave mode
  SPI.begin();  // Initialize SPI peripheral

  // Set SPI Slave select pin as input
  pinMode(SS_PIN, INPUT);
  
  // Enable SPI in Slave mode
  SPCR |= _BV(SPE);  // Enable SPI in Slave mode

  // Enable SPI interrupt
  SPCR |= _BV(SPIE);

  // Enable global interrupts
  sei();
}

void loop() {
  if (newQuantityReceived) {
    flashLED(receivedQuantity);
    newQuantityReceived = false; // Reset the flag
  }
}

ISR(SPI_STC_vect) {
  static byte highByte = 0;
  static byte lowByte = 0;
  static byte byteCount = 0;

  byte receivedData = SPDR;

  // Handle data reception
  if (byteCount == 0) {
    highByte = receivedData;
    byteCount++;
  } else if (byteCount == 1) {
    lowByte = receivedData;
    byteCount = 0;

    // Combine highByte and lowByte to reconstruct the integer
    receivedQuantity = (highByte << 8) | lowByte;
    newQuantityReceived = true;

    // Debug message
    Serial.print("SPI Interrupt: Received quantity: ");
    Serial.println(receivedQuantity);
  }
}

void flashLED(int quantity) {
  for (int i = 0; i < quantity; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500); // LED on for 500 milliseconds
    digitalWrite(LED_PIN, LOW);
    delay(500); // LED off for 500 milliseconds
  }
}
