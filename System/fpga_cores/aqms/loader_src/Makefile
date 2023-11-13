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

OUTFILE = goaqms.aqx
FILES_TO_COPY = zout/$(OUTFILE) zout/aqms.core

all:
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n goaqms.asm --oo cim,lst
	mv zout/goaqms.cim zout/$(OUTFILE)
	cp ../top.bit zout/aqms.core

run_emu: all
	@echo Copying $(FILES_TO_COPY) to $(AQPLUS_EMU_DISK)/cores/aqms
	@cp $(FILES_TO_COPY) "$(AQPLUS_EMU_DISK)/cores/aqms"
	@echo Starting $(OUTFILE) in emulator
	@$(AQPLUS_EMU) -u "$(AQPLUS_EMU_DISK)" -t "\ncd\"cores/aqms\"\n run\"$(OUTFILE)\"\n"

run_aqp: all
	@$(foreach NAME,$(FILES_TO_COPY), \
		echo Uploading $(NAME); \
		curl -T $(NAME) http://$(AQPLUS_HOST)/cores/aqms/; \
	)
	@echo Running $(OUTFILE)
	@printf '\x1E\n\x1Dcd\"cores/aqms"\nrun"$(OUTFILE)"\n' | curl --data-binary @- http://$(AQPLUS_HOST)/keyboard
