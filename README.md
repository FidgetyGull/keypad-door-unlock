# Keypad-Enabled ESP32 Door Unlocker
## Overview
An esp32-powered keypad and door unlocker. Fits onto a lever-style door handle, requiring you to push on the door to actually open it (planned automatic door open as well).<br>
More information coming soon!

## Features
- ESP32-enabled
- Individually salted SHA-256 PIN hashes stored in ESP32 NVS
- Automatic lockout after unsuccessful attempts
- Integrated I2C screen with custom messages
- First-boot admin setup and keypad PIN management (up to 20 door PINs)

### Planned Features
- Guest/temporary codes
- Tamper alarm
- Automatic door open

## Hardware requirements
- ESP32 or equivalent
- 4x4 keypad (must have 1-9 AND A-D with `*` and `#`)
- 20x4 I2C LCD
- Servo, based on door force needs.



## Instructions

On first boot, enter an admin PIN, press `#`, then enter it again and press `#`
to confirm. This opens the admin menu so you can add your first door PIN.
The admin PIN and door PINs persist across power loss and normal firmware uploads.

PINs contain 1-8 characters: `0`-`9` or `A`-`D`. Entries are masked on the 20x4
LCD. 
- Press `#` to submit. 
- Press `*` to clear an entry; press `*` with an empty
entry to go back or cancel. 
- New PINs must be entered twice. 
- Duplicate PINs, including using the admin PIN as a door PIN, are rejected.

Unlocking door:
1. Enter a door PIN followed by `#`

Entering admin menu:
1. Press `#` with empty entry
2. Enter admin pin followed by `#`
- NOTE: The admin PIN is only for administration and does not unlock the door.

| Admin menu key | Action |
| --- | --- |
| `A` | Add a door PIN, then confirm it |
| `B` | Enter the door PIN to remove, then press `#` |
| `C` | Set and confirm a new admin PIN |
| `D` | Exit to the door screen |

These letter commands apply at the menu; inside a PIN entry, letters remain PIN
characters. The admin PIN cannot be removed through the remove-door-PIN action.
Ten seconds of inactivity clears pending entries, logs the admin out, and turns
off the backlight. The next key wakes the screen without entering a character.
Three failed door/admin authentication attempts cause a shared 30-second lockout.

WARNING: A forgotten admin PIN requires complete erasure and re-flashing of the esp32.
There is no recovery method included.

The physical door actuator is still a TODO in `unlock()` in `src/main.cpp`.