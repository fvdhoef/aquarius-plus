#!/bin/sh
set -e
make -C ../../../rom_src/fpgarom
./genrom.py ../../../rom_src/fpgarom/zout/fpgarom.cim rom.v
