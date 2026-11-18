#include "door_auth.h"
#include <Preferences.h>
#include "mbedtls/sha256.h"

namespace {
struct Credential {
  uint8_t salt[16];
  uint8_t hash[32];
};
// One versioned NVS blob keeps salts, hashes and membership together on writes.
struct PinStore {
  uint8_t magic[4];
  uint8_t version;
  uint8_t count;
  Credential admin;
  Credential pins[MAX_DOOR_PINS];
};
static_assert(sizeof(PinStore) == 6 + 48 * (MAX_DOOR_PINS + 1), "Unexpected padding");
PinStore store = {};
bool initialized = false;
bool adminPresent = false;

bool hashPin(const char *pin, const uint8_t salt[16], uint8_t out[32]) {
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  const bool ok = mbedtls_sha256_starts(&ctx, 0) == 0 &&
                  mbedtls_sha256_update(&ctx, salt, 16) == 0 &&
                  mbedtls_sha256_update(&ctx, reinterpret_cast<const uint8_t *>(pin), strlen(pin)) == 0 &&
                  mbedtls_sha256_finish(&ctx, out) == 0;
  mbedtls_sha256_free(&ctx);
  return ok;
}

bool matches(const char *pin, const Credential &credential) {
  uint8_t computed[32] = {};
  if (!hashPin(pin, credential.salt, computed)) return false;
  uint8_t diff = 0;
  for (size_t i = 0; i < sizeof(computed); ++i) diff |= computed[i] ^ credential.hash[i];
  return diff == 0;
}

int findPin(const char *pin) {
  for (uint8_t i = 0; i < store.count; ++i) {
    if (matches(pin, store.pins[i])) return i;
  }
  return -1;
}

bool makeCredential(const char *pin, Credential &credential) {
  esp_fill_random(credential.salt, sizeof(credential.salt));
  return hashPin(pin, credential.salt, credential.hash);
}

PinResult save(const PinStore &next) {
  Preferences prefs;
  if (!prefs.begin("door", false)) return PinResult::StorageError;
  const bool saved = prefs.putBytes("pins_v1", &next, sizeof(next)) == sizeof(next);
  prefs.end();
  if (!saved) return PinResult::StorageError;
  store = next; // Do not change active credentials if persistence failed.
  adminPresent = true;
  return PinResult::Success;
}
} // namespace

AuthState initAuth() {
  initialized = false;
  adminPresent = false;
  store = {};
  Preferences prefs;
  if (!prefs.begin("door", false)) return AuthState::StorageError;
  bool valid = true;
  if (prefs.isKey("pins_v1")) {
    valid = prefs.getBytesLength("pins_v1") == sizeof(store) &&
            prefs.getBytes("pins_v1", &store, sizeof(store)) == sizeof(store) &&
            memcmp(store.magic, "DPIN", 4) == 0 && store.version == 1 &&
            store.count <= MAX_DOOR_PINS;
    adminPresent = valid;
  } else {
    memcpy(store.magic, "DPIN", 4);
    store.version = 1;
    // Preserve the original salted PIN during the first admin provisioning.
    if (prefs.isKey("salt") || prefs.isKey("hash")) {
      valid = prefs.getBytesLength("salt") == 16 && prefs.getBytesLength("hash") == 32 &&
              prefs.getBytes("salt", store.pins[0].salt, 16) == 16 &&
              prefs.getBytes("hash", store.pins[0].hash, 32) == 32;
      if (valid) store.count = 1;
    }
  }
  prefs.end();
  initialized = valid;
  if (!valid) return AuthState::StorageError;
  return adminPresent ? AuthState::Ready : AuthState::NeedsAdmin;
}

bool hasAdminPin() { return initialized && adminPresent; }

bool isValidPin(const char *pin) {
  if (pin == nullptr || pin[0] == '\0') return false;
  for (size_t i = 0; pin[i] != '\0'; ++i) {
    if (i >= MAX_PIN_LENGTH) return false;
    const char key = pin[i];
    if (!((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D'))) return false;
  }
  return true;
}

bool checkPin(const char *entered) {
  return hasAdminPin() && isValidPin(entered) && findPin(entered) >= 0;
}

bool checkAdminPin(const char *entered) {
  return hasAdminPin() && isValidPin(entered) && matches(entered, store.admin);
}

PinResult setupAdminPin(const char *pin) {
  if (!initialized || adminPresent) return PinResult::NotReady;
  if (!isValidPin(pin)) return PinResult::Invalid;
  if (findPin(pin) >= 0) return PinResult::Duplicate;
  PinStore next = store;
  if (!makeCredential(pin, next.admin)) return PinResult::StorageError;
  return save(next);
}

PinResult changeAdminPin(const char *pin) {
  if (!hasAdminPin()) return PinResult::NotReady;
  if (!isValidPin(pin)) return PinResult::Invalid;
  if (findPin(pin) >= 0 || matches(pin, store.admin)) return PinResult::Duplicate;
  PinStore next = store;
  if (!makeCredential(pin, next.admin)) return PinResult::StorageError;
  return save(next);
}

PinResult addPin(const char *pin) {
  if (!hasAdminPin()) return PinResult::NotReady;
  if (!isValidPin(pin)) return PinResult::Invalid;
  if (findPin(pin) >= 0 || matches(pin, store.admin)) return PinResult::Duplicate;
  if (store.count == MAX_DOOR_PINS) return PinResult::Full;
  PinStore next = store;
  if (!makeCredential(pin, next.pins[next.count])) return PinResult::StorageError;
  ++next.count;
  return save(next);
}

PinResult removePin(const char *pin) {
  if (!hasAdminPin()) return PinResult::NotReady;
  if (!isValidPin(pin)) return PinResult::Invalid;
  const int index = findPin(pin);
  if (index < 0) return PinResult::NotFound;
  PinStore next = store;
  --next.count;
  next.pins[index] = next.pins[next.count];
  memset(&next.pins[next.count], 0, sizeof(Credential));
  return save(next);
}
