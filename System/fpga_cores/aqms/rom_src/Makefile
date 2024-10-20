all:
	zmac --zmac -n --oo cim,lst aqmsrom.asm

run: all
	xxd -i zout/aqmsrom.cim > /Users/frank/Work/emulator_sms/aqmsrom.c
	make -C /Users/frank/Work/emulator_sms
	/Users/frank/Work/emulator_sms/build/smsemu
