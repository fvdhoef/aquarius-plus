1000 REM            One Big Sprite
1010 REM          by Curtis F Kaylor
1020 REM --------------------------------------
1030 REM Program creates a sprite from sprtiles
1040 REM that are defined using text strings,
1050 REM and then moves the large sprite from
1060 REM left to right on the screen.
1070 REM --------------------------------------

1500 _setup
1501 REM Setup routines
1510 SET SPRITE * CLEAR
1511 REM Clear all sprites
1520 SCREEN ,,1,,
1521 REM Turn sprites on (screen txt-mode, gfx_mode, sprites, txt-priority, border-remap)
1530 CLS 6,0
1531 REM Clear screen to cyan text on black background
1540 LOCATE 1,11
1541 REM Move text cursor to middle of the screen
1550 PRINT "Building big sprite..."
1560 T1$="11111111"
1561 REM Define top & bottom bit pattern of tile
1570 T2$="10000001"
1571 REM Define middle bit pattern of tile
1580 T$=ASC$(T1$+T2$+T2$+T2$+T2$+T2$+T2$+T1$)
1581 REM Combine bit patterns into whole tile
1590 FOR T=0 TO 63
1591 REM Create a loop to define 64 tiles
1600 SET TILE T TO T$
1601 REM Set tile to T$ pattern
1610 NEXT T
1611 REM Loop through 64 tiles
1620 A$=STRING$(64,0)
1621 REM Create sprite attributes variable
1630 S$=$"408080"
1631 REM Create sprite definition variable
1640 T$=""
1641 REM Create sprtile definition variable
1650 FOR I=0 TO 7
1651 REM Outer VERTICAL layout loop
1660 FOR J=0 TO 7
1661 REM Inner HORIZONTAL layout loop
1670 T=I*8+J
1671 REM Current layout location value
1680 S$=S$+CHR$(T)+CHR$(J*8)+CHR$(I*8)
1681 REM Concatenate new location to end of sprite definition
1690 T$=T$+CHR$(T)+CHR$(0)
1691 REM Concatenate next tile to end of sprtile definition
1700 NEXT J
1701 REM Advance inner HORIZONTAL position
1710 NEXT I
1711 REM Advance outer VERTICAL position

2000 _main
2001 REM Main Program
2010 LOCATE 1,12
2011 REM Move down a line
2020 PRINT "Moving sprite..."
2030 SET SPRITE S$ ATTR A$ TILE T$ POS 0,0 ON
2031 REM Create sprite and turn it on
2040 FOR X=0 TO 320-64
2041 REM Move sprite loop, left to right minus sprite width
2050 SET SPRITE S$ POS X,0
2060 NEXT X
2070 LOCATE 1,13
2071 REM Move down a line
2080 PRINT "Press any key to quit..."
2090 _keywait
2091 REM Wait for keypress loop
2100 IF INKEY$="" then goto _keywait
2110 GOTO _closeout
2999 END

9000 _closeout
9001 REM Closeout routintes
9010 SCREEN ,,0,,
9011 REM Turn sprites off
9020 CLS
9021 REM Clear screen
9999 END
