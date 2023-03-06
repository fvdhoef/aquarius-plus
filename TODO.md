# TODO
*(not necessarily in this order)*

## Hardware + FPGA
- [ ] Enabling cartridge bus
- [ ] Implementing the front USB ports
- [x] Adding the second AY-3-8910 (not much work)
- [ ] Implementing tile+sprite engine
- [ ] Fixing board issues in PCB rev2

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
