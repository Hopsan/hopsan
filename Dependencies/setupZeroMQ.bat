@ECHO OFF
REM $Id$

REM Bat script building ZeroMQ and unpacking messagepack automatically 
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set name=zeromq
set codedir=%basedir%\%name%_code
set builddir=%basedir%\%name%_build
set installdir=%basedir%\%name%

REM Setup PATH
REM We don want msys sh.exe in the PATH so we have clean it and set it manually
set OLDPATH=%PATH%
set OLDPATH=%OLDPATH:C:\Program Files (x86)\Git\usr\bin;=%
set OLDPATH=%OLDPATH:C:\Program Files\Git\usr\bin;=%
call setHopsanBuildPaths.bat
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
PATH
REM build
mkdir %builddir%
cd %builddir%
REM bash.exe -c "./autogen.sh; ./configure --without-libsodium --host=x86_64-w64-mingw32; mingw32-make -j4"
cmake -Wno-dev -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make -j4
mingw32-make install
REM mingw32-make test

REM Now "install" cppzmq (header only), to the zmq install dir
REM TODO in the future use cmake to install cppzmq, but that does not work right now since no "zeromq-config.cmake" file seem to be created.
REM For now lets just copy the file
set codedir=%basedir%\cppzmq
set installdir=%installdir%\include\

xcopy /Y %codedir%\zmq.hpp %installdir%

cd %basedir%
echo.
echo setupZeroMQ.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
