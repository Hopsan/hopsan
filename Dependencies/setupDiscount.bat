@ECHO OFF
REM $Id$

REM Bat script for building Discount automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=discount
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%


call setHopsanBuildPaths.bat

REM Apply build patch
cd %codedir%
patch.exe --forward -p0 < ..\discount-attribute.patch

REM Configure with CMake and then build and install
mkdir %builddir%
cd %builddir%
cmake -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%\cmake
mingw32-make SHELL=cmd -j4
mingw32-make SHELL=cmd install

cd %basedir%
echo.
echo setupDiscount.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
