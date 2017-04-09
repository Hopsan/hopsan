@ECHO OFF
REM $Id$

REM Bat script to setup build paths for Hopsan and dependencies 
REM Author: Peter Nordin peter.nordin@liu.se
REM Argument %1 should be arch: x86 or x64

REM Default values
REM Use x64 for 64-bit, use x86 for 32-bit
set hopsan_arch=x64

REM Overwrite with command line argument values if present
if not "%~1"=="" (
	set hopsan_arch=%~1
)
if not "%~2"=="" (
REM Do something maybe	
)

echo Setting paths for architecture: %hopsan_arch%
if not "%hopsan_arch%"=="x86" (
  if not "%hopsan_arch%"=="x64" (
	echo Error: hopsan_arch must be x86 or x64
	pause
	goto :eof
  )
)

REM Setup compiler and Qt paths
REM These paths require the official Qt version from Qt.io (for 32-bit) and the custom build Qt library for Hopsan 64-bit
set mingw_path32=C:\Qt\Tools\mingw491_32\bin
set mingw_path64=C:\Qt\x86_64-4.9.3-release-posix-seh-rt_v4-rev1\mingw64\bin
set qmake_path32=C:\Qt\5.4\mingw491_32\bin
set qmake_path64=C:\Qt\qt-5.4.2-x64-mingw493r4-seh-rev1\bin

REM Tool paths
set msys_path=C:\msys64\usr\bin

REM Set default installation tools paths depending on current Windows architecture
if defined ProgramFiles(x86) (
	REM do stuff for 64bit here
	echo 64bit Windows detected, expecting 64-bit tools, but checking for 32-bit anyway
	if exist "%ProgramFiles(x86)%\CMake\bin" (
		set "cmake_path=%ProgramFiles(x86)%\CMake\bin" 
	) else (
		set "cmake_path=%ProgramFiles%\CMake\bin"
	)
	if exist "%ProgramFiles(x86)%\doxygen\bin" (
		set "doxygen_path=%ProgramFiles(x86)%\doxygen\bin"
	) else (
		set "doxygen_path=%ProgramFiles%\doxygen\bin"
	)
	REM Assume official "Git for Windows" 
	if exist "%ProgramFiles(x86)%\Git\bin" (
		set "git_path=%ProgramFiles(x86)%\Git\bin"
		set "gitmsys_path=%ProgramFiles(x86)%\Git\usr\bin"
	) else (
		set "git_path=%ProgramFiles%\Git\bin"
		set "gitmsys_path=%ProgramFiles%\Git\usr\bin"
	)
) else (
    REM do stuff for 32bit here
	echo 32bit Windows detected, expecting 32-bit tools
	set "cmake_path=%ProgramFiles%\CMake\bin"
	set "doxygen_path=%ProgramFiles%\doxygen\bin"
	REM Assume official "Git for Windows"
	set "git_path=%ProgramFiles%\Git\bin"
	set "gitmsys_path=%ProgramFiles%\Git\usr\bin"
)

REM Choose compiler and Qt path depending on selected build type
if "%hopsan_arch%"=="x64" (
	set "mingw_path=%mingw_path64%"
	set "qmake_path=%qmake_path64%"
) else (
	set "mingw_path=%mingw_path32%"
	set "qmake_path=%qmake_path32%"
)

REM Echo expected paths
echo cmake_path:   %cmake_path%
echo doxygen_path: %doxygen_path%
echo msys_path:    %msys_path%
echo mingw_path:   %mingw_path%
echo qmake_path:   %qmake_path%
echo git_path:     %git_path%
echo gitmsys_path: %gitmsys_path%

REM Avoid duplicate msys in path, add msys if it is available, else add msys shipped with Git for Windows"
if exist "%msys_path%" (
	set "msys_to_path=%msys_path%"
) else (
	set "msys_to_path=%gitmsys_path%"
)

echo.

set PATH=%mingw_path%;%qmake_path%;%git_path%;%cmake_path%;%doxygen_path%;%msys_to_path%;%PATH%

:eof