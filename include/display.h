#pragma once

#include <Arduino.h>
#include <DIYables_LCD_I2C.h>

extern DIYables_LCD_I2C lcd;

// helper LCD functions
void initDisplay();
void printMessage(const String& message);
void printLockout();