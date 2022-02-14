Building Aqubasic from Source 
=============================

   file                purpose
aqubasic.asm     ROM initialisation, boot menu, extended BASIC commands
dos.asm          interface between BASIC and USB driver 
ch376.asm        USB driver

All the code was assembled with zmac, slightly modified to output only binary files. 

command: zmac.exe --zmac -n -I include aqubasic.asm

zmac creates a directory 'zout' for the binary output file. 

To create a ROM file with both the 'full' and 'lite' debugger versions, first define the variable 'aqubug' then assemble 'aqubasic.asm' to produce 'aqubasic.bin'. Rename it to 'aqubasic0.bin', undefine 'aqubug' and assemble again. Rename the new binary to 'aqubasic1.bin'. Finally run the batch file 'aqubasic.bat' which merges the two ROMs together to produce 'aqubasic10.rom'.

