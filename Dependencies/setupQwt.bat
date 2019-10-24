@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=qwt
set codedir=%basedir%\%name%_code
set builddir=%basedir%\%name%_build
set installdir=%basedir%\%name%

REM Setup paths
call setHopsanBuildPaths.bat

REM Patch libQWT
cd %codedir%
patch.exe --forward -p1 < ..\qwt-build.patch

REM Build
mkdir %builddir%
cd %builddir%

qmake %codedir%\qwt.pro -r -spec win32-g++
mingw32-make -j8
mingw32-make install

cd %basedir%
echo.
echo setupQwt.bat Done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
