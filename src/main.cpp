#include <Arduino.h>
#include <Keypad.h>
#include <DIYables_LCD_I2C.h>
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
byte rowPins[ROWS] = {19, 18, 5, 17}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {16, 4, 2, 15}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Door unlock control
void unlock() {
  // TODO: Implement door unlock
}


// Static variables to store number of pin failures
static int failures = 0;
const unsigned long LOCKOUT_DURATION = 30000; // 30 seconds
static bool lockoutActive = false;
static unsigned long lockoutStarted = 0;

bool checkLockout() {
  if (lockoutActive && millis() - lockoutStarted >= LOCKOUT_DURATION) {
    lockoutActive = false;
    resetDisplaySession();
  }
  return lockoutActive;
}

void handleKey(char key) {
  if (checkLockout()) return;
  const KeyResult result = handleDisplayKey(key);
  if (result == KeyResult::None) return;
  if (result == KeyResult::AdminAuthenticated) {
    failures = 0;
    return;
  }
  if (result == KeyResult::DoorGranted) {
    failures = 0;
    printMessage("Access Granted");
    // BEGIN DOOR UNLOCK FUNCTION
    unlock();
  }
  // 3 failures to lockout, change as necessary
  else if (++failures >= 3) {
    lockoutStarted = millis();
    lockoutActive = true;
    failures = 0;
    resetDisplaySession();
    printLockout();
  }

  else {
    printMessage("Try again");
  }
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
    resetDisplaySession(); // Clear PINs and end any admin session.
    if (lockoutActive) printLockout();
  }
}

void setup() {
  initDisplay();
  lastActivity = millis();
}

void loop() {
  checkLockout(); // Expire lockout even when no key is pressed
  char key = kpd.getKey(); // Get keypad key continuously
  if (key) {
    bool wasAsleep = !backlightOn;
    wakeBacklight();
    if (!wasAsleep) {
      handleKey(key);
    }
  }
  checkIdle();
}
