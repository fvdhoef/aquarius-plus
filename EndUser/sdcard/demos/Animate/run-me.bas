100 REM Animated Screen Loader
110 CLS
130 REM Frame Start
131 LOAD SCREEN "scr/tvtest01.scr"
132 LOAD SCREEN "scr/tvtest02.scr"
133 LOAD SCREEN "scr/tvtest03.scr"
134 LOAD SCREEN "scr/tvtest04.scr"
140 REM Wait for key press
150 IF INKEY$="" GOTO 130
160 CLS
170 END
