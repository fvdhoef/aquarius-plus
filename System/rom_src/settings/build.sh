#!/bin/sh

set -e
make
xxd -i -n settings_caq_start build/settings.caq > ../../emulator/settings_caq.h
sed -i "" 's/unsigned char/static const uint8_t/' ../../emulator/settings_caq.h
sed -i "" 's/unsigned int.*$//' ../../emulator/settings_caq.h
