#include "door_auth.h"
#include "display.h"
#include <Preferences.h>
#include <Keypad.h>
#include <algorithm>
#include <iostream>

void setup();
void loop();
void handleKey(char key);
bool checkLockout();
void checkIdle();
void wakeBacklight();

static void keys(const char *sequence) {
  for (const char *p = sequence; *p; ++p) handleKey(*p);
}
static bool lineHas(int row, const char *text) {
  return lcd.rows[row].find(text) != std::string::npos;
}
static void login(const char *pin) {
  keys("#"); keys(pin); keys("#");
  assert(lineHas(0, "Admin menu"));
}
static void fresh() {
  Preferences::data.clear();
  Preferences::failOpen = Preferences::failRead = Preferences::failWrite = false;
  initDisplay();
}

int main() {
  fresh();
  assert(lineHas(0, "Set first admin PIN"));
  assert(!checkPin("1234") && !hasAdminPin());
  assert(addPin("1234") == PinResult::NotReady);
  keys("1234#4321#");
  assert(!hasAdminPin() && lineHas(3, "PINs do not match"));
  keys("123456789#");
  assert(!hasAdminPin() && lineHas(3, "Use 1-8 PIN keys"));
  keys("1234#");
  resetDisplaySession(); // Incomplete setup cannot survive idle timeout.
  assert(!hasAdminPin() && lineHas(0, "Set first admin PIN"));
  Preferences::failWrite = true;
  keys("1234#1234#");
  assert(!hasAdminPin() && lineHas(3, "Save failed"));
  Preferences::failWrite = false;
  keys("12AD#12AD#");
  assert(hasAdminPin() && checkAdminPin("12AD") && !checkPin("12AD"));
  assert(lineHas(0, "Admin menu"));
  assert(setupAdminPin("1111") == PinResult::NotReady);

  // Confirmation, duplicate detection, cancellation, masked input and alphabetic PINs.
  keys("A23BC");
  assert(lineHas(2, "****") && !lineHas(2, "23BC"));
  keys("#23BC#");
  assert(checkPin("23BC"));
  keys("A23BC#23BC#");
  assert(lineHas(3, "PIN already in use"));
  keys("A12AD#12AD#");
  assert(lineHas(3, "PIN already in use"));
  keys("A7777#8888#");
  assert(!checkPin("7777") && !checkPin("8888"));
  keys("*"); // Empty field returns to menu.
  keys("A7777#*"); // Cancel confirmation.
  assert(!checkPin("7777") && lineHas(0, "Admin menu"));
  keys("B9999#");
  assert(lineHas(3, "PIN not found"));
  keys("B12AD#"); // Removing door PINs cannot delete admin.
  assert(checkAdminPin("12AD") && lineHas(3, "PIN not found"));

  // Save errors retain current credentials, including across reboot.
  const auto beforeFailure = Preferences::data;
  Preferences::failWrite = true;
  keys("B23BC#");
  assert(checkPin("23BC") && lineHas(3, "Save failed"));
  keys("C4567#4567#");
  assert(checkAdminPin("12AD") && !checkAdminPin("4567"));
  assert(Preferences::data == beforeFailure);
  Preferences::failWrite = false;
  initDisplay();
  assert(lineHas(0, "Welcome") && checkPin("23BC") && checkAdminPin("12AD"));
  login("12AD");
  keys("C4567#4567#D");
  initDisplay();
  assert(checkAdminPin("4567") && !checkAdminPin("12AD"));
  login("4567");
  keys("B23BC#D");
  initDisplay();
  assert(!checkPin("23BC"));

  // Every credential gets a fresh salt, even after deleting/re-adding the same PIN.
  assert(addPin("2345") == PinResult::Success);
  const auto first = Preferences::data.at("pins_v1");
  assert(removePin("2345") == PinResult::Success);
  assert(addPin("2345") == PinResult::Success);
  const auto second = Preferences::data.at("pins_v1");
  assert(!std::equal(first.begin() + 54, first.begin() + 70, second.begin() + 54));
  assert(!std::equal(second.begin() + 6, second.begin() + 22, second.begin() + 54));
  const std::string persisted(second.begin(), second.end());
  assert(persisted.find("2345") == std::string::npos);
  assert(persisted.find("4567") == std::string::npos);
  for (int i = 0; i < MAX_DOOR_PINS - 1; ++i) {
    assert(addPin(std::to_string(10000 + i).c_str()) == PinResult::Success);
  }
  assert(addPin("9999") == PinResult::Full);
  assert(removePin("10005") == PinResult::Success);
  assert(addPin("9999") == PinResult::Success);
  assert(initAuth() == AuthState::Ready);
  assert(checkPin("9999") && checkPin("10018") && !checkPin("10005"));
  assert(!isValidPin(nullptr) && !isValidPin("") && !isValidPin("123456789"));
  assert(!isValidPin("12#3") && !isValidPin("abcd"));

  // Upgrade from the previous salt/hash keys preserves its door credential.
  Preferences::data.clear();
  Preferences::data["salt"] = {second.begin() + 54, second.begin() + 70};
  Preferences::data["hash"] = {second.begin() + 70, second.begin() + 102};
  assert(initAuth() == AuthState::NeedsAdmin);
  assert(setupAdminPin("2345") == PinResult::Duplicate);
  assert(setupAdminPin("4567") == PinResult::Success);
  assert(initAuth() == AuthState::Ready && checkPin("2345"));
  assert(removePin("2345") == PinResult::Success);
  assert(initAuth() == AuthState::Ready && !checkPin("2345")); // No legacy resurrection.

  // Corrupt or unreadable storage must not expose first-boot provisioning.
  const auto saved = Preferences::data;
  Preferences::data["pins_v1"].resize(10);
  initDisplay();
  assert(lineHas(0, "PIN storage error"));
  keys("1234#1234#");
  assert(!hasAdminPin() && !checkPin("2345"));
  Preferences::data = saved;
  Preferences::data["pins_v1"][5] = 255;
  assert(initAuth() == AuthState::StorageError);
  Preferences::data = saved;
  Preferences::failRead = true;
  assert(initAuth() == AuthState::StorageError);
  Preferences::failRead = false;
  Preferences::failOpen = true;
  assert(initAuth() == AuthState::StorageError);
  Preferences::failOpen = false;
  Preferences::data.clear();
  Preferences::data["salt"] = std::vector<uint8_t>(16);
  assert(initAuth() == AuthState::StorageError);

  // Door and admin failures share the lockout; switching screens never clears it.
  fresh();
  keys("4567#4567#A2345#2345#D");
  keys("0000##0000#*0000#");
  assert(checkLockout() && lineHas(3, "Locked Out"));
  keys("#4567#A8888#8888#");
  assert(!checkPin("8888") && checkLockout());
  fakeMillis += 30000;
  assert(!checkLockout());
  login("4567");
  keys("A8888#");
  wakeBacklight();
  fakeMillis += 10000;
  checkIdle();
  assert(!lcd.lit && lineHas(0, "Welcome"));
  Keypad::nextKey = 'A';
  loop(); // First key only wakes the LCD.
  assert(lcd.lit && lcd.rows[2] == std::string(20, ' '));
  keys("8888#");
  assert(!checkPin("8888") && lineHas(3, "Try again"));
  keys("2345#");
  assert(lineHas(3, "Access Granted"));
  login("4567");
  keys("D#456700000#"); // Valid admin prefix plus overflow must fail.
  assert(lineHas(3, "Try again") && !lineHas(0, "Admin menu"));
  std::cout << "PASS: storage, migration, menu, LCD bounds, lockout, timeout and wake tests\n";
}
