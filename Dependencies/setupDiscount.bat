@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically 
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set codedir=%basedir%\discount
set builddir=%codedir%_build
set installdir=%codedir%_install

REM This path is needed for bash / posix compatibility inside the bash shell
REM TODO it would be nice if we could auto generate the bash path from the windows install dir path
set installdir_bash=../discount_install

call setHopsanBuildPaths.bat

REM Copy code to build dir, not sure if out-of-source build is possible
mkdir %builddir%
cd %builddir%
xcopy %codedir%\* . /Y

REM Build with mingw patches

REM The first patch was taken from https://github.com/Alexpux/MINGW-packages/tree/master/mingw-w64-discount
REM It was however modified to grep for Msys instead of MINGW (from uname -a)
bash.exe -c "patch -p0 < ../discount-mingw-building.patch; patch -p1 < ../discount-2.1.8-msys.patch; CC=gcc ./configure.sh --shared --prefix=%installdir_bash% --confdir=%installdir_bash%/etc; make -j4; make install"
REM Note! We use msys make here as mingw32-make is to strict since 4.9.2

cd %basedir%
echo.
echo setupDiscount.sh done
pause
