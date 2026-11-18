#pragma once
#include <Arduino.h>
#include <DIYables_LCD_I2C.h>

extern DIYables_LCD_I2C lcd;
enum class KeyResult { None, DoorGranted, AdminAuthenticated, AuthFailed };
void initDisplay();
KeyResult handleDisplayKey(char key);
// Clears all PIN buffers and ends the authenticated admin session.
void resetDisplaySession();
void printMessage(const String &message);
void printLockout();
