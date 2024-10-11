@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=asio
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

pause

call setHopsanBuildPaths.bat

cd %codedir%
configure --prefix=$installdir --without-boost
cd src
mingw32-make -f Makefile.mgw.

cd %basedir%
echo.
echo setupAsio.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal

