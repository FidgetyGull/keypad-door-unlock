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

## Instructions

On first boot, enter an admin PIN, press `#`, then enter it again and press `#`
to confirm. This opens the admin menu so you can add your first door PIN.
The admin PIN and door PINs persist across power loss and normal firmware uploads.
An existing PIN from the older single-PIN firmware is preserved during setup.

PINs contain 1-8 characters: `0`-`9` or `A`-`D`. Entries are masked on the 20x4
LCD. Press `#` to submit. Press `*` to clear an entry; press `*` with an empty
entry to go back or cancel. New PINs must be entered twice. Duplicate PINs,
including using the admin PIN as a door PIN, are rejected.

At the normal door screen, enter a door PIN followed by `#` to unlock. To manage
PINs, press `#` with an empty entry, then enter the admin PIN followed by `#`.
The admin PIN is only for administration and does not unlock the door.

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

There is no admin PIN recovery or keypad reset. A forgotten admin PIN requires
erasing the ESP32 flash/NVS and reflashing; a normal upload alone preserves NVS
and will not reset the PIN. Erasing NVS also deletes all door PINs. Unreadable or
invalid credential storage disables access instead of reopening first-boot setup.

The physical door actuator is still a TODO in `unlock()` in `src/main.cpp`.

### Validation

Build the firmware with `pio run`. Windows host regression tests can be run with
`./test/host/run.ps1` using MinGW `g++`. They exercise the production authentication,
display, and keypad handling against simulated NVS, LCD, keypad, and time, with
Windows CNG providing SHA-256 and random bytes. Hardware LCD/keypad and power-loss
testing still requires the ESP32.
