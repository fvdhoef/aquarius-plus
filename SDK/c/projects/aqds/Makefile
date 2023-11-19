AQPLUS_SDK  ?= ../..
AQPLUS_HOST ?= aqplus
AQPLUS_EMU_DISK = $(AQPLUS_SDK)/../EndUser/sdcard

# Detect OS
ifeq ($(OS),Windows_NT)
	OS = Windows
	AQPLUS_EMU_DISK ?= "$(USERPROFILE)/Documents/Aquarius+/Emulator/sdcard/"
	AQPLUS_EMU ?= "$(USERPROFILE)/Documents/Aquarius+/Emulator/aquarius-emu.exe"
else ifeq ($(shell uname -s),Darwin)
	OS = MacOS
	AQPLUS_EMU_DISK ?= $(HOME)/Documents/AquariusPlusDisk/
	AQPLUS_EMU ?= open -a AquariusPlusEmu --args
else ifeq ($(shell uname -s),Linux)
	OS = Linux
endif

OUTFILE = aqds.aqx
FILES_TO_COPY = zout/$(OUTFILE) editor/build/editor.bin fileman/build/fileman.bin

all:
	make -C editor
	make -C fileman
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n aqds.asm --oo cim,lst
	mv zout/aqds.cim zout/$(OUTFILE)

run_emu: all
	@echo Copying files to $(AQPLUS_EMU_DISK)
	@mkdir -p $(AQPLUS_EMU_DISK)/aqds
	@cp $(FILES_TO_COPY) "$(AQPLUS_EMU_DISK)/aqds"
	@echo Starting $(OUTFILE) in emulator
	@$(AQPLUS_EMU) -u "$(AQPLUS_EMU_DISK)" -t "\nrun\"aqds/$(OUTFILE)\"\n"

run_aqp: all
	@$(foreach NAME,$(FILES_TO_COPY), \
		echo Uploading $(NAME); \
		curl -T $(NAME) http://$(AQPLUS_HOST)/aqds/; \
	)
	@echo Running $(OUTFILE)
	@printf '\x1E\n\x1DRUN"aqds/$(OUTFILE)"\n' | curl --data-binary @- http://$(AQPLUS_HOST)/keyboard
