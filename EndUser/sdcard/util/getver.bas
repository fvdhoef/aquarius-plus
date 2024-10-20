100 REM Check and display plusBASIC version information
110 CV$=VER$(1)
120 LOAD "http://aquarius.plus/pbver.php",^LV$
130 PRINT "Current Version: ";CV$
140 PRINT "Newest Version:  ";LV$
