#include <Arduino.h>
#include <Preferences.h>
#include <Keypad.h>
#include <DIYables_LCD_I2C.h>
#include "door_auth.h"
#include "display.h"

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

// Door unlock control
void unlock() {
  // TODO: Implement door unlock
}


// Static variables to store number of pin failures
static int failures = 0;
static unsigned long lockoutUntil = 0;

void onPinEntered(const char *entered) {
  if (millis() < lockoutUntil) {
    printLockout();
    return;
  }

  if (checkPin(entered)) {
    failures = 0;
    printMessage("Unlocked");
    // BEGIN DOOR UNLOCK FUNCTION
    unlock();
  }
  // 3 failues to lockout, change as necessary
  else if (++failures >= 3) {
    // 30,000 ms = 30 seconds, change as necessary
    lockoutUntil = millis() + 30000; 
    failures = 0;
  }
}

// PIN entry setup
const uint8_t MAX_LEN = 8; // Set max password length
char buffer[MAX_LEN + 1]; // max password input + terminator
uint8_t len = 0; // current password length

void clearBuffer() {
  memset(buffer, 0, MAX_LEN); // Clears buffer
  len = 0;
  clearPIN(); // Reset display PIN entry
}

// Keypad handling functions with idle timer
const unsigned long IDLE_TIMEOUT = 10000; // 10 second idle timeout
unsigned long lastActivity = 0;
bool backlightOn = true;

void wakeBacklight() {
  lastActivity = millis();
  if (!backlightOn) {
    lcd.backlight();
    backlightOn = true;
  }
}

void checkIdle() {
  if (backlightOn && (millis() - lastActivity >= IDLE_TIMEOUT)) {
    lcd.noBacklight();
    backlightOn = false;
    clearBuffer(); // Clears any entered pin when idle
  }
}

void handleKey(char key) {
  Serial.println(key); // Comment out if no serial monitoring necessary
  if (key == '*') {
    clearBuffer();
  } else if (key == '#') {
    buffer[len] = '\0';
    onPinEntered(buffer);
    clearBuffer();
  } else if (len < MAX_LEN) {
    buffer[len++] = key;
  }
}


void setup() {
  // Begin serial monitoring (comment out for permanent install)
  Serial.begin(9600);
  // setPin("1234");  // run once, flash, then remove

  initDisplay();
}

void loop() {
  char key = kpd.getKey(); // Get keypad key continuously
  if (key) {
    bool wasAsleep = !backlightOn;
    if (!wasAsleep) {
      handleKey(key);
    }
  }
  checkIdle();
}
