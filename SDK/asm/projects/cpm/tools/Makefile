
# Detect OS
ifeq ($(OS),Windows_NT)
	OS = Windows
	AQPLUS_EMU_DISK ?= "$(USERPROFILE)/Documents/Aquarius+/Emulator/sdcard/"
	AQPLUS_EMU ?= "$(USERPROFILE)/Documents/Aquarius+/Emulator/aquarius-emu.exe"
else ifeq ($(shell uname -s),Darwin)
	OS = MacOS
	AQPLUS_EMU_DISK ?= $(HOME)/Documents/AquariusPlusDisk/
	AQPLUS_EMU ?= open -a AquariusPlusEmu --args
else ifeq ($(shell uname -s),Linux)
	OS = Linux
endif

DISKIMG = $(AQPLUS_SDK)/../EndUser/sdcard/cpm/disk-a.dsk

build:
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n aqmount.asm --oo cim,lst
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n aqdir.asm --oo cim,lst
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n aqcd.asm --oo cim,lst
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n aqcopy.asm --oo cim,lst

	# dd if=/dev/zero of=zout/disk.dsk bs=3328 count=77
	# mkfs.cpm -f ibm-3740 zout/disk.dsk
	cpmrm -f ibm-3740 $(DISKIMG) 0:AQMOUNT.COM
	cpmcp -f ibm-3740 $(DISKIMG) zout/aqmount.cim 0:AQMOUNT.COM
	cpmrm -f ibm-3740 $(DISKIMG) 0:AQDIR.COM
	cpmcp -f ibm-3740 $(DISKIMG) zout/aqdir.cim 0:AQDIR.COM
	cpmrm -f ibm-3740 $(DISKIMG) 0:AQCD.COM
	cpmcp -f ibm-3740 $(DISKIMG) zout/aqcd.cim 0:AQCD.COM
	cpmrm -f ibm-3740 $(DISKIMG) 0:AQCOPY.COM
	cpmcp -f ibm-3740 $(DISKIMG) zout/aqcopy.cim 0:AQCOPY.COM
	# cp zout/disk.dsk $(AQPLUS_EMU_DISK)

run: build
	make -C .. run_emu

clean:
	rm -rf zout
