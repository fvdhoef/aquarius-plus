all:
	zmac --zmac -n -c fpgarom.asm --oo cim,lst

	xxd -i -n fpgarom_start zout/fpgarom.cim > ../../emulator/fpgarom.h
	sed -i "" 's/unsigned char/static const uint8_t/' ../../emulator/fpgarom.h
	sed -i "" 's/unsigned int.*$$//' ../../emulator/fpgarom.h

run: all
	make -C ../../emulator run
