all:
	zmac --zmac -n -c boot.asm --oo cim,lst
	cp zout/boot.cim zout/boot.bin

run: all
	(cd ..; ./generate.sh)
	make -C ../../emulator run
