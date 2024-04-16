1000 REM Color Bitmap Demo
1001 REM Updated 12 APR 2024
1002 REM by Sean P. Harrington

1200 _init
1210 SCREEN 1
1220 LOAD SCREEN "data/color-title.scr"
1230 fb$ = "data/image"
1240 fe$ = ".bm4"
1299 PAUSE

2000 _main
2001 REM Set mem map to VRAM
2010 REM Files are in the data folder
2020 REM Filenames are currently image1.bm4 through image10.bm4
2030 REM Change the FOR loop below to add more filenames

2100 for n = 1 to 10
2110 fs$ = fb$ + TRIML$(STR$(n)," ") + fe$
2120 load fs$,@20,0
2130 gosub _process
2140 next n
2999 GOTO _closeout

3000 _process
3001 REM Loops and Subroutines
3150 GOSUB _setpal
3151 REM Set Video Reg to Color BM
3152 OUT 224,6
3153 PAUSE
3154 OUT 224,1
3155 RETURN

4000 _setpal
4001 REM Set palette
4100 FOR I=0TO31
4200 OUT 234,I+32
4300 OUT 235, PEEK(@20,16000+I)
4400 NEXT
4500 RETURN

9000 _closeout
9100 CLS
9999 END