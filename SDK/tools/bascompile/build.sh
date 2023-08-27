#!/bin/sh
set -e

./bascompile.py test.txt test.asm
zmac --zmac -n test.asm --oo cim,lst
cp zout/test.cim ../../../EndUser/sdcard/test.caq
