
all: tiledata.bin
	zmac --zmac -n tiledemo.asm --oo cim,lst
	cp zout/tiledemo.cim ../../../usb/tiledemo.caq
	cp tiledata.bin ../../../usb/

tiledata.bin:
	./convert.py

run: all
	../../../../build/aquarius_emu -u ../../../usb -t '\nrun"tiledemo.caq"\n'
