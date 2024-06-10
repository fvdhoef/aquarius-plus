1000 REM               ISS Location Mapper
1001 REM              by Sean P. Harrington
1002 REM               Updated 10 JUN 2024 
1003 REM -----------------------------------------------

1100 _init
1110 SET FAST ON
1120 D=20
1130 SET SPRITE * CLEAR
1140 SCREEN 1,0,1,0
1150 CLS
1160 GOSUB _MSPRITE
1170 DIM D$(D)
1180 LOAD SCREEN "data/issmap.scr"

1200 WS$ = "http://api.open-notify.org/iss-now.json"
1210 LOAD WS$, ^J$
1220 SPLIT J$ INTO *D$ DEL 34
1230 I=INDEX(*D$,"timestamp")
1240 IF I=0 THEN GOTO _nonet
1299 goto _main

1300 _nonet
1301 REM Exit program politely
1310 SET SPRITE * CLEAR
1320 SCREEN 1,0,0,0
1330 CLS
1340 PRINT "Sorry, ISS-Map requires"
1350 PRINT "an Internet connection!"
1360 PRINT
1399 END

2000 _main
2001 REM Main loop
2010 LOAD WS$, ^J$
2020 SPLIT J$ into *D$ DEL 34
2030 GOSUB _PARSE
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
4010 O=INDEX(*D$,"longitude")
4020 A=INDEX(*D$,"latitude")
4030 LO$=D$(O+2)
4040 LA$=D$(A+2)
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