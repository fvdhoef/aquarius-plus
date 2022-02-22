#!/usr/bin/env python3

with open("../../aquarius.rom", "wb") as f:
    # Patched Aquarius S2 ROM
    with open("../../assets/aquarius_s2.rom", "rb") as f2:
        stockrom = bytearray(f2.read())

        def set_jump(offset, target):
            stockrom[offset+0] = 0xC3
            stockrom[offset+1] = target & 0xFF
            stockrom[offset+2] = (target >> 8) & 0xFF

        def set_call(offset, target):
            stockrom[offset+0] = 0xCD
            stockrom[offset+1] = target & 0xFF
            stockrom[offset+2] = (target >> 8) & 0xFF

        # Reset entry point in ROM
        set_call(0x0000, 0x2000)

        # Common initialization
        set_call(0x0046, 0x2003)

        # Cold start entry point in ROM
        set_jump(0x010F, 0x2006)

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
