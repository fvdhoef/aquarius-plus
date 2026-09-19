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

OUTFILE = gocpm.aqx
FILES_TO_COPY = zout/$(OUTFILE)

all:
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n cpm22.asm --oo cim,lst
	zmac -I $(AQPLUS_SDK)/asm/inc --zmac -n cpmloader.asm --oo cim,lst
	mv zout/cpmloader.cim zout/$(OUTFILE)

run_emu: all
	@echo Copying $(FILES_TO_COPY) to $(AQPLUS_EMU_DISK)/cpm
	@cp $(FILES_TO_COPY) "$(AQPLUS_EMU_DISK)/cpm"
	@echo Starting $(OUTFILE) in emulator
	@$(AQPLUS_EMU) -u "$(AQPLUS_EMU_DISK)" -t "\ncd\"CPM\"\n run\"$(OUTFILE)\"\n"

run_aqp: all
	@$(foreach NAME,$(FILES_TO_COPY), \
		echo Uploading $(NAME); \
		curl -T $(NAME) http://$(AQPLUS_HOST)/; \
	)
	@echo Running $(OUTFILE)
	@printf '\x1E\nRUN"$(OUTFILE)"\n' | curl --data-binary @- http://$(AQPLUS_HOST)/keyboard
