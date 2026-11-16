#include <Arduino.h>
#include <DIYables_LCD_I2C.h>
#include "display.h"

// I2C LCD Setup
// Change LCD I2C information according to manufacturer
DIYables_LCD_I2C lcd(0x27, 20, 4);

// LCD DISPLAY LAYOUT (template) for 20x4
/*   12345678901234567890
    +--------------------+
    |  Welcome to 341!
    |Enter PIN:
    |********
    |Locked
    +--------------------+
*/

void initDisplay() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.noCursor();
}

void printMessage(const String& message) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(message);
}

// Print locked out message. TODO: Maybe change message?
void printLockout() {
    lcd.clearRow(2);
    lcd.setCursor(0, 2);
    lcd.print("Please wait");
}