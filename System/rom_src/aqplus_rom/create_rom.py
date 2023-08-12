#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser(description="Generate Aquarius+ system ROM")
parser.add_argument("base_rom", help="Base ROM")
parser.add_argument("-patch", help="Patch base ROM", action="store_true")
args = parser.parse_args()


with open("../../emulator/aquarius.rom", "wb") as f:
    # Patched Aquarius S2 ROM
    with open(args.base_rom, "rb") as f2:
        stockrom = bytearray(f2.read())

        def set_jump(offset, target):
            stockrom[offset + 0] = 0xC3
            stockrom[offset + 1] = target & 0xFF
            stockrom[offset + 2] = (target >> 8) & 0xFF

        if args.patch:
            # Reset entry point in ROM
            set_jump(0x0000, 0x2000)

            # Cold start entry point in ROM
            set_jump(0x010F, 0x2003)

            # Cartridge start
            set_jump(0x007F, 0x2006)

            # Patch to keep scramble register at $00 in BASIC
            stockrom[0x0156] = 0xAF

        f.write(stockrom)

    # Extended ROM
    with open("zout/aqplus.cim", "rb") as f2:
        f.write(f2.read())

    # Aquarius Character ROM
    with open("../../assets/AquariusCharacterSet.bin", "rb") as f2:
        f.write(f2.read())

    # CP437 Character ROM
    with open("../../assets/font8x8.bin", "rb") as f2:
        f.write(f2.read())
