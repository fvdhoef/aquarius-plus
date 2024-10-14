1000 REM               ISS Location Mapper
1001 REM              by Sean P. Harrington
1002 REM               Updated 07 JUL 2024 
1003 REM -----------------------------------------------

1100 _INIT
1110 SET FAST ON
1120 SET FILE ERROR OFF
1130 WS$ = "https://api.wheretheiss.at/v1/satellites/25544"
1140 SET SPRITE * CLEAR
1150 SCREEN 1,2,1,1
1160 CLS
1170 LOAD PALETTE 1, "data/map.pal"
1180 MM$ = "00"
1190 MW$ = "http://worldtimeapi.org/api/timezone/Europe/London"
1200 CH  = VAL(MID$(DATETIME$,9,2))
1210 LH  = CH
1220 XL  = 151
1221 REM X value of left-most part of map area for plot
1230 YU  = 125
1231 REM Y value of upper-most part of map area for plot

1500 GOSUB _MSPRITE
1510 GOSUB _LOADSCR
1520 GOSUB _LOADMAP
1530 GOSUB _MAPHOUR

2000 _MAIN
2001 REM Main loop
2010 ON ERROR GOTO 4200
2020 LOAD WS$,@40,0
2030 ON ERROR GOTO 0
2040 GOSUB _PARSE

2050 _DRAWMAIN
2070 GOSUB _DRAWLL
2071 REM Draw LAT/LON at top of page
2080 CH  = VAL(MID$(DATETIME$,9,2))
2090 IF LH <> CH THEN GOSUB _MAPHOUR
2091 REM Compare last HOUR with current HOUR to check for redraw

2099 TIMER = 300
2100 _WAITHERE
2110 GOSUB _BLINK
2111 REM Blink ISS sprite every second
2120 IF TIMER THEN GOTO _WAITHERE
2130 if inkey$ <> "" THEN GOTO _closeout
2140 GOTO _main

2500 _BLINK
2501 REM Alternate sprite tile every second
2510 sc = val(right$(datetime$,2)) mod 2
2520 if sc=1 then DEF TILELIST SC$=288
2530 if sc=0 then DEF TILELIST SC$=289
2540 SET SPRITE S$ TILE SC$
2599 RETURN

3000 _MSPRITE
3001 REM This section defines the ISS ON sprite/icon
3010 S0$="........"
3011 S1$=".7.7...."
3012 S2$="..7....."
3013 S3$=".7.7...."
3014 S4$="........"
3015 S5$="........"
3016 S6$="........"
3017 S7$="........"
3020 SET TILE 288 TO ASC$(S0$+S1$+S2$+S3$+S4$+S5$+S6$+S7$)
3030 DEF TILELIST T$=288
3040 DEF ATTRLIST A$=64
3050 DEF SPRITE S$=0,0,0
3060 SET SPRITE S$ TILE T$
3070 SET SPRITE S$ ATTR A$
3080 SET SPRITE S$ ON

3101 REM This section defines the ISS OFF sprite/icon
3110 S0$="........"
3111 S1$=".F.F...."
3112 S2$="..F....."
3113 S3$=".F.F...."
3114 S4$="........"
3115 S5$="........"
3116 S6$="........"
3117 S7$="........"
3120 SET TILE 289 TO ASC$(S0$+S1$+S2$+S3$+S4$+S5$+S6$+S7$)

3999 RETURN

4000 _PARSE
4001 REM This section parses the JSON data
4010 SA=INMEM(@40,0,"lati")+ 10
4011 REM Start LAT
4020 EA=INMEM(@40,0,"long")- 10
4021 REM End LAT
4030 SO=INMEM(@40,0,"long")+ 11
4031 REM Start LON
4040 EO=INMEM(@40,0,"alti")- 10
4041 REM End LON
4050 LA$=PEEK$(@40,SA,EA-SA)
4060 LO$=PEEK$(@40,SO,EO-SO)
4061 REM Calc LAT & LON strings from RAM locations
4099 RETURN

4200 _NONET
4201 REM If no net for UTC pull, use Wichita, KS
4210 LA$="37.6872"
4220 LO$="-97.3301"
4299 GOTO _drawmain

4300 _DRAWLL
4301 REM Draw LAT LON values at top
4310 O3=5-INSTR(LO$,".")
4320 A3=5-INSTR(LA$,".")
4321 REM Find the decimal point to seed spaces before value
4330 POKE SCREEN 14+(1*40),"ISS Position"
4340 POKE SCREEN 13+(3*40),"LAT: "+STRING$(A3," ")+LA$+"    "
4350 POKE SCREEN 13+(4*40),"LON: "+STRING$(O3," ")+LO$+"    "
4351 REM Extra spaces after in case previously printed numbers are there
4360 X=INT((VAL(LO$)/1.875)+0.5)+XL
4361 REM X scaling for sprite location from LON
4370 Y=INT(0-(VAL(LA$)/1.600)+0.5)+YU
4371 REM Y scaling for sprite location from LAT
4380 SET SPRITE S$ POS X,Y
4399 RETURN

7000 _LOADSCR
7001 LOAD SCREEN "data/issmap.scr"
7099 RETURN

7100 MM$ = "00"
7101 REM In case UTC web load doesn't work, set to 00 file
7110 GOTO 7099

7500 _LOADMAP
7510 REM Load 1bpp map background
7520 LOAD BITMAP "data/map.bmp1"
7530 REM FILL BITMAP COLOR 11,13
7535 FILL BITMAP COLOR 4,2
7599 RETURN

7800 _MAPHOUR
7801 REM Shade map according to GMT hour (from net)
7810 LH = CH
7820 FILL COLORMAP (7,7)-(32,22) COLOR 4,2
7830 ON ERROR GOTO 7100
7840 LOAD MW$,@41,0
7850 ON ERROR GOTO 0
7860 UT=INMEM(@41,0,"utc_datetime")
7870 MM$ = PEEK$(@41,UT+26,2)

7880 FOR i = 8 to 31
7890 tm = (VAL(MM$) + 24 + (i - 20)) mod 24
7900 IF (tm < 6) OR (tm > 17) THEN FILL COLORMAP (i,7)-(i,22) COLOR 11,13
7901 REM POKE SCREEN 13+(5*40)," TM: " + STR$(tm)
7903 REM PAUSE
7910 NEXT i

7919 REM Fill left column
7920 tm = (VAL(MM$) + 24 + (8 - 20)) mod 24
7930 if (tm < 6) OR (tm > 17) THEN FILL COLORMAP (7,7)-(7,22) COLOR 11,13
7939 REM Fill right column
7940 tm = (VAL(MM$) + 24 + (31 - 20)) mod 24
7950 if (tm < 6) OR (tm > 17) THEN FILL COLORMAP (32,7)-(32,22) COLOR 11,13

7999 RETURN

9000 _closeout
9001 REM Exit program politely
9010 SET SPRITE * CLEAR
9020 LOAD PALETTE 1, "data/default.pal"
9030 SCREEN 1,0,0,0
9040 CLS
9050 PRINT "     We hope you enjoyed ISS-Map!"
9060 SET FAST OFF
9070 SET FILE ERROR ON

9999 END
