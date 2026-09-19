1000 REM Horoscope & Biorhythm
1100 SET FAST ON
1200 _init
1500 _setup
1510 REM Leap Year Function
1511 DEF FN LY(YR)=ABS(SGN(YR MOD 4)-SGN(YR MOD 100)+SGN(YR MOD 400)-1)
2000 _main
2100 load screen "data/horoscope.scr"
9000 pause
9100 cls
9110 PRINT "Leap Years Since 1880:"
9115 PRINT
9120 for i=1880 to 2024
9130 if (FN LY(i)) then print i;"    ";
9140 next i
9150 set fast off
