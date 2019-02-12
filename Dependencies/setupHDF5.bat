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

call setHopsanBuildPaths.bat

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
REM mingw32-make.exe -j4 STOP! DO NOT enable multi-core build (make -j4), we must build sequentially for some reason
mingw32-make.exe SHELL=cmd
mingw32-make.exe SHELL=cmd install

cd %basedir%
echo.
echo setupHDF5.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
