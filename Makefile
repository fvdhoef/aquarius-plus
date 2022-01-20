all:
	mkdir -p build
	gcc -Wall -Ilibz80 -o build/aquarius_emu main.c libz80/z80.c assets/charset.c `sdl2-config --cflags --libs`

run: all
	build/aquarius_emu
