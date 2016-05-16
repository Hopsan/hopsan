@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename=matio-1.5.2.zip
set dirname=matio-1.5.2

REM Automatic code begins here

REM Unpack or checkout
echo.
echo ======================
echo Unpack Discount
echo ======================
echo Removing old directories (if they exist)
if exist %dirname% rd /s/q %dirname%
mkdir %dirname%
REM Unpack 
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Build
echo.
echo ======================
echo Building 64-bit Discount
echo ======================
call setHopsanBuildPaths.bat

cd %dirname%
bash.exe -c "patch -p1 < ../%dirname%.patch; autoreconf -i --force; LT_LDFLAGS=-no-undefined ./configure --without-zlib; mingw32-make.exe -j4"

echo.
echo Done
pause
