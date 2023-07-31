![Aquarius+ Logo](EndUser/images/aquarius_plus_logo_BLUE.png)

**Next-generation Z80-based retro computer system**

## Abstract:

What would the Aquarius have evolved into if Mattel hadn't abandoned the system back in 1984? The Aquarius+ is the next-generation of Z80-based 8-bit hardware for the Aquarius platform. The intent is to deliver a cost-effective, backwards-compatible retro computer system that both developers and nostalgists will love and use, all in the spirit of the original, orphaned Aquarius platform.

![Aquarius Case](System/case/case%20v25%20crop.png)

## Features:

- Backward compatibility with existing Aquarius hardware and software
- Modern, flexible video solution that supports bitmapped graphics, sprites, and tiles, delivered through clean VGA output
- Dual AY sound chips (virtual) for six sound voices
- 512kb of paged RAM in four 16k banks
- Reworked SYSROM and programmable CHARROM
- Integrated SD card and commands (remove dependency on cassette loading)
- WiFi connectivity
- Bluetooth connectivity for modern game controllers

## Folder structure

| Directory           | Description                                                                                                                                                                     |
| ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [EndUser](EndUser/) | These files are meant for the normal end-user, that just wants to play and work with the system. You can find some nice sample programs, games and music to run on system here. |
| [Maker](Maker/)     | Files required for makers to manufacture and assemble an Aquarius+ system.                                                                                                      |
| [SDK](SDK/)         | If you want to develop your own software for the Aquarius+, look here.                                                                                                          |
| [System](System/)   | This contains all other files related to the system, including: Schematics/PCB, FPGA, ESP32, Emulator                                                                           |
