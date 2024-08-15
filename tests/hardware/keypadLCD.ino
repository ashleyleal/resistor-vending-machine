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

void setup() {
  Serial.begin(9600);

  lcd.init();           // Initialize the LCD
  lcd.clear();          // Clear the display
  lcd.backlight();      // Turn on the backlight
  lcd.setCursor(0,0);   // Set cursor to the beginning
}

void loop() {
  char key = keypad.getKey(); // Read the key

  // Print if key is pressed
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    handleKey(key);  // Process the key input in the buffer
  }
}

void handleKey(char key) {
  if (key == '*') { 
    textBuffer = "";  // Clear the text buffer if '*' is pressed
    lcd.clear();      // Clear the LCD display
    Serial.println("Buffer Cleared");
  } else {
    textBuffer += key;  // Append the pressed key to the buffer
    Serial.print("Buffer: ");
    Serial.println(textBuffer);
    
    // Print the updated buffer to the LCD
    lcd.clear();  // Optional: Clear the LCD to avoid overlap
    lcd.setCursor(0,0);   // Set cursor to the beginning
    lcd.print(textBuffer); // Display the buffer on the LCD
  }
}
