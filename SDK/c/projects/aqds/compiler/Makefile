
all:
	gcc -Istubs -Wall -Wextra -g -O0 -o compiler main.c tokenizer.c common.c parser.c expr.c symbols.c stubs/file_io.c

test: all
	./compiler testdata/test.cb
	../assembler/assembler testdata/out/test.asm

emu: test
	cp testdata/out/out/test.aqx ../../../../../EndUser/sdcard/
	../../../../../System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -t "\n   RUN\"test.aqx\"\n"
