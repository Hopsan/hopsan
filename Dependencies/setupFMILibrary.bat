@ECHO OFF
REM $Id$

REM Bat script building FMILibrary dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-01-29

set filename=FMILibrary-2.0.1-src.zip
set dirname=FMILibrary-2.0.1

REM Automatic code starts here

REM Unpack
echo.
echo Clearing old directory (if it exists)
if exist %dirname% rd /s/q %dirname%
echo Unpacking %filename%
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Build
echo.
echo ======================
echo Building 64-bit version of FMILibrary
echo ======================
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to build it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
cd %dirname%
mkdir build-fmilib
cd build-fmilib
cmake -Wno-dev -G "MinGW Makefiles" -DFMILIB_FMI_PLATFORM="win64" ../
mingw32-make.exe -j4
mingw32-make.exe install

echo.
echo Done
pause