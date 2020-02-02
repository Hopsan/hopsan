@ECHO OFF

REM Bat script for installing msgpack-c dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=msgpack-c
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM build
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make SHELL=cmd -j8
mingw32-make SHELL=cmd install
REM mingw32-make test


cd %basedir%
echo.
echo setupMsgpack.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
