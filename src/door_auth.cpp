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

bool isValidPin(const char *pin) {
  if (pin == nullptr || pin[0] == '\0') return false;

  for (size_t i = 0; pin[i] != '\0'; ++i) {
    if (i >= MAX_PIN_LENGTH) return false;
    const char key = pin[i];
    if (!((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D'))) {
      return false;
    }
  }
  return true;
}

// Call once in setup() to provision the PIN; remove the plaintext afterward.
bool setPin(const char *pin) {
  if (!isValidPin(pin)) return false;

  uint8_t salt[16], hash[32];
  esp_fill_random(salt, 16);
  hashPin(pin, salt, hash);
  if (!prefs.begin("door", false)) return false;
  const bool saved = prefs.putBytes("salt", salt, 16) == 16 &&
                     prefs.putBytes("hash", hash, 32) == 32;
  prefs.end();
  return saved;
}

bool checkPin(const char *entered) {
  if (!isValidPin(entered)) return false;

  uint8_t salt[16], stored[32], computed[32];
  prefs.begin("door", true);
  prefs.getBytes("salt", salt, 16);
  prefs.getBytes("hash", stored, 32);
  prefs.end();
  hashPin(entered, salt, computed);
  return hashesMatch(computed, stored);
}
