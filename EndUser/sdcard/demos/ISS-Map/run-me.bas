1000 REM               ISS Location Mapper
1001 REM              by Sean P. Harrington
1002 REM               Updated 14 JUN 2024 
1003 REM -----------------------------------------------

1100 _INIT
1110 SET FAST ON
1120 WS$ = "https://api.wheretheiss.at/v1/satellites/25544"
1130 SET SPRITE * CLEAR
1140 SCREEN 1,0,1,0
1150 CLS
1160 MS$ = "data/issmap-"
1170 MM$ = "00"
1180 ME$ = ".scr"
1190 MW$ = "http://worldtimeapi.org/api/timezone/Europe/London"
1200 CH  = VAL(MID$(DATETIME$,9,2))
1210 LH  = CH
1220 XL  = 152
1221 REM X value of left-most part of map area for plot
1230 YU  = 126
1231 REM Y value of upper-most part of map area for plot

1500 GOSUB _MSPRITE
1510 GOSUB _LOADMAP

2000 _MAIN
2001 REM Main loop
2010 ON ERROR GOTO 4200
2020 LOAD WS$,@40,0
2030 ON ERROR GOTO 0
2040 GOSUB _PARSE

2050 _DRAWMAIN
2060 GOSUB _DRAWLL
2061 REM Draw LAT/LON at top of page
2070 CH  = VAL(MID$(DATETIME$,9,2))
2080 IF LH <> CH THEN GOSUB _LOADMAP
2081 REM Compare last HOUR with current HOUR to check for redraw

2099 TIMER = 300
2100 _WAITHERE
2110 IF TIMER THEN GOTO _WAITHERE
2120 if inkey$ <> "" THEN GOTO _closeout
2130 GOTO _main

3000 _MSPRITE
3001 REM This section defines the ISS sprite/icon
3010 S0$="7.7....."
3011 S1$=".7......"
3012 S2$="7.7....."
3013 S3$="........"
3014 S4$="........"
3015 S5$="........"
3016 S6$="........"
3017 S7$="........"
3020 SET TILE 0 TO ASC$(S0$+S1$+S2$+S3$+S4$+S5$+S6$+S7$)
3030 DEF SPRITE S$=0,0,0
3040 SET SPRITE S$ ON
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

7000 _LOADMAP
7001 REM Load MAP SCR file according to GMT hour (from net)
7010 LH = CH
7020 ON ERROR GOTO 7100
7030 LOAD MW$,@41,0
7040 ON ERROR GOTO 0
7050 UT=INMEM(@41,0,"utc_datetime")
7060 MM$ = PEEK$(@41,UT+26,2)
7070 LOAD SCREEN MS$+MM$+ME$
7099 RETURN

7100 MM$ = "00"
7101 REM In case UTC web load doesn't work, set to 00 file
7110 GOTO 7070

9000 _closeout
9001 REM Exit program politely
9010 SET SPRITE * CLEAR
9020 SCREEN 1,0,0,0
9030 CLS
9040 PRINT "     We hope you enjoyed ISS-Map!"

9999 END
