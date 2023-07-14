all:
	rm -rf build
	mkdir -p build
	sdasz80 -o build/crt0.rel crt0.s
	sdasz80 -o build/putchar.rel putchar.s
	sdcc -mz80 -c file_io.c -o build/
	sdcc -mz80 --code-loc 0x391B --data-loc 0 --no-std-crt0 build/crt0.rel build/putchar.rel build/file_io.rel peaktop.c -o build/
	makebin -p -o 0x38E1 build/peaktop.ihx build/peaktop.caq

run: all
	cp build/peaktop.caq ../../../../usb/splash
	../../../../../build/aquarius_emu -u ../../../../usb -t '\nrun"peaktop.caq"\n'
