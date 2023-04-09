all:
	rm -rf build
	mkdir -p build
	sdasz80 -o build/crt0.rel crt0.s
	sdasz80 -o build/putchar.rel putchar.s
	sdasz80 -o build/getchar.rel getchar.s
	sdcc -mz80 -c file_io.c -o build/
	sdcc -mz80 --code-loc 0x391B --data-loc 0 --no-std-crt0 build/crt0.rel build/putchar.rel build/getchar.rel build/file_io.rel main.c -o build/
	makebin -p -o 0x38E1 build/main.ihx build/esp_terminal.caq

run: all
	cp build/esp_terminal.caq ../../usb/
	../../../emulator/build/aquarius_emu -u ../../usb -t '\nrun"esp_terminal.caq"\n'
