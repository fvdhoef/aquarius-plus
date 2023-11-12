all:
	zmac --zmac -n --oo cim,lst aqmsrom.asm
	xxd -i zout/aqmsrom.cim > /Users/frank/Work/emulator_sms/aqmsrom.c

run: all
	make -C /Users/frank/Work/emulator_sms
	/Users/frank/Work/emulator_sms/build/smsemu
