#include <Arduino.h>
#include <Preferences.h>
#include <Keypad.h>
#include <DIYables_LCD_I2C.h>
#include <mbedtls/sha256.h>
#include <string.h>

// Module to store hashed password/pin across restarts
Preferences prefs;

// Keypad module and initial setup
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
// char keys[ROWS][COLS] = {
// {'1','2','3'},
// {'4','5','6'},
// {'7','8','9'},
// {'*','0','#'}
// };
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {8, 7, 6}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );



// I2C LCD Setup
// Change LCD I2C address according to manufacture
DIYables_LCD_I2C lcd(0x27, 20, 4);







void setup() {
  // Begin serial monitoring (comment out for permanent install)
  Serial.begin(9600);
}

void loop() {
  // Get keypad key continuously
  char key = kpd.getKey();
}
