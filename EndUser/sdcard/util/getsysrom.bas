100 REM Download latest plusBASIC System ROM
110 V$="v0.21n":IF VER(1)<VER(V$) GOTO _badv
115 S$="sysrom.bin"
120 ON ERROR GOTO _error
130 PRINT "Downloading %%..." % (S$)
135 LOAD "http://aqplus.ohiodivide.com/sdcard/sysrom.bin",@32,0
140 PRINT "Writing /%%..." % (S$)
145 SAVE "/sysrom.bin",@32,0,49152
150 PRINT "Press Ctrl-Esc to restart Aquarius+"
160 END
200 _error
210 M$="in line %%" % (ERRLINE)
220 IF ERRLINE=120 THEN M$="downloading sysrom.bin"
230 IF ERRLINE=130 THEN M$="writing sysrom.bin"
240 PRINT ERR$+" error "+M$
250 END
300 PRINT "This utility requires plusBASIC %% or higher" % (V$)
