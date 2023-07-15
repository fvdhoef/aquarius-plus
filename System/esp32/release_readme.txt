Firmware installation instructions
----------------------------------

There are 2 ways to install the firmware onto the Aquarius+ board:
- Option 1: if your Aquarius+ board already has a firmware installed, you can use the builtin update functionality
- Option 2: if your Aquarius+ does NOT have any firmware installed or you don't want to use the builtin update functionality


Option 1: use the builtin update functionality
----------------------------------------------
- Copy the aquarius-plus.bin file included in this archive to the root directory of a FAT32-formatted SD card.
- Insert the SD card into the Aquarius+ and power the system on.
- On the BASIC prompt type (and press ENTER):

RUN "esp:terminal.caq"

- This will start the builtin settings system, a ESP> prompt will appear. Here type (and press ENTER):

update

- This will show you the existing running version and the version on your SD card. If you want to update type (and press ENTER):

yes

- The system will indicate the progress and once it is done updating you can press ENTER to reboot the system with the new firmware.
- Enjoy your updated system!


Option 2: use the builtin update functionality
----------------------------------------------
This option is needed if your board doesn't have a firmware installed yet.

First you'll need to install the correct tools on your PC (or Mac).
First make sure your system has Python installed. This is usually already the case on Linux of macOS, but on Windows you probably need to install it first.
You can download the latest version from: https://www.python.org/downloads/

Open a command prompt in the directory of this archive.
If you haven't done before you have to install esptool first, which is the software used to program the ESP32 on your Aquarius+ board. This can be done using the command:

pip install esptool

After you have installed esptool you can proceed with updating your Aquarius+:

- Make sure your Aquarius+ system is turned off.
- Remove the keyboard (or any other device) connected to the USB-A port.
- If your device is in a case open the enclosure.
- Place 3 jumpers on JP1,JP2,JP3. (JP1/JP2 will connect the datalines of the micro-USB to the ESP32. JP3 will put the ESP32 in bootloader mode).
- Connect your Aquarius+ to your PC via a micro-USB cable.
- Turn on the Aquarius+ by switching the power switch in the ON position.

- Now on your PC:
  - On Windows run: write.bat
  - On macOS/Linux run: ./write.sh
- The update tool should find your board and start uploading the firmware to the ESP32.
  If this step fails on Windows modify the write.bat file: change COM3 to the correct port.
  You can use device manager to lookup which COM port number Windows assigned to the Aquarius+ connection.
- After the update tool finished, turn off the Aquarius+ by switching the power switch in the OFF position.

- Remove jumpers JP1,JP2,JP3.
- Reconnect a keyboard to the USB-A port.
- Switch on the system and check if it is working properly. If so, you can put the PCB back in an enclosure if you want.

- Enjoy your Aquarius+ system!
