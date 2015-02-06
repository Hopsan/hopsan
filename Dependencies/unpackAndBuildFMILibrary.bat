@ECHO OFF
REM $Id$

REM Bat script building FMILibrary dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-01-29

set filename="FMILibrary-2.0.1-src.zip"
set updirname="FMILibrary-2.0.1"

REM Here you can set your paths to CMake and you MinGW version, do not commit changes unless they are official
set cmake_path32="C:\Program Files\CMake\bin"
set cmake_path64="C:\Program Files (x86)\CMake\bin"
set mingw-w64_path=C:\Qt\qt-5.3.2-x64-mingw491r1-seh-opengl\mingw64\bin
set mingw_path=NotAvailable


if defined ProgramFiles(x86) (
	REM do stuff for 64bit here
	echo 64bit
	set dirname=%updirname%_x64
	set cmake_path=%cmake_path64%
	set mingw_path=%mingw-w64_path%
) else (
    REM do stuff for 32bit here
	set dirname=%updirname%
	set cmake_path=%cmake_path32%
	set mingw_path=%mingw_path%
)

echo.
echo ======================
echo Using CMake path: %cmake_path%
echo Using MinGW path: %mingw_path%
echo ======================
REM Set the local PATH variable
set PATH=%mingw_path%;%cmake_path%;%PATH%
timeout 2

REM Unpack or checkout
echo.
echo Clearing old directory (if it exists)
rd /s/q %dirname%
echo Unpacking %filename%
..\..\ThirdParty\7z\7z.exe x %filename% -y > nul
move %updirname% %dirname%

REM Build
echo.
echo ======================
echo Building
echo ======================
cd %dirname%
mkdir build-fmilib
cd build-fmilib
cmake -G "MinGW Makefiles" ../
mingw32-make.exe -j4
mingw32-make.exe install

echo.
echo Done
pause