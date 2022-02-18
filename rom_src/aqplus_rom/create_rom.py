#!/usr/bin/env python3

with open("../../aquarius.rom", "wb") as f:
    # Patched Aquarius S2 ROM
    with open("../../assets/aquarius_s2.rom", "rb") as f2:
        stockrom = bytearray(f2.read())

        # Cold start entry point in ROM
        stockrom[0x010F] = 0xC3   # JP $2000
        stockrom[0x0110] = 0x00
        stockrom[0x0111] = 0x20

        # Warm start entry point in ROM
        stockrom[0x00ED] = 0xC3   # JP $2003
        stockrom[0x00EE] = 0x03
        stockrom[0x00EF] = 0x20

        f.write(stockrom)

    # Extended ROM
    with open("zout/aqplus.cim", "rb") as f2:
        f.write(f2.read())

    # Aquarius Character ROM
    with open("../../assets/AquariusCharSetS2.bin", "rb") as f2:
        f.write(f2.read())

    # CP437 Character ROM
    with open("../../assets/font8x8.bin", "rb") as f2:
        f.write(f2.read())
