1000 REM      Animated Screens Loader
1001 REM       by Sean P. Harrington
1002 REM        updated 18 APR 2024
1010 REM ----------------------------------
1011 REM This simple application loads SCR
1012 REM files in a loop. It's a brute-
1013 REM force method of animating, but it
1014 REM works for simple paged animation.

1100 _init
1110 SCREEN 1
1120 USE CHRSET "data/default.chr"
1130 set fast on
1140 CLS

2000 _mainloop
2001 REM Animate frames
2010 LOAD SCREEN "data/TVTEST01.SCR"
2020 LOAD SCREEN "data/TVTEST02.SCR"
2030 LOAD SCREEN "data/TVTEST03.SCR"
2040 LOAD SCREEN "data/TVTEST04.SCR"
2100 REM Wait for key press
2110 IF INKEY$="" THEN GOTO _mainloop

9000 _closeout
9010 CLS
9999 END
