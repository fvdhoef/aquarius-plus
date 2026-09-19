100 REM Download latest plusBASIC System ROM
105 P$="getsysrom.baq"
110 CLS:PRINT CHR$(7);
120 PRINT "Warning: getsysrom.bas is deprecated."
125 PRINT
130 PRINT "Press Enter to run "+P$
140 PRINT "or any other key to abort."
150 K$=INKEY$:IF K$="" THEN GOTO 150
155 IF K$<>CHR$(13) THEN END
160 PRINT:PRINT "Running "+P$"..."
165 RUN "/util/"+P$
