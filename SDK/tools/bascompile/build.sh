#!/bin/sh
set -e

./bascompile.py mandelbrot.txt mandelbrot.asm
zmac --zmac -n mandelbrot.asm --oo cim,lst
cp zout/mandelbrot.cim ../../../EndUser/sdcard/mandelbrot.caq
../txt2bas.py mandelbrot.txt ../../../EndUser/sdcard/mandelbrot.bas
../../../System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -u ../../../EndUser/sdcard -t "\nrun\"mandelbrot.caq\"\n"
