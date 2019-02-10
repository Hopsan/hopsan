@ECHO OFF
REM $Id$

REM Bat script building FMILibrary dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=fmilibrary
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%


REM We dont want msys sh.exe in the PATH so we have clean it and set it manually
set OLDPATH=%PATH%
set OLDPATH=%OLDPATH:C:\Program Files (x86)\Git\usr\bin;=%
set OLDPATH=%OLDPATH:C:\Program Files\Git\usr\bin;=%
call setHopsanBuildPaths.bat
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DFMILIB_FMI_PLATFORM="win64" -DFMILIB_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make.exe -j8
mingw32-make.exe install

cd %basedir%
echo.
echo setupFMILibrary.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
