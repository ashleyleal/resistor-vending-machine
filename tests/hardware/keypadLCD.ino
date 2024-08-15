#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

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

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

void setup(){
  Serial.begin(9600);
}

void loop(){
  char key = keypad.getKey(); // Read the key
  
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on

  // Print if key pressed
  if (key){
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);  // Process the key input in the buffer
  }
  
  lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
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

  lcd.print(textBuffer);
}
