1000 REM ----------------------------------------------
1001 REM               Horoscope Loader
1002 REM     by Sean P. Harrington, sph@1stage.com
1003 REM             Updated 18 APR 2024
1004 REM ----------------------------------------------
1005 REM     Based on the Horoscope Arcade Machine
1006 REM               (c) 1975 by Ramtek
1007 REM ----------------------------------------------

1100 _setup
1101 REM Setup system components
1110 PRINT "Loading Horoscope..."

1120 SET FAST ON
1130 USE CHRSET  "data/horoscope.chr"
1140 LOAD SCREEN "data/horoscope.scr"
1150 LOAD PT3    "data/theme.pt3"
1160 PLAY PT3
1170 SET  PT3 LOOP ON

1200 _init
1201 REM Initialize data structures
1210 dim c1(11):dim c2(11)
1211 REM Arrays to hold "PRESS ANY KEY TO BEGIN" screenlet
1220 load "data/tClick1.arry",*c1  : REM Text visible array
1230 load "data/tClick2.arry",*c2  : REM Text invisible array
1240 sc = 1                        : REM Click text visible

2000 _main
2001 REM Splash screen loop
2010 REM for i=1to500:next i:REM Debounce
2020 sc = val(right$(datetime$,2)) mod 2
2030 if sc=1 then put screen (9,20),*c1
2040 if sc=0 then put screen (9,20),*c2
2050 if key(-1) then goto _horoscope
2060 goto _main

9000 _horoscope
9001 REM Load Horoscope program
9010 RUN "data/horoscope.bas"
9999 END