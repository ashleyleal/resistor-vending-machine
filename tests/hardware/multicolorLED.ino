int redPin = 13;    // Red RGB pin -> D13
int greenPin = 12;  // Green RGB pin -> D12
int bluePin = 11;   // Blue RGB pin -> D11

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  // make it white to start
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
    delay(500);
}

void loop() {
  // Flash RED
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(500); // Wait for 500ms

  // Flash GREEN
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
  delay(500); // Wait for 500ms

  // Flash BLUE
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
  delay(500); // Wait for 500ms

  // Flash YELLOW (Red + Green)
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
  delay(500); // Wait for 500ms

  // Flash all colors OFF (Optional)
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  delay(500); // Wait for 500ms
}
