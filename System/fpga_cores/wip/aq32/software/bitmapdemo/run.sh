#!/bin/sh
set -e
ninja -C build
cp build/bitmapdemo.aq32 ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/
~/Projects/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
