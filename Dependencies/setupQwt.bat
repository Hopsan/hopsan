@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set codedir=%basedir%\qwt
set builddir=%codedir%_build
set installdir=%codedir%_install

REM Setup paths
call setHopsanBuildPaths.bat

REM Patch libQWT
cd %codedir%
patch.exe --forward -p1 < ..\qwt-build.patch

REM Build
mkdir %builddir%
cd %builddir%

qmake %codedir%\qwt.pro -r -spec win32-g++
mingw32-make -j4
mingw32-make install

cd %basedir%
echo.
echo setupQwt.bat Done
pause