# TODO
Updated 08 AUG 2023

## In Process

### Hardware + FPGA
- [ ] Allow other DB9-based control inputs to be used (Atari joystick, Amiga Mouse, Sega 6-button, etc.)
- [ ] Mini form factor 
  - [ ] Order and test PCB (correct BOM & CPL)
- [ ] XL form factor
  - [ ] Develop PCB

### ESP32 + Z80
- [ ] Support for extra keyboard keys
- [ ] Extending the system ROM with extra functionality:
  - [ ] Nice splash screen
  - [ ] Easy file navigation (like on the micro expander)
  - [ ] Built-in full-screen editor to edit text files and BASIC programs

### Soft/Firmware
 - [ ] Dumping of Hardware ROMs with bootkey combo
 - [ ] Rework of SYSROM to include subset of MX BASIC 2.0 commands
 - [ ] Emulator enhancements, UI

## Complete

### Hardware + FPGA
- [x] Fixing board issues in PCB rev2
  - [x] Wire in Spartan 6, remove unecessary power regulators, remove EPROM 
  - [x] Rework BOM for manufacturing 
- [x] Enabling cartridge bus
- [x] Add second AY-3-8910
- [x] Implement tile+sprite engine

### ESP32 + Z80
- [x] Hot pluggable keyboard USB
- [x] WiFi support (with time sync for correct file date/time)
- [x] Support for descrambling ROMs when we remove the scrambling from FPGA

### Soft/Firmware
 - [x] Loading of legacy ROMs
 - [X] PT3 Player
