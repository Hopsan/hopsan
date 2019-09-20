@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=discount
set codedir=%basedir%\%name%_code
set builddir=%basedir%\%name%_build
set installdir=%basedir%\%name%

REM This path is needed for bash / posix compatibility inside the bash shell
REM TODO it would be nice if we could auto generate the bash path from the windows install dir path
set installdir_bash=../%name%

call setHopsanBuildPaths.bat
REM Make sure that the real msys is found first, TODO It would be better if we could tell the setupPaths script what msys environment to prefere
set PATH=%msys_path%;%PATH%

REM Copy code to build dir, not sure if out-of-source build is possible
mkdir %builddir%
cd %builddir%
xcopy %codedir%\* . /Y

REM Build with mingw patches

REM The first patch was taken from https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-discount
REM It was however modified to grep for Msys instead of MINGW (from uname -a)
bash.exe -c "patch -p0 < ../discount-mingw-building.patch; patch -p1 < ../discount-2.1.8-msys.patch; CC=gcc ./configure.sh --shared --prefix=%installdir_bash% --confdir=%installdir_bash%/etc; mingw32-make -j4; mingw32-make install"

cd %basedir%
echo.
echo setupDiscount.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
