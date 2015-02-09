@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

set filename="qwt-6.1.2"

REM Automatic code begins here
set dirname=%filename%
set dirname64=%filename%_x64

REM Unpack or checkout
echo.
echo ======================
echo Unpack libQWT
echo ======================
rd /s/q %dirname%
rd /s/q %dirname64%
mkdir %dirname%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %filename%.zip -y > nul

echo.
echo ======================
echo Patch libQWT
echo ======================
..\ThirdParty\patch\doit.exe -p0 < %filename%.patch

REM Copy to 64-bit dir
echo.
echo ======================
echo Copying to %dirname64%
echo ======================
robocopy /e /NFL /NDL /NJH /NJS /nc /ns /np  %dirname% %dirname64%

REM Build
echo.
echo ======================
echo Building 64-bit libQWT
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64
rd /s/q %dirname64%_shb
mkdir %dirname64%_shb
cd %dirname64%_shb
qmake ../%dirname64%/qwt.pro -r -spec win32-g++
mingw32-make -j4

echo.
echo Done
pause