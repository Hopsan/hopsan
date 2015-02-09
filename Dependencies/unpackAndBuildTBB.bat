@ECHO OFF
REM $Id$

REM Bat script building HopsaCore dependency TBB automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set filename="tbb41_20130613oss_src.7z"
set dirname=tbb41_20130613oss

REM Automatic code begins here

set dirname64=%dirname%_x64

REM Unpack or checkout
echo.
echo ======================
echo Unpack TBB
echo ======================
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
echo Building 64-bit TBB
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64
cd %dirname64%
mingw32-make compiler=gcc tbb_build_prefix=output info
mingw32-make compiler=gcc tbb_build_prefix=output -j4

echo.
echo Done
pause