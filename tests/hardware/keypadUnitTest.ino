#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 

char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte rowPins[ROWS] = {37, 35, 33, 31}; 
byte colPins[COLS] = {29, 27, 25, 23}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String textBuffer = "";  // Initialize the text buffer

void setup(){
  Serial.begin(9600);
}

void loop(){
  char key = keypad.getKey(); // Read the key
  
  // Print if key pressed
  if (key){
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);  // Process the key input in the buffer
  }
}

void handleKey(char key) {
  if (key == '*') { 
    textBuffer = "";  
    Serial.println("Buffer Cleared");
  } else {
    textBuffer += key;  
    Serial.print("Buffer: ");
    Serial.println(textBuffer);
  }
}
