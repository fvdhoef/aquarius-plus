#!/bin/sh
set -e
ninja -C build
cp build/bitmapdemo.aq32 ~/Work/aquarius-plus/EndUser/sdcard/cores/aq32/
~/Work/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
