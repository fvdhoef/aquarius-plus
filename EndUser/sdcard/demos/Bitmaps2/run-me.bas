1000 REM Color Bitmap Demo
1001 REM Updated 12 APR 2024
1002 REM by Sean P. Harrington

1200 _init
1210 SCREEN 1
1220 LOAD SCREEN "data/COLOR-TITLE.SCR"
1230 fb$ = "data/image"
1240 fe$ = ".bm4"
1299 PAUSE
1300 SCREEN 0,3

2000 _main
2001 REM Set mem map to VRAM
2010 REM Files are in the data folder
2020 REM Filenames are currently image1.bm4 through image10.bm4
2030 REM Change the FOR loop below to add more filenames

2100 for n = 1 to 10
2110 fs$ = fb$ + TRIML$(STR$(n)," ") + fe$
2120 LOAD BITMAP fs$
2130 pause
2140 next n
2999 GOTO _closeout

9000 _closeout
9010 SCREEN 1,0
9100 CLS
9999 END