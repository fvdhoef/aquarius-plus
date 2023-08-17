# TODO
Updated 17 AUG 2023

## In Process

### Hardware + FPGA
- [ ] Allow other DB9-based control inputs to be used (Atari joystick, Amiga Mouse, Sega 6-button, etc.)
- [ ] Mini form factor
  - [ ] Order prototype case
  - [ ] Order prototype PCB
  - [x] Correct BOM & CPL and stage placement
- [ ] XL form factor
  - [ ] Develop PCB

### ESP32 + Z80
- [ ] Support for extra keyboard keys
- [ ] Hot-swappable SD cards
- [ ] Extend the system ROM with extra functionality:
  - [ ] Nice splash screen
  - [ ] Easy file navigation (like on the micro expander)
  - [ ] Built-in full-screen editor to edit text files and BASIC programs
- [x] Dump Hardware ROMs with bootkey combo
- [x] SCR dump

### Emulator
- [ ] Built-in User & Developer documentation
- [ ] Built-in editor for SCR (screen RAM) files
- [ ] Built-in editor/convertor for BASIC files
- [ ] Built-in editor for CHR (character) files
- [x] Config file to save prefs/locations
- [x] Floating windows for Memory, Registers, Interrupts

## Complete

### Hardware + FPGA
- [x] Fix board issues in PCB rev2
  - [x] Wire in Spartan 6, remove unnecessary power regulators, remove EPROM 
  - [x] Rework BOM for manufacturing 
- [x] Enable cartridge bus
- [x] Add second AY-3-8910
- [x] Implement tile+sprite engine

### ESP32 + Z80
- [x] Hot pluggable keyboard USB
- [x] WiFi support (with time sync for correct file date/time)
- [x] Support for descrambling ROMs when we remove the scrambling from FPGA

### Soft/Firmware
 - [x] Load legacy ROMs
 - [X] PT3 Player
