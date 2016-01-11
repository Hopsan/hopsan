@ECHO OFF
REM $Id$

REM Bat script to setup build paths for Hopsan and dependencies 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

REM Argument %1 should be version
REM Argument %2 should be arch: x86 or x64

echo Setting paths for %1 %2

REM Setup 0.7.x paths
if "%~1"=="0.7.x" (
	REM These paths require the following download http://sourceforge.net/projects/qt64ng/files/qt/x86-64/5.4.1/mingw-4.9/seh/qt-5.4.1-x64-mingw492r1-seh-rev1.exe
	set mingw_path32=NotAvailable
	set mingw_path64=C:\Qt\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
	set qmake_path32=NotAvailable
	set qmake_path64=C:\Qt\qt-5.4.2-x64-mingw493r4-seh-rev1\bin
) else (
	echo "Error Unsupported arg1 (version) %~1"
	pause
	goto :eof
)

if not "%2"=="x86" (
  if not "%2"=="x64" (
	echo Error arg2 must be x86 or x64
	pause
	goto :eof
  )
)

REM Tools paths
set cmake_path32=%ProgramFiles%\CMake\bin
set cmake_path64=%ProgramFiles(x86)%\CMake\bin
set doxygen_path32=%ProgramFiles%\doxygen\bin
set doxygen_path64=%ProgramFiles%\doxygen\bin
set msys_path=C:\msys64\usr\bin

REM Set tools paths depending on current arch
if defined ProgramFiles(x86) (
	REM do stuff for 64bit here
	echo 64bit Windows detected, expecting 64-bit tools
	set "cmake_path=%cmake_path64%"
	set "doxygen_path=%doxygen_path64%"
) else (
    REM do stuff for 32bit here
	echo 32bit  Windows detected, expecting 32-bit tools
	set "cmake_path=%cmake_path32%"
	set "doxygen_path=%doxygen_path32%"
)

REM Set compilers and libs depending on selection
if "%2"=="x64" (
	set "mingw_path=%mingw_path64%"
	set "qmake_path=%qmake_path64%"
) else (
	set "mingw_path=%mingw_path32%"
	set "qmake_path=%qmake_path32%"
)

REM Echo resulting paths
echo CMake path:   %cmake_path%
echo Doxygen path: %doxygen_path%
echo Msys path:    %msys_path%
echo MinGW path:   %mingw_path%
echo QMake path:   %qmake_path%

echo.

set PATH=%mingw_path%;%qmake_path%;%cmake_path%;%doxygen_path%;%msys_path%;%PATH%

:eof