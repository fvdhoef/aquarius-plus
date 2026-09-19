#!/bin/sh
set -e
ninja -C build
mkdir -p ~/Work/aquarius-plus/EndUser/sdcard/cores/aq32
cp build/shell.aq32 ~/Work/aquarius-plus/EndUser/sdcard/cores/aq32/shell.aq32
~/Work/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
