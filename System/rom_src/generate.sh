#!/bin/bash

set -e

make -C fpgarom
make -C boot

romfsgen/romfsgen.py romfs.bin boot/zout/boot.bin assets/default.chr assets/latin1b.chr assets/sysrom_s2.bin plusbasic/plusBasic/zout/sysrom.bin plusbasic/plusBasic/zout/ptplay.bin

ROMFS_H=../emulator/esp32/romfs_contents.h
xxd -i -n romfs_start romfs.bin > $ROMFS_H
sed 's/unsigned char/static const uint8_t/' $ROMFS_H > tmp.h
mv tmp.h $ROMFS_H
sed 's/unsigned int.*$//' $ROMFS_H > tmp.h
mv tmp.h $ROMFS_H

cp romfs.bin ../esp32/main/assets/

echo Done.
