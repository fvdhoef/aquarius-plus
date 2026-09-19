#!/bin/sh
set -e
ninja -C build
(cd help/content; ./convert.py)
cp build/basic.aq32 ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/
cp help/content/basic.hlp ~/Projects/aquarius-plus/EndUser/sdcard/cores/aq32/
~/Projects/aquarius-plus/System/emulator/build/aqplus-emu -t ' run aq32.core\n'
