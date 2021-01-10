@ECHO OFF
REM $Id$

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=qwt
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup paths
call setHopsanBuildPaths.bat

REM Patch libQWT
set PATH=%PATH_WITH_MSYS%
cd %codedir%
patch.exe --forward -p0 < ..\qwt-build.patch

REM Build
set PATH=%PATH_WITHOUT_MSYS%
mkdir %builddir%
cd %builddir%

if "%HOPSAN_BUILD_COMPILER%" == "msvc" (
  echo "Building Qwt for MSVC"
  qmake %codedir%\qwt.pro -r -spec win32-msvc
  nmake
  nmake install
) else (
  echo "Building Qwt for MinGW"
  qmake %codedir%\qwt.pro -r -spec win32-g++
  mingw32-make -j8
  mingw32-make install
)

cd %basedir%
echo.
echo setupQwt.bat Done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
