#!/bin/sh
zmac --zmac -n -I include aqubasic.asm
../../genrom.py zout/aqubasic.cim ../../uexp_rom.v
sed -i -e 's/module rom/module uexp_rom/g' ../../uexp_rom.v
sed -i -e 's/\[12:/\[13:/g' ../../uexp_rom.v
sed -i -e "s/13'h/14'h/g" ../../uexp_rom.v
