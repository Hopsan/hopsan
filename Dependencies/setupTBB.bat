@ECHO OFF
REM $Id$

REM Bat script building HopsaCore dependency TBB automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename="tbb41_20130613oss_src.7z"
set dirname=tbb41_20130613oss

REM Automatic code begins here

REM Unpack or checkout
echo.
echo ======================
echo Unpack TBB
echo ======================
if exist %dirname% rd /s/q %dirname%
mkdir %dirname%
REM Unpack
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Build
echo.
echo ======================
echo Building 64-bit TBB
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64
cd %dirname%
mingw32-make compiler=gcc tbb_build_prefix=output info
mingw32-make compiler=gcc tbb_build_prefix=output -j4

echo.
echo Done
pause