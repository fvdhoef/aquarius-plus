# TODO

## Hardware + FPGA
- [ ] Fixing board issues in PCB rev2
  - [ ] Wire in Spartan 6, remove unecessary power regulators, remove EPROM 
  - [ ] Rework BOM for manufacturing 
- [ ] Enabling cartridge bus
- [x] Adding the second AY-3-8910 (not much work)
- [x] Implementing tile+sprite engine
- [ ] Allow other DB9-based control inputs to be used (Atari joystick, Amiga Mouse, Sega 6-button, etc.)

## ESP32 + Z80
- [x] Hot pluggable keyboard USB
- [ ] Support for extra keyboard keys
- [ ] Front USB port handling
- [ ] WiFi support (with time sync for correct file date/time)
- [x] Support for descrambling ROMs when we remove the scrambling from FPGA
- [ ] Extending the system ROM with extra functionality:
  - [ ] Nice splash screen
  - [ ] Easy file navigation (like on the micro expander)
  - [ ] Built-in music player
  - [ ] Built-in full-screen editor to edit text files and BASIC programs

## Soft/Firmware
 - [x] Loading of legacy ROMs
 - [ ] Rework of SYSROM to include subset of MX BASIC 2.0 commands
