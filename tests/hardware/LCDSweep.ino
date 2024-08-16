#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Call the LCDsweep function with the strings for each row and the desired speed
  LCDsweep("Hello world!", "LCD Tutorial", 250);  // Adjust speed as needed
}

void loop() {
  // Nothing needed in the loop for now
}

// Function to sweep text on the LCD if it's too long
void LCDsweep(const char* row0, const char* row1, int speed) {
  int len0 = strlen(row0);
  int len1 = strlen(row1);

  // Print initial strings
  lcd.setCursor(0, 0);
  lcd.print(row0);
  
  lcd.setCursor(0, 1);
  lcd.print(row1);

  // Check if the text needs sweeping
  if (len0 > 16 || len1 > 16) {
    // Scroll text on the first row if it's too long
    for (int position = 0; position < len0 + 16; position++) {
      lcd.setCursor(0, 0);  // Reset cursor to start of the row
      lcd.print(getSubString(row0, position, 16));
      delay(speed);
    }
    
    // Scroll text on the second row if it's too long
    for (int position = 0; position < len1 + 16; position++) {
      lcd.setCursor(0, 1);  // Reset cursor to start of the row
      lcd.print(getSubString(row1, position, 16));
      delay(speed);
    }
  }
}

// Helper function to get a substring with wrapping
String getSubString(const char* text, int startPos, int length) {
  String result = "";
  int len = strlen(text);
  
  for (int i = 0; i < length; i++) {
    int index = (startPos + i) % len;
    result += text[index];
  }
  
  return result;
}
