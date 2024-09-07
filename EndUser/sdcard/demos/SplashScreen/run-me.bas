1000 REM AqPlus Tile Scroll Demo
1010 REM Set starting tile positions
1020 X=0:Y=0
1030 REM Set Tilemap HScroll Registers
1040 M=225:N=226
1090 REM Load tile data into Page 20
1100 LOAD"data/ss.til",@20,0
1120 LOAD"data/ss.pal",-24576
1130 REM Set palette loop
1140 FOR I=0TO31
1150 REM Update VPALSEL
1160 OUT 234,I
1170 REM Update VPALDATA
1180 OUT 235,PEEK(-24576+I)
1190 NEXT
1200 REM Set VCTRL to 64x32 tilemap mode
1210 OUT 224,2
1220 REM Scroll tileset
1230 X=X+1:IFX>511THENX=0
1240 Q=XAND255:R=X/256:OUTM,Q:OUTN,R
1250 REM Uncomment next 2 lines for diag
1260 Y=Y+1:IFY>255THENY=0
1270 OUT227,Y
1280 IF INKEY$="" GOTO 1220
1290 REM Set VCTRL to text mode
1300 OUT 224,1
1310 RUN"data/reset-pal.baq"
