#!/bin/sh

make -C settings
cp settings/build/settings.aqx settings/build/settings

romfsgen/romfsgen.py romfs.bin settings/build/settings

xxd -i -n romfs_start romfs.bin > ../emulator/romfs_contents.h
sed -i "" 's/unsigned char/static const uint8_t/' ../emulator/romfs_contents.h
sed -i "" 's/unsigned int.*$//' ../emulator/romfs_contents.h
