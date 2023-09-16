#!/bin/sh

set -e
make
xxd -i -n settings_aqx_start build/settings.aqx > ../../emulator/settings_aqx.h
sed -i "" 's/unsigned char/static const uint8_t/' ../../emulator/settings_aqx.h
sed -i "" 's/unsigned int.*$//' ../../emulator/settings_aqx.h
