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
	set mingw_path=C:\Qt\Tools\mingw530_32\bin
	set qmake_path=C:\Qt\5.9.9\mingw53_32\bin
) else (
	REM These paths require the custom build Qt library for Hopsan 64-bit
	set mingw_path=C:\hopsan-dev\x86_64-5.4.0-release-posix-seh-rt_v5-rev0\mingw64\bin
	set qmake_path=C:\hopsan-dev\qt-5.9.9-x64-5.4.0-release-posix-seh-rt_v5-rev0\bin
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
	if exist "%ProgramFiles(x86)%\Git\cmd" (
		set "git_path=%ProgramFiles(x86)%\Git\cmd"
		set "gitmsys_path=%ProgramFiles(x86)%\Git\usr\bin"
	) else (
		set "git_path=%ProgramW6432%\Git\cmd"
		set "gitmsys_path=%ProgramW6432%\Git\usr\bin"
	)
) else (
    REM Lookup tools paths  for 32-bit Windows
	echo 32bit Windows detected, expecting 32-bit tools
	set "cmake_path=%ProgramFiles%\CMake\bin"
	set "doxygen_path=%ProgramFiles%\doxygen\bin"
	REM Assume official "Git for Windows"
	set "git_path=%ProgramFiles%\Git\cmd"
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

set hopsan_build_paths=%mingw_path%;%qmake_path%;%git_path%;%cmake_path%;%doxygen_path%

REM Set default CMake generator, can be used to override build to use MSVC instead of MinGW
if %HOPSAN_BUILD_CMAKE_GENERATOR%x == x (
	set HOPSAN_BUILD_CMAKE_GENERATOR="MinGW Makefiles"
)

REM Assume mingw compiler
if %HOPSAN_BUILD_COMPILER%x == x (
	set HOPSAN_BUILD_COMPILER=mingw
)
REM If CMAKE_GENERATOR is "Visual Stuido ***" auto set compiler to MSVC
if "%HOPSAN_BUILD_CMAKE_GENERATOR:~1,6%" == "Visual" (
	set HOPSAN_BUILD_COMPILER=msvc
)

REM Echo expected paths and generator
echo HOPSAN_BUILD_CMAKE_GENERATOR: %HOPSAN_BUILD_CMAKE_GENERATOR%
echo cmake_path:   %cmake_path%
echo doxygen_path: %doxygen_path%
echo mingw_path:   %mingw_path%
echo qmake_path:   %qmake_path%
echo git_path:     %git_path%
echo gitmsys_path: %gitmsys_path% (This one is added to PATH_WITH_MSYS)
echo msys_path:    %msys_path%    (This one is not added to PATH_WITH_MSYS automatically)
echo.

set PATH_WITHOUT_MSYS=%hopsan_build_paths%;%PATH%
set PATH_WITH_MSYS=%hopsan_build_paths%;%gitmsys_path%;%PATH%
set PATH=%PATH_WITHOUT_MSYS%

:eof
