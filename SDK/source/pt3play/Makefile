
all:
	zmac --zmac -n aqubasic.asm

run: all
	cp zout/aqubasic.cim ~/Documents/AquariusPlusDisk/pt3play.caq
	~/Work/aquarius-plus/System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -t '\nrun"pt3play.caq"\n'
#	~/Work/aquarius-plus/System/emulator/build/AquariusPlusEmu.app/Contents/MacOS/aquarius_emu -c zout/aqubasic.cim
