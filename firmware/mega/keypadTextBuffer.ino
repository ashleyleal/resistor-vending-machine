#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 3; 

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {35, 33, 31, 29}; 
byte colPins[COLS] = {27, 25, 23}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String textBuffer = "";  //empty string for the text buffer

void setup(){
  Serial.begin(9600);
}
  
void loop() {
  char key = keypad.getKey();  
  
  if (key){
    if (key == '*' || key == '#') { 
      textBuffer = "";  
      Serial.println("Buffer Cleared");
    } else {
      textBuffer += key;  
      Serial.print("Buffer: ");
      Serial.println(textBuffer);
    }
  }
}