#pragma once
#include "Arduino.h"
#include <map>
#include <vector>
// In-memory NVS double: failed writes leave the previous blob intact.
class Preferences {
public:
  inline static std::map<std::string, std::vector<uint8_t>> data;
  inline static bool failOpen = false, failWrite = false, failRead = false;
  bool begin(const char *, bool = false) { return !failOpen; }
  void end() {}
  bool isKey(const char *key) { return data.count(key) != 0; }
  size_t getBytesLength(const char *key) { return isKey(key) ? data.at(key).size() : 0; }
  size_t getBytes(const char *key, void *out, size_t size) {
    if (failRead || !isKey(key) || data.at(key).size() > size) return 0;
    memcpy(out, data.at(key).data(), data.at(key).size());
    return data.at(key).size();
  }
  size_t putBytes(const char *key, const void *value, size_t size) {
    if (failWrite) return 0;
    const auto *bytes = static_cast<const uint8_t *>(value);
    data[key] = std::vector<uint8_t>(bytes, bytes + size);
    return size;
  }
};
