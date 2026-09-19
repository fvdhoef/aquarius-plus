100 REM Extra TAR file
105 SET FNKEY 3 TO \"run xtar.bas\r"
110 PG=32:AD=0:REM Start Page, Address
120 DB=0:REM Debug Flag
125 DF$="FS$=%%, SO$=%%, SZ=%%, SB=%%, FT=%%, PG=%%, AD=%%"
300 INPUT "FILENAME";TN$:IF TN$="" THEN END:REM TAR file name
305 IF FILEEXT$(TN$)="" THEN TN$=TN$+".tar"
310 _LOAD:PRINT "Loading ";TN$:PRINT:LOAD TN$,@PG,AD
400 _LOOP:IF TRIM$(PEEK$(@PG,AD,255))="" THEN END
410 FS$=TRIM$(PEEK$(@PG,AD,100)):REM FileSpec
415 SO$=TRIM$(PEEK$(@PG,AD+124,12)):REM File Size in Octal
420 SZ=0:FOR I=1 TO LEN(SO$):SZ=SZ*8+VAL(SO$[I]):NEXT:REM File Size
425 SB=SZ:PD=SZ MOD 512:IF PD THEN SB=SB+512-PD:REM Padded File Size
430 FT=PEEK(@PG,AD+156)
435 IF DB THEN PRINT DF$ % (FS$,SO$,SZ,SB,FT,PG,AD)
440 AD=AD+512:IF AD>16383 THEN PG=PG+1:AD=AD-16384
450 IF FT=53 THEN GOSUB _DIRECTORY
455 IF FT=0 OR FT=48 THEN GOSUB _EXTRACT
460 AD=AD+SB:IF AD>16383 THEN PG=PG+1:AD=AD-16384
465 GOTO _LOOP
500 _EXTRACT:
510 PRINT "Extracting ";FS$
520 SAVE FS$,@PG,AD,SZ
590 RETURN
600 _DIRECTORY:
610 FS$=TRIMR$(FS$,"/")
620 ON ERROR GOTO 640
630 MKDIR FS$
640 IF ERR AND ERR<>54 THEN PRINT ERR$;" error":END
650 IF ERR THEN RESUME
655 ON ERROR GOTO 0
690 RETURN
900 REM From https://en.wikipedia.org/wiki/Tar_(computing)
910 REM Header (512 bytes) Octal fields terminated with space or NUL
911 REM Offset  Size  Field
912 REM    0     100  File name
913 REM  100       8  File mode (octal)
914 REM  108       8  Owner's numeric user ID (octal)
915 REM  116       8  Group's numeric user ID (octal)
916 REM  124      12  File size in bytes (octal)
917 REM  136      12  Last modification time in octal Unix time format (octal)
918 REM  148       8  Checksum for header record
919 REM  156       1  Link indicator (file type)
920 REM  157     100  Name of linked file
921 REM  257     255  Padding (ASCII NUL)
923 REM Link indicator field
924 REM Value	Meaning
925 REM  '0'  (or ASCII NUL) Normal file
926 REM  '1'	Hard link
927 REM  '2'	Symbolic link
930 REM Checksum
931 REM Sum of the unsigned byte values of the header record with the eight
932 REM checksum bytes taken to be ASCII spaces (decimal value 32).`
933 REM Stored as a six digit octal number with leading zeroes followed by
934 REM a NUL and then a space.
935 REM Various implementations do not adhere to this format. In addition,
936 REM some historic tar implementations treated bytes as signed.
937 REM Implementations typically calculate checksum both ways, and treat as
938 REM good if either signed or unsigned sum matches the included checksum.
940 REM File data
941 REM 512 contiguous 512 byte blocks
