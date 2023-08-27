#!/bin/sh
set -e

./bascompile.py mandelbrot.txt mandelbrot.asm
zmac --zmac -n mandelbrot.asm --oo cim,lst
cp zout/mandelbrot.cim ../../../EndUser/sdcard/mandelbrot.caq
