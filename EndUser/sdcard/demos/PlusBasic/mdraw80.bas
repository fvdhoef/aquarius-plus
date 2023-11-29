1000 REM        Mouse Draw 80 Column
1010 REM         by Curtis F Kaylor
1020 REM       and Sean P Harrington
1030 REM ----------------------------------
1040 REM Uses the mouse to draw bloxels on
1050 REM the screen in 80 column mode.
1060 REM ----------------------------------
1500 _SETUP
1501 REM Setup routines
1510 SET FAST ON
1511 REM Turn Turbo mode on
1520 SET SPRITE * CLEAR
1521 REM Clear all sprites
1530 SCREEN 3,,1,0,0
1531 REM Set screen to 80 column mode and give sprites priority
1532 REM (screen txt-mode, gfx_mode, sprites, txt-priority, border-remap)
1540 CLS 15,7
1541 REM Clear screen to dark grey on white
1550 POKE PEEK($3801),32
1560 POKE COLOR 0, STRING$(80,$70)
1561 REM Add white on black menubar at top
1570 POKE $3001,"MouseDraw80"
1580 POKE $3049,"Q=Quit"
1590 REM Define the pencil cursor sprite
1591 REM 0 is transparent, 1 through F are pixel colors
1600 T1$ = $"FFF00000"
1610 T2$ = $"FD3F0000"
1620 T3$ = $"F333F000"
1630 T4$ = $"0F333F00"
1640 T5$ = $"00F333F0"
1650 T6$ = $"000F3F5F"
1660 T7$ = $"0000F55F"
1670 T8$ = $"00000FF0"
1680 TX$ = T1$+T2$+T3$+T4$+T5$+T6$+T7$+T8$
1682 REM Concatenate the cursor pixels into one tile string
1690 SET TILE 511 TO TX$
1691 REM Update tile 511 to the pixel pattern
1700 DEF SPRITE M$ = 0,0,0
1701 REM Define the cursor sprite
1710 DEF TILELIST T$=511
1711 REM Define the tilelist for the sprite
1720 SET SPRITE M$ TILE T$ POS 156,96 ON
1721 REM Update the cursor sprite tile and position, and turn it on
2000 _MAIN
2001 REM Main Loop
2010 X = MOUSEX
2011 REM Update variable X to be the mouse x position
2020 Y = MOUSEY
2021 REM Update variable Y to be the mouse y position
2030 B = MOUSEB
2031 REM Update variable B to be the mouse button value
2040 SET SPRITE M$ POS X,Y
2041 REM Update the cursor sprite to the mouse location
2050 IF Y<8 THEN GOTO _MAIN
2051 REM If the mouse is in the menubar, don't do anything
2060 IF B=1 THEN PSET(X/2,(Y-8)*3/8)
2061 REM If the left button is pushed, draw a bloxel
2070 IF B=2 THEN PRESET(X/2,(Y-8)*3/8)
2071 REM If the right button is pushed, erase a bloxel
2080 IF INKEY$="q" OR INKEY$="Q" THEN GOTO _CLOSEOUT
2081 REM Scan keyboard for QUIT
2999 GOTO _MAIN
9000 _CLOSEOUT
9001 REM Closeout program
9010 SCREEN 1,,0,0,0
9011 REM Set screen back to normal mode
9020 CLS
9021 REM Clear screen
9030 SET FAST OFF
9031 REM Turn Turbo mode off
9999 END
