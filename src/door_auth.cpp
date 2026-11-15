#include "door_auth.h"
#include <Preferences.h>
#include "mbedtls/sha256.h"

static Preferences prefs;


static void hashPin(const char *pin, const uint8_t salt[16], uint8_t out[32]) {
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, salt, 16);
  mbedtls_sha256_update(&ctx, (const uint8_t *)pin, strlen(pin));
  mbedtls_sha256_finish(&ctx, out);
  mbedtls_sha256_free(&ctx);
}

static bool hashesMatch(const uint8_t *a, const uint8_t *b) {
  uint8_t diff = 0;
  for (int i = 0; i < 32; i++) diff |= a[i] ^ b[i];
  return diff == 0;
}

// setPin is called once plaint-text in setup() to store pin in permanent memory. Remove after setup.
void setPin(const char *pin) {
  uint8_t salt[16], hash[32];
  esp_fill_random(salt, 16);
  hashPin(pin, salt, hash);
  prefs.begin("door", false);
  prefs.putBytes("salt", salt, 16);
  prefs.putBytes("hash", hash, 32);
  prefs.end();
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