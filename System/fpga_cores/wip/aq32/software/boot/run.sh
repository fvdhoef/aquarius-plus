#!/bin/sh
set -e
ninja -C build
mkdir -p ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32
cp build/boot.bin ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/boot.aq32
~/Projects/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
