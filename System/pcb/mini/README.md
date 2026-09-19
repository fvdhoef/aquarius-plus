# Aquarius<sup>+</sup> Mini
A reduced-cost, small form factor version of the **Aquarius<sup>+</sup>**, designed as an entry point for 8-bit enthusiasts who want physical hardware.

## Introduction
The **Aquarius<sup>+</sup> Mini** retains all of the components relevant to enjoying the platform, but removes legacy hardware components that are more specific to collectors of original **Aquarius Computer** such as the cartridge and cassette ports. In November of 2024, the **Aquarius<sup>+</sup> Mini vZ** was introduced - "vZ" stands for Virtual Z80 - removing the physical Z80 chip and moving it into the FPGA along with the virtualized AY sound chips and the logic from the original PLA1 and PLA2 from the original **Aquarius Computer** architecture.

## PCB Revisions
 * **rev1** (aka Mini vZ) - Current release platform
   * Introduced: 22 NOV 2024
 * **rev0** - Z80-based hardware
   * Retired: 22 NOV 2024
   * Introduced: 24 FEB 2024

## Platform Comparison
| Feature | Aq<sup>+</sup> Mini | Aq<sup>+</sup> Standard |
| ---- | :----: | :----: |
| 512k RAM |  ✅  |  ✅  |
| 16k VRAM for Bitmaps, Tiles, and Sprites | ✅ | ✅ |
| 2 @ AY-3-8910 Sound Chips | In FPGA | In FPGA |
| Z80 CPU | In FPGA*, up to 28Mhz | Hardware, up to 7Mhz<br>In FPGA, up to 28Mhz |
| SD Card | ✅ | ✅ |
| Cassette Ports | :x: | ✅ |
| WiFi | ✅ | ✅ |
| Bluetooth LE | ✅ | ✅ |
| Soft loading of Cartridge and Cassette images | ✅ | ✅ |
| Expansion/Cartridge Port | :x: | ✅ |
| Wireless Game Controller Access | ✅ | ✅ |
| Wired Hand Controller Ports | :x: | ✅ |

*As of **rev1**, the **Aquarius<sup>+</sup> Mini** no longer relies on a physical Z80 CPU to function, moving it instead into the FPGA.

