1000 REM ----------------------------------------------
1001 REM                 Horoscope Menu
1002 REM     by Sean P. Harrington, sph@1stage.com
1003 REM              Updated 20 OCT 2024
1004 REM ----------------------------------------------
1005 REM     Based on the Horoscope Arcade Machine
1006 REM               (c) 1975 by Ramtek
1007 REM ----------------------------------------------
1008 REM     New algorithms and planetary routines 
1009 REM             from Paul Schlyter's
1010 REM         Computing Planetary Positions -
1011 REM        A Tutorial with Worked Examples
1012 REM http://www.stjarnhimlen.se/comp/tutorial.html
1013 REM            pausch@stjarnhimlen.se
1014 REM            http://stjarnhimlen.se
1015 REM ----------------------------------------------

1100 _setup
1101 REM Setup system components
1110 SCREEN 1,0
1159 REM Maintain Track Play status
1160 mu = trackstatus

1200 _init
1201 REM Initialize data structures

1900 _loadmenu
1901 REM Load main menu assets
1910 CLS 2,0
1920 SCREEN 1,3,,1,
1930 LOAD BITMAP "data/red_back.bmp4"
1940 LOAD SCREEN "data/horomenu_blk.scr"

2000 _mainmenu
2001 REM Main Menu loop
2010 PAUSE
2020 if key("q") then goto _closeout  : REM q = Quit
2030 if key("a") then goto _about     : REM a = About
2040 if key("m") then goto _toggleMu  : REM m = Toggle music
2050 if key("1") then goto _futura    : REM 1 = Futura
2060 if key("2") then goto _birth     : REM 2 = Birth
2070 if key("3") then goto _aspects   : REM 3 = Aspects
2080 if key("4") then goto _stars     : REM 4 = Stars
2090 if key("5") then goto _biorhythm : REM 5 = Biorhythm
2999 goto _mainmenu

3100 _futura
3101 REM Load Futura program
3105 goto _mainmenu : REM Delete me when Futura works
3110 RUN "data/futura.bas"
3199 END

3200 _birth
3201 REM Load Birth program
3205 goto _mainmenu : REM Delete me when Birth works
3210 RUN "data/birth.bas"
3299 END

3300 _aspects
3301 REM Load Aspects program
3305 goto _mainmenu : REM Delete me when Aspects works
3310 RUN "data/aspects.bas"
3399 END

3400 _stars
3401 REM Load Stars program
3405 goto _mainmenu : REM Delete me when Stars works
3410 RUN "data/stars.bas"
3499 END

3500 _biorhythm
3501 REM Load Biorhythm program
3510 LOAD SCREEN "data/bioLoad_blk.scr"
3520 RUN "data/biorhythm.bas"
3599 END

4000 _about
4001 REM Show About... pages
4002 LOAD BITMAP "data/arcade.bmp4"
4010 LOAD SCREEN "data/horoAbout_blk.scr"
4020 PAUSE
4030 goto _loadmenu

4999 RETURN

5000 _toggleMu
5001 REM Toggle Music
5010 mu = NOT mu
5020 if     mu then RESUME TRACK
5030 if not mu then PAUSE  TRACK
5040 goto _mainmenu

9000 _closeout
9001 REM Closeout system and reset
9010 STOP TRACK
9020 SET FAST OFF
9030 USE CHRSET 0

9900 _quit
9901 REM Finish out
9910 CLS
9920 PRINT "    We hope you enjoyed Horoscope!"
9930 PRINT
9999 END