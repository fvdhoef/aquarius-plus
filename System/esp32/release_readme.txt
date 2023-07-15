Aquarius+ Firmware Installation Instructions
--------------------------------------------

There are 2 ways to install the firmware onto the Aquarius+ board:
- Option 1: if your Aquarius+ board already has a firmware installed, you can use the built-in update functionality.
- Option 2: if your Aquarius+ does NOT have any firmware installed (or you don't want to use the built-in update functionality), you will upload the firmware via USB from a computer.


Option 1: Use the built-in update functionality
----------------------------------------------
- Copy the aquarius-plus.bin file included in this archive to the root directory of a FAT32-formatted SD card.
- Insert the SD card into the Aquarius+ and power the system on.
- At the BASIC prompt type the text below, and press ENTER/RETURN:

RUN "esp:terminal.caq"

- This will start the built-in settings system, and an ESP> prompt will appear. Here type the text below, and press ENTER/RETURN:

update

- This will show you both the currently installed version and the version on your SD card. If you want to update, type the text below, and press ENTER/RETURN:

yes

- The system will update the system, indicating the progress. Once it is done updating, you can press ENTER/RETURN to reboot the system with the new firmware installed.
- Enjoy your updated system!


Option 2: Update from a computer via USB
----------------------------------------------
This option is needed if your board doesn't have a firmware installed yet (newly built Aquarius+ motherboards).

To start, you'll need to install the correct tools on your PC (or Mac).

First, make sure your system has Python installed. This is usually already installed on Linux and macOS, but on Windows you will usually need to manually install it (or update it).

You can download the latest version (Python 3 is fine) from: https://www.python.org/downloads/

Open a command prompt in the directory to which this archive was unzipped.

Next you have to install esptool, which is the software used to program the ESP32 on your Aquarius+ board. This can be done using the command:

pip install esptool

If you have previously uploaded firmware to your Aquarius+ before, esptool may already be installed. 

Now you can proceed with updating your Aquarius+:

- Make sure your Aquarius+ system is turned off.
- Remove the keyboard (or any other device) connected to the USB-A port.
- If your motherboard is already enclosed in a case, remove the screws and open the enclosure.
- Place 3 jumpers on JP1,JP2,JP3:
	- JP1/JP2 will connect the datalines of the micro-USB to the ESP32.
	- JP3 will put the ESP32 in bootloader mode.
- Connect your Aquarius+ to your PC via a micro-USB cable. The micro-USB end of the cable goes into the Aquarius+ power port, and the larger USB-A end plugs into your computer.
- Turn on the Aquarius+ by switching the power switch in the ON position. Note that the blue LED on the Aquarius+ will NOT turn on.

- Now on your computer:
  - On Windows run: write.bat
  - On macOS/Linux run: ./write.sh
- The update tool should find your board and start uploading the firmware to the ESP32.
  If this step fails on Windows modify the write.bat file: change COM3 to the correct port. You can use Windows Device Manager to lookup which COM port number Windows assigned to the Aquarius+ connection.
- After the update tool finished, turn off the Aquarius+ by turning its power switch to the OFF position.
- Remove jumpers JP1,JP2,JP3.
- Connect a USB keyboard to the USB-A port.
- Switch on the system and check if it is working properly. If the Aquarius+ is working properly, you can put the PCB back in the enclosure if you want.

- Enjoy your Aquarius+ system!
