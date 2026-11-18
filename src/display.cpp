#include "display.h"
#include "door_auth.h"

DIYables_LCD_I2C lcd(0x27, 20, 4);

namespace {
enum class Screen {
  Door, AdminLogin, Setup, SetupConfirm, Menu, Add, AddConfirm,
  Remove, Change, ChangeConfirm, StorageError
};
Screen screen = Screen::StorageError;
char entry[MAX_PIN_LENGTH + 1] = {};
char pending[MAX_PIN_LENGTH + 1] = {};
uint8_t length = 0;
bool overflowed = false;

void wipe(char *buffer, size_t size) {
  volatile char *p = buffer;
  while (size--) *p++ = 0;
}

void clearEntry() {
  wipe(entry, sizeof(entry));
  length = 0;
  overflowed = false;
}

void row(uint8_t line, const String &text) {
  lcd.clearRow(line);
  lcd.setCursor(0, line);
  lcd.print(text.substring(0, 20));
}

bool isInput() { return screen != Screen::Menu && screen != Screen::StorageError; }

void show(Screen next, const char *message = nullptr) {
  screen = next;
  clearEntry();
  lcd.clear();
  lcd.noCursor();
  if (screen == Screen::Menu) {
    row(0, "Admin menu  D:Exit");
    row(1, "A:Add PIN B:Remove");
    row(2, "C:Change admin PIN");
  } else if (screen == Screen::StorageError) {
    row(0, "PIN storage error");
    row(1, "Access disabled");
  } else {
    const char *title = "";
    switch (screen) {
      case Screen::Door: title = "Welcome to 341!"; break;
      case Screen::AdminLogin: title = "Enter admin PIN"; break;
      case Screen::Setup: title = "Set first admin PIN"; break;
      case Screen::SetupConfirm: title = "Confirm admin PIN"; break;
      case Screen::Add: title = "Add door PIN"; break;
      case Screen::AddConfirm: title = "Confirm door PIN"; break;
      case Screen::Remove: title = "Remove door PIN"; break;
      case Screen::Change: title = "New admin PIN"; break;
      case Screen::ChangeConfirm: title = "Confirm admin PIN"; break;
      default: break;
    }
    row(0, title);
    row(1, screen == Screen::Door ? "PIN or # for admin" : "1-8 keys: 0-9, A-D");
    row(3, screen == Screen::Door ? "Locked" : "#:OK *:Clear/Back");
    lcd.cursor();
  }
  if (message) row(3, message);
  if (isInput()) lcd.setCursor(0, 2);
}

const char *resultMessage(PinResult result) {
  switch (result) {
    case PinResult::Success: return "Saved";
    case PinResult::Invalid: return "Use 1-8 PIN keys";
    case PinResult::Duplicate: return "PIN already in use";
    case PinResult::Full: return "PIN storage full";
    case PinResult::NotFound: return "PIN not found";
    case PinResult::StorageError: return "Save failed; retry";
    default: return "Admin required";
  }
}

KeyResult submit() {
  if (screen == Screen::Door && length == 0 && !overflowed) {
    show(Screen::AdminLogin);
    return KeyResult::None;
  }
  if (screen == Screen::Door || screen == Screen::AdminLogin) {
    const bool admin = screen == Screen::AdminLogin;
    const bool valid = !overflowed && (admin ? checkAdminPin(entry) : checkPin(entry));
    show(valid && admin ? Screen::Menu : screen);
    if (!valid) return KeyResult::AuthFailed;
    return admin ? KeyResult::AdminAuthenticated : KeyResult::DoorGranted;
  }
  if (overflowed || !isValidPin(entry)) {
    show(screen, "Use 1-8 PIN keys");
    return KeyResult::None;
  }
  if (screen == Screen::Setup || screen == Screen::Add || screen == Screen::Change) {
    memcpy(pending, entry, sizeof(pending));
    show(screen == Screen::Setup ? Screen::SetupConfirm :
         screen == Screen::Add ? Screen::AddConfirm : Screen::ChangeConfirm);
    return KeyResult::None;
  }
  if (screen == Screen::Remove) {
    const PinResult result = removePin(entry);
    show(Screen::Menu, result == PinResult::Success ? "PIN removed" : resultMessage(result));
    return KeyResult::None;
  }
  const bool setup = screen == Screen::SetupConfirm;
  if (strcmp(entry, pending) != 0) {
    wipe(pending, sizeof(pending));
    show(setup ? Screen::Setup : screen == Screen::AddConfirm ? Screen::Add : Screen::Change,
         "PINs do not match");
    return KeyResult::None;
  }
  const PinResult result = setup ? setupAdminPin(pending) :
                           screen == Screen::AddConfirm ? addPin(pending) : changeAdminPin(pending);
  wipe(pending, sizeof(pending));
  show(setup && result != PinResult::Success ? Screen::Setup : Screen::Menu, resultMessage(result));
  return setup && result == PinResult::Success ? KeyResult::AdminAuthenticated : KeyResult::None;
}
} // namespace

void initDisplay() {
  lcd.init();
  lcd.backlight();
  wipe(pending, sizeof(pending));
  const AuthState state = initAuth();
  show(state == AuthState::Ready ? Screen::Door :
       state == AuthState::NeedsAdmin ? Screen::Setup : Screen::StorageError);
}

void printMessage(const String &message) {
  row(3, message);
  if (isInput()) lcd.setCursor(length, 2);
}

void printLockout() { printMessage("Locked Out"); }

void resetDisplaySession() {
  wipe(pending, sizeof(pending));
  show(screen == Screen::StorageError ? Screen::StorageError :
       hasAdminPin() ? Screen::Door : Screen::Setup);
}

KeyResult handleDisplayKey(char key) {
  if (screen == Screen::StorageError) return KeyResult::None;
  if (screen == Screen::Menu) {
    switch (key) {
      case 'A': show(Screen::Add); break;
      case 'B': show(Screen::Remove); break;
      case 'C': show(Screen::Change); break;
      case 'D': resetDisplaySession(); break;
    }
    return KeyResult::None;
  }
  if (key == '*') {
    if (length || overflowed) {
      show(screen);
    } else {
      wipe(pending, sizeof(pending));
      if (screen == Screen::AdminLogin || screen == Screen::Door) show(Screen::Door);
      else if (screen == Screen::Setup || screen == Screen::SetupConfirm) show(Screen::Setup);
      else show(Screen::Menu);
    }
  } else if (key == '#') {
    return submit();
  } else if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D')) {
    if (length < MAX_PIN_LENGTH && !overflowed) {
      entry[length++] = key;
      lcd.write('*');
      lcd.setCursor(length, 2);
    } else {
      overflowed = true;
      printMessage("PIN too long");
    }
  }
  return KeyResult::None;
}
