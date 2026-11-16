#include <Arduino.h>
#include <DIYables_LCD_I2C.h>
#include "display.h"

// I2C LCD Setup
// Change LCD I2C information according to manufacturer
DIYables_LCD_I2C lcd(0x27, 20, 4);

// LCD DISPLAY LAYOUT (template) for 20x4
/*   01234567890123456789
    +--------------------+
    |  Welcome to 341!
    |Enter PIN:
    |********
    |Locked
    +--------------------+
*/

// lcd.setCursor(col, row); indexes start at 0

void initDisplay() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.noCursor();

    lcd.setCursor(0,2);
    lcd.print("Welcome to 341!");
}

void printMessage(const String& message) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(message);
}

// Print locked out message. TODO: Maybe change message?
void printLockout() {
    lcd.clearRow(2);
    lcd.setCursor(2,0);
    lcd.print("Please wait");
}