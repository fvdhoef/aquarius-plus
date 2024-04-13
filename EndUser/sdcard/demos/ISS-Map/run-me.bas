1000 REM               ISS Location Mapper
1001 REM              by Sean P. Harrington
1002 REM               Updated 13 APR 2024
1003 REM -----------------------------------------------

1100 _init
1110 SET FAST ON
1120 D=10
1130 SET SPRITE * CLEAR
1140 SCREEN 1,0,1,0
1150 CLS
1160 GOSUB _MSPRITE
1170 DIM D$(D)
1180 LOAD SCREEN "data/issmap.scr"
1190 ON ERROR GOTO _nonet

1200 WS$ = "http://api.open-notify.org/iss-now.json"
1210 LOAD WS$, *D$
1220 goto _main

1300 _nonet
1301 REM No network connection
1310 POKE SCREEN 13+(2*40),"! NO NETWORK !"
1320 WS$ = "data/iss-now.json"
1330 LOAD WS$, *D$

2000 _main
2001 REM Main loop
2010 LOAD WS$, *D$
2020 FOR I = 0 TO D
2030 IF LEN(D$(I)) THEN GOSUB _PARSE
2040 NEXT I
2050 TIMER = 300
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
4030 O1=INSTR(D$(I),"longitude")+13
4040 O2=INSTR(D$(I),"latitude")-4
4050 LO$=MID$(D$(I),O1,O2-O1)
4060 A1=INSTR(D$(I),"latitude")+12
4070 A2=INSTR(D$(I),"},")-1
4080 LA$=MID$(D$(I),A1,A2-A1)
4400 O3=5-INSTR(LO$,".")
4410 A3=5-INSTR(LA$,".")
4500 POKE SCREEN 14+(1*40),"ISS Position"
4510 POKE SCREEN 13+(3*40),"LAT: "+STRING$(A3," ")+LA$
4530 POKE SCREEN 13+(4*40),"LON: "+STRING$(O3," ")+LO$
4600 X=INT((VAL(LO$)/1.875)+0.5)+152
4610 Y=INT(0-(VAL(LA$)/1.600)+0.5)+126
4800 SET SPRITE S$ POS X,Y
4999 RETURN

9000 _closeout
9001 REM Exit program politely
9010 SET SPRITE * CLEAR
9020 SCREEN 1,0,0,0
9030 CLS
9040 PRINT "     We hope you enjoyed ISS-Map!"
9999 END