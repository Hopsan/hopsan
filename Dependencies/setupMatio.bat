@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename=matio-1.5.2.zip
set dirname=matio-1.5.2

REM Automatic code begins here
set dirname64=%dirname%_x64

REM Unpack or checkout
echo.
echo ======================
echo Unpack Discount
echo ======================
echo Removing old directories (if they exist)
rd /s/q %dirname%
rd /s/q %dirname64%
mkdir %dirname%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Copy to 64-bit dir
echo.
echo ======================
echo Copying to %dirname64%
echo ======================
robocopy /e /NFL /NDL /NJH /NJS /nc /ns /np  %dirname% %dirname64%

REM Build
echo.
echo ======================
echo Building 64-bit Discount
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64

cd %dirname64%
bash.exe -c "patch -p1 < ../%dirname%.patch; autoreconf -i --force; LT_LDFLAGS=-no-undefined ./configure --without-zlib; mingw32-make.exe -j4"

echo.
echo Done
pause
