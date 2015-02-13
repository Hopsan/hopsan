@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename=discount-2.1.8.zip
set dirname=discount-2.1.8

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

REM We also need msys
if not exist "%CD%\msys" (
echo Unpacking MSYS...
 ..\ThirdParty\7z\7z.exe x MSYS-20111123.zip -y > nul
)
set PATH=%CD%\msys\bin;%PATH%
cd %dirname64%

echo.
echo ========== READ THIS ==========
echo When configuring you will get a program crash, just close that window and pretend that it is raining.
timeout 10

REM This first patch was taken from https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-discount
bash.exe -c "cp ../discount-mingw-building.patch .; patch -p0 < discount-mingw-building.patch"
bash.exe -c "cp ../discount-2.1.8.patch .; patch -p1 < discount-2.1.8.patch"
bash.exe -c "CC=gcc ./configure.sh --shared; mingw32-make -j4"

echo.
echo Done
pause
