@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

set filename="qwt-6.1.2"

REM Automatic code begins here
set dirname=%filename%

REM Unpack or checkout
echo.
echo ======================
echo Unpack libQWT
echo ======================
if exist %dirname% rd /s/q %dirname%
mkdir %dirname%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %filename%.zip -y > nul

echo.
echo ======================
echo Patch libQWT
echo ======================
..\ThirdParty\patch\doit.exe -p0 < %filename%.patch

REM Build
echo.
echo ======================
echo Building 64-bit libQWT
echo ======================
call setHopsanBuildPaths.bat
rd /s/q %dirname%_shb
mkdir %dirname%_shb
cd %dirname%_shb
qmake ../%dirname%/qwt.pro -r -spec win32-g++
mingw32-make -j4

echo.
echo Done
pause