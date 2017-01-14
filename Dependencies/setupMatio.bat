@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-09

set dirname=matio

REM Automatic code begins here

echo.
echo ======================
echo Building 64-bit Discount
echo ======================
call setHopsanBuildPaths.bat

cd %dirname%
bash.exe -c "patch -p1 < ../%dirname%.patch; autoreconf -i --force; LT_LDFLAGS=-no-undefined ./configure --without-zlib; mingw32-make.exe -j4"

echo.
echo Done
pause
