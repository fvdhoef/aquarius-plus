all:
	zmac --zmac -n -c fpgarom.asm --oo cim,lst

	xxd -i -n fpgarom_start zout/fpgarom.cim > ../../emulator/fpga_cores/aqplus/fpgarom.h
	sed 's/unsigned char/static const uint8_t/' ../../emulator/fpga_cores/aqplus/fpgarom.h > tmp.h
	mv tmp.h ../../emulator/fpga_cores/aqplus/fpgarom.h
	sed 's/unsigned int.*$$//' ../../emulator/fpga_cores/aqplus/fpgarom.h > tmp.h
	mv tmp.h ../../emulator/fpga_cores/aqplus/fpgarom.h

run: all
	make -C ../../emulator run
