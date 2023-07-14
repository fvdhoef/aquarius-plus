all:
	rm -rf build
	mkdir -p build
	sdasz80 -o build/crt0.rel crt0.s
	sdasz80 -o build/putchar.rel putchar.s
	sdcc -mz80 --code-loc 0x391B --data-loc 0 --no-std-crt0 build/crt0.rel build/putchar.rel sdcc_test.c -o build/
	makebin -p -o 0x38E1 build/sdcc_test.ihx build/sdcc_test.caq

run: all
	cp build/sdcc_test.caq ../../usb/
	../../../emulator/build/aquarius_emu -u ../../usb -t '\nrun"sdcc_test.caq"\n'
