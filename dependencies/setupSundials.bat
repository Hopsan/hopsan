@ECHO OFF
REM $Id$

REM Bat script to build Sundials dependency automatically
REM Author: Robert Braun robert.braun@liu.se

setlocal
set basedir=%~dp0
set name=sundials
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup paths
call setHopsanBuildPaths.bat
set PATH=%PATH_WITH_MSYS%

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%

set sundials_cmake_args="-Wno-dev -DBUILD_STATIC_LIBS=OFF -DBUILD_ARKODE=OFF -DBUILD_IDAS=OFF -DBUILD_IDA=OFF -DBUILD_CVODE=OFF -DBUILD_CVODES=OFF -DBUILD_KINSOL=ON -DBUILD_EXAMPLES_C=OFF -DEXAMPLES_INSTALL=OFF -DCMAKE_INSTALL_LIBDIR=lib"

REM Configure
cmake ${sundials_cmake_args} -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
cmake --build . --parallel 8
cmake --build . --target install


REM Return to basedir
cd %basedir%
echo.
echo setSundials.bat Done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
