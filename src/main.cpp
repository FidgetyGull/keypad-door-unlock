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
    printMessage("Locked");
  }
  return lockoutActive;
}

void onPinEntered(const char *entered, bool overflowed = false) {
  if (checkLockout()) {
    printLockout();
    return;
  }

  if (!overflowed && checkPin(entered)) {
    failures = 0;
    printMessage("Unlocked");
    // BEGIN DOOR UNLOCK FUNCTION
    unlock();
  }
  // 3 failures to lockout, change as necessary
  else if (++failures >= 3) {
    lockoutStarted = millis();
    lockoutActive = true;
    failures = 0;
    printLockout();
  }

  else {
    printMessage("Try again");
  }
}

// PIN entry setup
const uint8_t MAX_LEN = MAX_PIN_LENGTH;
char buffer[MAX_LEN + 1]; // max password input + terminator
uint8_t len = 0; // current password length
bool pinOverflowed = false; // Stays invalid until cleared or submitted

void clearBuffer() {
  memset(buffer, 0, sizeof(buffer)); // Clears buffer
  len = 0;
  pinOverflowed = false;
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
    if (pinOverflowed) printMessage("Locked");
    clearBuffer(); // Clears any entered pin when idle
  }
}

void handleKey(char key) {
  if (checkLockout()) return;

  Serial.println(key); // Comment out if no serial monitoring necessary
  if (key == '*') {
    if (pinOverflowed) printMessage("Locked");
    clearBuffer();
  } else if (key == '#') {
    buffer[len] = '\0';
    onPinEntered(buffer, pinOverflowed);
    clearBuffer();
  } else if (len < MAX_LEN) {
    buffer[len++] = key;
    advanceCursor();
  } else {
    pinOverflowed = true;
    printMessage("PIN too long");
  }
}


void setup() {
  // Begin serial monitoring (comment out for permanent install)
  Serial.begin(9600);
  // setPin("1234");  // run once, flash, then remove

  initDisplay();
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
