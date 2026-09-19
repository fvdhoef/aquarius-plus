#!/bin/bash
set -e

if [ "$1" != "nobuild" ]
then
  idf.py build
fi

NAME=AquariusPlus-Firmware-`git describe`

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
cat <<EOT > $NAME/write.bat
IF [%1]==[] GOTO BLANK
esptool --chip esp32s3 -p COM%1 write_flash `cat build/flash_args | tr '\n' ' '`
ECHO Done!
GOTO DONE

:BLANK
ECHO Usage "write.bat 3" where 3 is the COM port (i.e. COM3).

:DONE
EOT
unix2dos $NAME/write.bat

# macOS/Linux shell script
cat <<EOT > $NAME/write.sh
#!/bin/sh
esptool.py --chip esp32s3 write_flash `cat build/flash_args | tr '\n' ' '`
EOT
chmod +x $NAME/write.sh

# Create ZIP file
zip -rv $NAME.zip $NAME

# Remove temp directory
rm -rf $NAME

echo Done!
