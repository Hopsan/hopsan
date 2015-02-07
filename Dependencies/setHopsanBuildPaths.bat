@ECHO OFF

REM Argument %1 should be version
REM Argument %2 should be arch: x86 or x64

echo Setting paths for %1 %2

REM Setup 0.7.x paths
if "%1"=="0.7.x" (
	set mingw_path32=NotAvailable
	set mingw_path64=C:\Qt\qt-5.3.2-x64-mingw491r1-seh-opengl\mingw64\bin
	set qmake_path32=NotAvailable
	set qmake_path64=C:\Qt\qt-5.3.2-x64-mingw491r1-seh-opengl\qt-5.3.2-x64-mingw491r1-seh-opengl\bin
)

REM Tools paths
set cmake_path32="C:\Program Files\CMake\bin"
set cmake_path64="C:\Program Files (x86)\CMake\bin"
set doxygen_path32="C:\Program Files (x86)\doxygen\bin"
set doxygen_path64="C:\Program Files\doxygen\bin"


REM Set tools paths depending on current arch
if defined ProgramFiles(x86) (
	REM do stuff for 64bit here
	echo 64bit Windows detected, expecting 64-bit tools
	set cmake_path=%cmake_path64%
	set doxygen_path=%doxygen_path64%
) else (
    REM do stuff for 32bit here
	echo 32bit  Windows detected, expecting 32-bit tools
	set cmake_path=%cmake_path32%
	set doxygen_path=%doxygen_path32%
)

REM Set compilers and libs depending on selection
if "%2"=="x64" (
	set mingw_path=%mingw_path64%
	set qmake_path=%qmake_path64%
) else (
	set mingw_path=%mingw_path32%
	set qmake_path=%qmake_path32%
)

echo CMake path:   %cmake_path%
echo MinGW path:   %mingw_path%
echo QMake path:   %qmake_path%
echo Doxygen path: %doxygen_path%
echo.

set PATH=%mingw_path%;%qmake_path%;%cmake_path%;%doxygen_path%;%PATH%
