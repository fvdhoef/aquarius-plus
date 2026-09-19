#!/bin/sh
set -e
./genrom.py ../../../assets/aquarius_s2.rom rom.v
./genrom.py ../../../assets/AquariusCharacterSet.bin charrom.v
