@ECHO OFF
REM $Id$

REM Bat script building FMILibrary dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set name=FMILibrary
set codedir=%basedir%\%name%_code
set builddir=%basedir%\%name%_build
set installdir=%basedir%\%name%


set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to set it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%

REM build
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DFMILIB_FMI_PLATFORM="win64" -DFMILIB_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make.exe -j4
mingw32-make.exe install

cd %basedir%
echo.
echo setupFMILibrary.bat done
pause
