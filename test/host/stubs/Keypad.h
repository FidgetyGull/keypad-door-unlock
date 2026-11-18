#pragma once
#include "Arduino.h"
#define makeKeymap(keys) reinterpret_cast<char *>(keys)
class Keypad {
public:
  inline static char nextKey = 0;
  Keypad(char *, byte *, byte *, byte, byte) {}
  char getKey() { char key = nextKey; nextKey = 0; return key; }
};
