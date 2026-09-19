@REM Build CP/M and copy into SD card distro
@REM for dev environments without MAKE support
zmac -I ../../../asm/inc --zmac -n cpm22.asm --oo cim,lst
@IF %ERRORLEVEL% NEQ 0 GOTO END
zmac -I ../../../asm/inc --zmac -n cpmloader.asm --oo cim,lst
@IF %ERRORLEVEL% NEQ 0 GOTO END
copy zout\cpmloader.cim zout\gocpm.aqx
@IF %ERRORLEVEL% NEQ 0 GOTO END
copy zout\gocpm.aqx ..\..\..\..\EndUser\sdcard\cpm
@IF %ERRORLEVEL% EQU 0 ECHO Build complete.
@IF "%AquaPlus%" NEQ "" copy zout\cpmloader.cim %AquaPlus%\sdcard\cpm
:END
