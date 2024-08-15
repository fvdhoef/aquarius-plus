#!/bin/sh

set -e
make
xxd -i -n settings_aqx_start build/settings.aqx > ../../emulator/settings_aqx.h
sed 's/unsigned char/static const uint8_t/' ../../emulator/settings_aqx.h > tmp.h
mv tmp.h ../../emulator/settings_aqx.h
sed 's/unsigned int.*$//' ../../emulator/settings_aqx.h > tmp.h
mv tmp.h ../../emulator/settings_aqx.h
