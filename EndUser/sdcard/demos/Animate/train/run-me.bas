100 REM Toy Train Animation Demo
110 GOSUB _init:GOSUB _load:GOSUB _setup

200 COPY !A(F),L(F) TO @V,V(F) FAST
220 PAUSE P
230 F=F+1:IF F>19 THEN F=0
290 GOTO 200

300 REM Start

600 _init:
605 F=0:V=20:P=0: 'Frame#, Video RAM Page, Pause Length
610 DIM V(19):READ *V: 'Video RAM Offset
620 DIM L(19):READ *L: 'Clip Length
630 DIM A(19):READ *A: 'Extended RAM Address
640 READ P$:SET PALETTE 1 TO ASC$(P$)
690 RETURN

700 _load:
710 IF PEEK$(!112,16)=$"02222222234444444443333222222220" THEN RETURN
720 PRINT "Loading animation"
730 SET SPEED 3
740 LOAD "train.bin",!0
750 RETURN

800 _setup:
810 SET SPEED 0
820 SCREEN 0,3
830 CLEAR BITMAP
890 RETURN

910 DATA 7120,10400,9840,8640,7760,6960,6400,6000,5760,5680
915 DATA 5680,5760,5760,5760,5840,6080,6320,6800,7520,8400
920 DATA 8800,5200,5760,6960,7840,8160,7680,6640,5440,4320
925 DATA 3360,2480,2880,3760,4720,5760,6960,7760,7840,7120
930 DATA 0,8800,16384,22144,32768,40608,49152,56832,65536,70976,75296
935 DATA 78656,81920,84800,88560,98304,104064,114688,122448,131072
940 DATA FF0F77088709AA0ACC0C64078908640A6606930AB50C000911021005110D000F
