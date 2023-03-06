#!/bin/bash
idf.py build

mkdir -p $1 $1/bootloader $1/partition_table
cp build/aquarius-plus.bin $1/
cp build/partition_table/partition-table.bin $1/partition_table/
cp build/bootloader/bootloader.bin $1/bootloader/
cp build/flash_args $1/
echo 'esptool.py --chip esp32s2 -p COM3 write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 bootloader/bootloader.bin 0x10000 aquarius-plus.bin 0x8000 partition_table/partition-table.bin' > $1/write.bat
unix2dos $1/write.bat
zip -rv $1.zip $1
rm -rf $1
