# TODO
Updated 30 AUG 2023

## In Process
### PlusBASIC
- [ ] Add PLAY command for 3 or 6 voice PT3 files
  - [ ] Assign @Page file for PT3 data
  - [ ] Allow psuedo-multitasking of playing sounds/music while other BASIC tasks are executing
  - [ ] Allow PLAY, PAUSE, STOP, RESTART directives

### Hardware + FPGA
- [ ] Allow other DB9-based control inputs to be used (Atari joystick, Amiga Mouse, Sega 6-button, etc.)
- [ ] Mini form factor
  - [X] Order prototype case
  - [X] Order prototype PCB
  - [x] Correct BOM & CPL and stage placement
- [ ] XL form factor
  - [ ] Develop PCB

### ESP32 + Z80
- [ ] Terminal program
  - [ ] FujiNet support
- [ ] Extend the system ROM with extra functionality:
  - [ ] Nice splash screen
  - [ ] Easy file navigation (like on the micro expander)
  - [ ] Built-in full-screen editor to edit text files and BASIC programs

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
- [x] Dump Hardware ROMs with bootkey combo
- [x] SCR dump
- [x] Support for extra keyboard keys
- [x] Hot-swappable SD cards
- [x] Hot pluggable keyboard USB
- [x] WiFi support (with time sync for correct file date/time)
- [x] Support for descrambling ROMs when we remove the scrambling from FPGA

### Soft/Firmware
 - [x] Load legacy ROMs
 - [X] PT3 Player
