.PHONY: all sysrom

all:
	zmac --zmac -n bootrom.asm --oo cim,lst
	./create_rom.py zout/bootrom.cim ../../fpga/src/bootrom.v

sysrom:
	./create_rom.py ../../assets/aquarius_s2.rom ../../fpga/src/bootrom.v
