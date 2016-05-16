@ECHO OFF
REM $Id$

REM Bat script to setup build paths for Hopsan and dependencies 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

REM Argument %1 should be version
REM Argument %2 should be arch: x86 or x64

REM Default values. Use x64 for 64-bit, use x86 for 32-bit
set hopsan_arch=x64
set hopsan_version=0.7.x

REM Overwrite with command line argument values if present
if not "%~1"=="" (
	set hopsan_version=%~1
)
if not "%~2"=="" (
	set hopsan_arch=%~2
)

echo Setting paths for Version: %1, Architecture: %hopsan_arch%

REM Setup 0.7.x paths
if "%hopsan_version%"=="0.7.x" (
	REM These paths require the official qt version from Qt.io (for 32-bit) and the custom Hopsan version for 64-bit
	set mingw_path32=C:\Qt\Tools\mingw491_32\bin
	set mingw_path64=C:\Qt\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
	set qmake_path32=C:\Qt\5.4\mingw491_32\bin
	set qmake_path64=C:\Qt\qt-5.4.2-x64-mingw493r4-seh-rev1\bin
) else (
	echo Error: Unsupported version: %hopsan_version%
	pause
	goto :eof
)

if not "%hopsan_arch%"=="x86" (
  if not "%hopsan_arch%"=="x64" (
	echo Error: hopsan_arch must be x86 or x64
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
	echo 32bit Windows detected, expecting 32-bit tools
	set "cmake_path=%cmake_path32%"
	set "doxygen_path=%doxygen_path32%"
)

REM Set compilers and libs depending on selection
if "%hopsan_arch%"=="x64" (
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