@ECHO OFF
REM $Id$

REM Bat script building libHDF5 dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set name=hdf5
set codedir=%basedir%\%name%_code
set builddir=%basedir%\%name%_build
set installdir=%basedir%\%name%

REM Setup path
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to set it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
REM mingw32-make.exe -j4 STOP! DO NOT enable multi-core build (make -j4), we must build sequentially for some reason
mingw32-make.exe
mingw32-make.exe install

cd %basedir% 
echo.
echo setupHDF5.bat done
pause
