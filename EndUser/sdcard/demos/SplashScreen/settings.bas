1000 REM AqPlus Tile Scroll Demo
1010 REM Set starting tile positions
1020 X=0:Y=0
1030 REM Set Tilemap HScroll Registers
1040 M=225:N=226
1090 REM Load tile data into Page 20
1100 LOAD TILEMAP "data/settings.map"
1110 LOAD TILESET INDEX 128, "data/settings.til"
1120 SCREEN 0, 1, 0, 1, 0
1130 REM LOAD "data/settings.bin",@20
1140 REM LOAD "data/settings.nxm",@20
1150 REM LOAD "data/settings.nxt",@20,$800
1199 LOAD PALETTE 1,"data/settings.pal"
1200 REM Set VCTRL to 64x32 tilemap mode
1210 REM OUT 224,2
1220 REM Scroll tileset
1230 X=X+1:IFX>511THENX=0
1240 Q=XAND255:R=X/256:OUTM,Q:OUTN,R
1250 REM Uncomment next 2 lines for diag
1260 Y=Y+1:IFY>255THENY=0
1270 OUT227,Y
1280 IF INKEY$="" GOTO 1220
1300 LOAD PALETTE 0,"data/default.pal"
1310 REM LOAD PALETTE 1,"data/default.pal"
1320 SCREEN 1, 0, 0, 1, 0
