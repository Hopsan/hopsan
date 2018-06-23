@ECHO OFF
REM $Id$

REM Bat script for building Matio automatically
REM Author: Peter Nordin peter.nordin@liu.se

set dirname=matio

REM Automatic code begins here

echo.
echo ======================
echo Building 64-bit Discount
echo ======================
call setHopsanBuildPaths.bat

cd %dirname%
bash.exe -c "patch -p1 < ../matio-1.5.2.patch; autoreconf -i --force; LT_LDFLAGS=-no-undefined ./configure --without-zlib; mingw32-make.exe -j4"

echo.
echo setupMatio.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
