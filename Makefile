all:
	mkdir -p build
	ln -sf ../aquarius.rom build/aquarius.rom
	gcc -O2 -Wall -Wextra -Ilibz80 -o build/aquarius_emu main.c audio.c ay8910.c ch376.c libz80/z80.c assets/charrom.c `sdl2-config --cflags --libs`

run: all
	build/aquarius_emu
