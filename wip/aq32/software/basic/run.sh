#!/bin/sh
set -e
ninja -C build
(cd help/content; ./convert.py)
cp build/basic.aq32 ~/Work/aquarius-plus/EndUser/sdcard/cores/aq32/
cp help/content/basic.hlp ~/Work/aquarius-plus/EndUser/sdcard/cores/aq32/
~/Work/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
