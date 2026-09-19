#!/bin/sh
set -e
ninja -C build
mkdir -p ~/Work/aquarius-plus/EndUser/sdcard/cores/aqua-8
cp build/aqua-8.bin ~/Work/aquarius-plus/EndUser/sdcard/cores/aqua-8/aqua-8.bin
~/Work/aquarius-plus/System/emulator/build/aqplus-emu
