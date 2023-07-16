AQPLUS_SDK  ?= ../..
OUTFILE      = tiledemo.caq
INC_DIRS    += .
SRC_DIRS    += .

DEPS          += assets/tiledata.bin
FILES_TO_COPY += assets/tiledata.bin

.DEFAULT_GOAL := all

assets/tiledata.bin: assets/tiledemo.tmx
	../../tools/tmxconvert.py $< $@

include $(AQPLUS_SDK)/lib/Makefile.common
