@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

set filename="qwt"

REM Automatic code begins here
set dirname=%filename%

echo.
echo ======================
echo Patch libQWT
echo ======================
REM ..\ThirdParty\patch\doit.exe -p0 < %filename%.patch

REM Build
echo.
echo ======================
echo Building 64-bit libQWT
echo ======================
call setHopsanBuildPaths.bat
REM rd /s/q %dirname%_shb
REM mkdir %dirname%_shb
cd %dirname%
qmake qwt.pro -r -spec win32-g++
mingw32-make -j4

echo.
echo Done
pause