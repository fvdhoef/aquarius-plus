#!/bin/bash

set -e

make -C fpgarom
make -C boot

romfsgen/romfsgen.py romfs.bin boot/zout/boot.bin assets/default.chr assets/latin1b.chr assets/sysrom_s2.bin plusbasic/plusBasic/zout/sysrom.bin plusbasic/plusBasic/zout/ptplay.bin

xxd -i -n romfs_start romfs.bin > ../emulator/romfs_contents.h
sed 's/unsigned char/static const uint8_t/' ../emulator/romfs_contents.h > tmp.h
mv tmp.h ../emulator/romfs_contents.h
sed 's/unsigned int.*$//' ../emulator/romfs_contents.h > tmp.h
mv tmp.h ../emulator/romfs_contents.h

echo Done.
