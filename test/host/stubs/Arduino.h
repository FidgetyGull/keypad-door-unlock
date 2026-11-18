#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <cassert>
#include <windows.h>
#include <bcrypt.h>
using byte = uint8_t;
class String : public std::string {
public:
  using std::string::string;
  String(const std::string &s) : std::string(s) {}
  String substring(size_t start, size_t end) const { return substr(start, end - start); }
};
inline unsigned long fakeMillis = 0;
inline unsigned long millis() { return fakeMillis; }
inline void esp_fill_random(void *out, size_t size) {
  assert(BCryptGenRandom(nullptr, static_cast<PUCHAR>(out), size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0);
}
