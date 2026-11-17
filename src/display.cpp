#include <Arduino.h>
#include <DIYables_LCD_I2C.h>
#include "display.h"

// I2C LCD Setup
// Change LCD I2C information according to manufacturer
DIYables_LCD_I2C lcd(0x27, 20, 4);

// LCD DISPLAY LAYOUT (template) for 20x4
/*   01234567890123456789
    +--------------------+
    |Welcome to 341!
    |Enter PIN:
    |********
    |Locked
    +--------------------+
*/

// lcd.setCursor(col, row); indexes start at 0

// Track current PIN cursor location
static uint8_t cursor = 0;

void initDisplay() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.noCursor();

    lcd.setCursor(0,0);
    lcd.print("Welcome to 341!");
    lcd.setCursor(0,1);
    lcd.print("Enter PIN: ");
    lcd.setCursor(0,3);
    lcd.print("Locked");

    lcd.setCursor(0,2); // move cursor to PIN entry and show
    lcd.cursor();
}

void printMessage(const String& message) {
    lcd.clearRow(3);
    lcd.setCursor(0,3);
    lcd.print(message);
    lcd.setCursor(cursor, 2); // Status changes must not move PIN entry
}

// Keep lockout feedback on the status row.
void printLockout() {
    printMessage("Locked Out");
}

// Called only from clearBuffer() from main.cpp
void clearPIN() {
    lcd.clearRow(2);
    lcd.setCursor(0,2);
    lcd.cursor();
    cursor = 0;
}

void advanceCursor() {
    ++cursor;
    lcd.write('*');
    lcd.setCursor(cursor, 2);
}
