@ECHO OFF
REM $Id$

REM Bat script building libHDF5 dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-07-06

set dirname=hdf5

REM Automatic code starts here

echo.
echo ======================
echo Building the HDF5 library and tools
echo ======================
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to set it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
cd %dirname%
mkdir hopsanbuild
cd hopsanbuild
cmake -Wno-dev -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX="../install" ../
REM DO NOT enable multi-core build (make -j4), we must build sequentially
mingw32-make.exe
mingw32-make.exe install
cd ..
echo.
echo Done
pause