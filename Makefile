all:
	mkdir -p build
	ln -sf ../aquarius.rom build/aquarius.rom
	gcc -Wall -Ilibz80 -o build/aquarius_emu main.c libz80/z80.c assets/charrom.c `sdl2-config --cflags --libs`

run: all
	build/aquarius_emu
