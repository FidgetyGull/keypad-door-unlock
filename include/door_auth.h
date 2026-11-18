#pragma once
#include <Arduino.h>

constexpr uint8_t MAX_PIN_LENGTH = 8;
constexpr uint8_t MAX_DOOR_PINS = 20;
enum class AuthState { NeedsAdmin, Ready, StorageError };
enum class PinResult { Success, Invalid, Duplicate, Full, NotFound, StorageError, NotReady };

// Loads NVS and imports the previous single door PIN, if present.
// Invalid/unreadable credentials fail closed, never trigger admin setup.
AuthState initAuth();
bool hasAdminPin();
// PINs contain 1-8 keypad characters: digits or uppercase A-D.
bool isValidPin(const char *pin);
bool checkPin(const char *entered);
bool checkAdminPin(const char *entered);
PinResult setupAdminPin(const char *pin);
// Mutations below must only be exposed by an authenticated admin session.
PinResult changeAdminPin(const char *pin);
PinResult addPin(const char *pin);
PinResult removePin(const char *pin);
