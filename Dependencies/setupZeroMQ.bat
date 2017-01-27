@ECHO OFF
REM $Id$

REM Bat script building ZeroMQ and unpacking messagepack automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

set zmqdirname=zeromq

REM Automatic code begins here

REM Build
echo.
echo ======================
echo Building 64-bit ZeroMQ
echo ======================
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We dont want msys in the path so we have to set it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
PATH

cd %zmqdirname%
REM bash.exe -c "./autogen.sh; ./configure --without-libsodium --host=x86_64-w64-mingw32; mingw32-make -j4"
mkdir build
cd build
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=../build_install ../ 
mingw32-make -j4
mingw32-make install
REM mingw32-make test

echo.
echo Done
pause
