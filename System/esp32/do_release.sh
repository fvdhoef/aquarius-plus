#!/bin/bash
set -e

if [ "$1" != "nobuild" ]
then
  idf.py build
fi

NAME=firmware-`git describe`

# Remove temp directory and zip if existant
rm -rf $NAME $NAME.zip

# Create temp directory
mkdir -p $NAME $NAME/bootloader $NAME/partition_table

# Copy files into temp directory
cp release_readme.txt $NAME/README.txt
cp build/bootloader/bootloader.bin $NAME/bootloader/
cp build/aquarius-plus.bin $NAME/
cp build/partition_table/partition-table.bin $NAME/partition_table/
cp build/ota_data_initial.bin $NAME/
cp build/flash_args $NAME/

# Windows batch file
echo "esptool.py --chip esp32s3 -p COM3 write_flash `cat build/flash_args | tr '\n' ' '`" > $NAME/write.bat
unix2dos $NAME/write.bat

# macOS/Linux shell script
echo "#!/bin/sh" > $NAME/write.sh
echo "esptool.py --chip esp32s3 write_flash `cat build/flash_args | tr '\n' ' '`" >> $NAME/write.sh
chmod +x $NAME/write.sh

# Create ZIP file
zip -rv $NAME.zip $NAME

# Remove temp directory
rm -rf $NAME

echo Done!
