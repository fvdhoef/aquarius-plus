#!/bin/bash

set -e

if [[ $(date +%s -r root_certs) -lt $(date +%s --date="7 days ago") ]]
then
curl -o root_certs https://ccadb.my.salesforce-sites.com/mozilla/IncludedRootsPEMTxt?TrustBitsInclude=Websites
dos2unix root_certs
fi

make -C fpgarom
make -C boot

cp ../fpga_cores/aqplus/aqp_top.bit aqplus.core

romfsgen/romfsgen.py \
    romfs.bin \
    boot/zout/boot.bin \
    assets/default.chr \
    assets/latin1b.chr \
    assets/sysrom_s2.bin \
    plusbasic/plusBasic/zout/sysrom.bin \
    plusbasic/plusBasic/zout/ptplay.bin \
    aqplus.core \
    root_certs

rm -rf aqplus.core

ROMFS_H=../emulator/esp32/romfs_contents.h
xxd -i -n romfs_start romfs.bin > $ROMFS_H
sed 's/unsigned char/static const uint8_t/' $ROMFS_H > tmp.h
mv tmp.h $ROMFS_H
sed 's/unsigned int.*$//' $ROMFS_H > tmp.h
mv tmp.h $ROMFS_H

cp romfs.bin ../esp32/main/assets/

echo Done.
