all:
	zmac --zmac -n --oo cim,lst aqmsrom.asm

run: all
	xxd -i zout/aqmsrom.cim > /Users/frank/Projects/emulator_sms/aqmsrom.c
	make -C /Users/frank/Projects/emulator_sms
	/Users/frank/Projects/emulator_sms/build/smsemu
