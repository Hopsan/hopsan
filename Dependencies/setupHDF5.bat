@ECHO OFF
REM $Id$

REM Bat script building libHDF5 dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-07-06

set dirname=hdf5

REM Automatic code starts here

echo.
echo ======================
echo Building 64-bit version of FMILibrary
echo ======================
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to set it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
cd %dirname%
mkdir build
cd build
cmake -Wno-dev -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DCMAKE_INSTALL_PREFIX="../install" ../
mingw32-make.exe -j4
mingw32-make.exe install
echo.
echo Done
pause