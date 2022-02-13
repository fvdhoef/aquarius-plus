#!/usr/bin/env python3

with open("../../assets/aquarius_s2.rom", "rb") as f:
    stockrom = bytearray(f.read())

with open("zout/aqubasic.cim", "rb") as f:
    extrom = f.read()

# Patch stock ROM

# Cold start entry point in ROM
stockrom[0x00FD] = 0xC3   # JP $2000
stockrom[0x00FE] = 0x00
stockrom[0x00FF] = 0x20

# Warm start entry point in ROM
stockrom[0x00ED] = 0xC3   # JP $2003
stockrom[0x00EE] = 0x03
stockrom[0x00EF] = 0x20

with open("../../aquarius.rom", "wb") as f:
    f.write(stockrom)
    f.write(extrom)
