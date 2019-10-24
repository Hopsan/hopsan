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

REM Setup Default compiler and Qt paths
if "%hopsan_arch%"=="x86" (
	REM These paths require the official Qt version from Qt.io (for 32-bit)
	set mingw_path=C:\Qt\Tools\mingw492_32\bin
	set qmake_path=C:\Qt\5.6.3\mingw49_32\bin
) else (
	REM These paths require the custom build Qt library for Hopsan 64-bit
	set mingw_path=C:\hopsan-dev\x86_64-4.9.4-release-posix-seh-rt_v5-rev0\mingw64\bin
	set qmake_path=C:\hopsan-dev\qt-5.6.3-x64-mingw494-posix-seh-rt_v5-rev0\bin
)

REM Tool paths
set msys_path=C:\msys64\usr\bin

REM Set default installation tools paths depending on current Windows architecture
if defined ProgramFiles(x86) (
	REM Lookup tools paths for 64-bit Windows
	echo 64bit Windows detected, expecting 64-bit tools, but checking for 32-bit anyway
	if exist "%ProgramFiles(x86)%\CMake\bin" (
		set "cmake_path=%ProgramFiles(x86)%\CMake\bin"
	) else (
		set "cmake_path=%ProgramW6432%\CMake\bin"
	)
	if exist "%ProgramFiles(x86)%\doxygen\bin" (
		set "doxygen_path=%ProgramFiles(x86)%\doxygen\bin"
	) else (
		set "doxygen_path=%ProgramW6432%\doxygen\bin"
	)
	REM Assume official "Git for Windows"
	if exist "%ProgramFiles(x86)%\Git\bin" (
		set "git_path=%ProgramFiles(x86)%\Git\bin"
		set "gitmsys_path=%ProgramFiles(x86)%\Git\usr\bin"
	) else (
		set "git_path=%ProgramW6432%\Git\bin"
		set "gitmsys_path=%ProgramW6432%\Git\usr\bin"
	)
) else (
    REM Lookup tools paths  for 32-bit Windows
	echo 32bit Windows detected, expecting 32-bit tools
	set "cmake_path=%ProgramFiles%\CMake\bin"
	set "doxygen_path=%ProgramFiles%\doxygen\bin"
	REM Assume official "Git for Windows"
	set "git_path=%ProgramFiles%\Git\bin"
	set "gitmsys_path=%ProgramFiles%\Git\usr\bin"
)

REM If the HOME directory of Qt is already specified, use that instead of default expected path
if not "%HOPSAN_BUILD_QT_HOME%" == "" (
	set "qmake_path=%HOPSAN_BUILD_QT_HOME%\bin"
)
REM If the HOME directory of MinGW is already specified, use that instead of default expected path
if not "%HOPSAN_BUILD_MINGW_HOME%" == "" (
	set "mingw_path=%HOPSAN_BUILD_MINGW_HOME%\bin"
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
