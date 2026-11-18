#pragma once
#include "Arduino.h"
#include <array>
class DIYables_LCD_I2C {
public:
  std::array<std::string, 4> rows;
  size_t x = 0, y = 0;
  bool lit = true;
  DIYables_LCD_I2C(int, int, int) { clear(); }
  void init() {}
  void backlight() { lit = true; }
  void noBacklight() { lit = false; }
  void cursor() {}
  void noCursor() {}
  void clear() { for (auto &line : rows) line.assign(20, ' '); x = y = 0; }
  void clearRow(uint8_t line) { assert(line < 4); rows[line].assign(20, ' '); }
  void setCursor(size_t col, size_t line) { assert(col < 20 && line < 4); x = col; y = line; }
  void write(char c) { assert(x < 20 && y < 4); rows[y][x++] = c; }
  void print(const String &s) { for (char c : s) write(c); }
};
