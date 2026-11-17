#pragma once
#include <Arduino.h>

constexpr uint8_t MAX_PIN_LENGTH = 8;

// PINs contain 1-8 keypad characters: digits or uppercase A-D.
bool isValidPin(const char *pin);
// Returns false for invalid PINs or failed storage writes.
bool setPin(const char *pin);
bool checkPin(const char *entered);
