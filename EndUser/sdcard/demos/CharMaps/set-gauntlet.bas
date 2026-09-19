1000 REM Load ARABIC.CHR font
1010 CLS
1020 USE CHRSET "charmaps/gaunt.chr"
1040 LOAD "chrdemo.scr",12288
1050 LOCATE 1,2
1060 PRINT "This is the"
1070 PRINT "Arabic"
1080 PRINT "font...
1090 PRINT
1100 PRINT "Press any key"
1110 PRINT "to continue..."
1500 REM Wait for key press
1510 K=GETKEY
1520 LOCATE 1,20
1530 END
