@ECHO OFF
REM $Id$

REM Bat script building libHDF5 dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=hdf5
set version=1.8.21
set codedir=%basedir%\%name%-code\%name%-%version%
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
REM We don want msys sh.exe in the PATH so we have clean it and set it manually
set OLDPATH=%PATH%
set OLDPATH=%OLDPATH:C:\Program Files (x86)\Git\usr\bin;=%
set OLDPATH=%OLDPATH:C:\Program Files\Git\usr\bin;=%
call setHopsanBuildPaths.bat
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
REM mingw32-make.exe -j4 STOP! DO NOT enable multi-core build (make -j4), we must build sequentially for some reason
mingw32-make.exe
mingw32-make.exe install

cd %basedir%
echo.
echo setupHDF5.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
