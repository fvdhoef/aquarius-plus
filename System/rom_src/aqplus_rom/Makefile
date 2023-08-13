all: s2

s2:
	zmac --zmac -n aqplus.asm --oo cim,lst
	./create_rom.py -patch ../../assets/aquarius_s2.rom

s3:
	make -C ../s3basic
	zmac --zmac -n aqplus.asm --oo cim,lst
	./create_rom.py ../s3basic/zout/s3basic.cim
