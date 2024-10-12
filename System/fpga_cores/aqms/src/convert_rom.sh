#!/bin/sh
make -C ../rom_src
./genrom.py ../rom_src/zout/aqmsrom.cim rom.v
