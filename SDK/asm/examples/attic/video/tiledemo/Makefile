
all: tiledata.bin
	zmac --zmac -n tiledemo.asm --oo cim,lst
	cp zout/tiledemo.cim ../../../usb/tiledemo.caq
	cp tiledata.bin ../../../usb/

tiledata.bin: tiledemo.tmx
	../../../../tools/tmxconvert.py tiledemo.tmx tiledata.bin

run: all
	../../../../build/aquarius_emu -u ../../../usb -t '\nrun"tiledemo.caq"\n'
