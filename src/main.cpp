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


void hashPin(const char *pin, const uint8_t salt[16], uint8_t out[32]) {
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, salt, 16);
  mbedtls_sha256_update(&ctx, (const uint8_t *)pin, strlen(pin));
  mbedtls_sha256_finish(&ctx, out);
  mbedtls_sha256_free(&ctx);
}

bool hashesMatch(const uint8_t *a, const uint8_t *b) {
  // TODO: Constant-time compare
}

// setPin is called once plaint-text in setup() to store pin in permanent memory. Remove after setup.
void setPin(const char *pin) {
  uint8_t salt[16], hash[32];
  // TODO: set pin
}

bool checkPin(const char *entered) {
  uint8_t salt[16], stored[32], computed[32];
  prefs.begin("door", true);
  prefs.getBytes("salt", salt, 16);
  prefs.getBytes("hash", stored, 32);
  prefs.end();
  hashPin(entered, salt, computed);
  return hashesMatch(computed, stored);
}

// Static variables to store number of pin failures
static int failures = 0;
static unsigned long lockoutUntil = 0;

void onPinEntered(const char *entered) {
  if (millis() < lockoutUntil) return;

  if (checkPin(entered)) {
    failures = failures = 0;
    // BEGIN DOOR UNLOCK FUNCTION
  }
  // 3 failues to lockout, change as necessary
  else if (++failures >= 3) {
    // 30,000 ms = 30 seconds, change as necessary
    lockoutUntil = millis() + 30000; 
    failures = 0;
  }
}




void setup() {
  // Begin serial monitoring (comment out for permanent install)
  Serial.begin(9600);
}

void loop() {
  // Get keypad key continuously
  char key = kpd.getKey();
}
