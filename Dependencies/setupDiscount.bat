@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename=discount-2.1.8.zip
set dirname=discount-2.1.8

REM Automatic code begins here

REM Unpack or checkout
echo.
echo ======================
echo Unpack Discount
echo ======================
echo Removing old directories (if they exist)
if exist %dirname% rd /s/q %dirname%
mkdir %dirname%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Build
echo.
echo ======================
echo Building 64-bit Discount
echo ======================
call setHopsanBuildPaths.bat

cd %dirname%

REM The first patch was taken from https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-discount
REM It was however modified to grep for Msys instead of MINGW (from uname -a)
bash.exe -c "patch -p0 < ../discount-mingw-building.patch; patch -p1 < ../discount-2.1.8-msys.patch; CC=gcc ./configure.sh --shared; make -j4"
REM Note! We use msys make here as mingw32-make is to strict since 4.9.2

echo.
echo Done
pause
