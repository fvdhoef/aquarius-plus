all:
	zmac --zmac -n -c boot.asm --oo cim,lst
	curl -X PUT -T zout/boot.cim http://aqplus-minivz/cores/aqplus_v2/boot.bin
