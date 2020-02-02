@ECHO OFF
REM $Id$

REM Bat script for building ZeroMQ dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se

setlocal
set basedir=%~dp0
set name=zeromq
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM Apply build patch
set PATH=%PATH_WITH_MSYS%
cd %codedir%
patch.exe --forward -p1 < ..\zeromq-Prevent-conflicting-definition-of-close-when-compili.patch
set PATH=%PATH_WITHOUT_MSYS%

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -DWITH_LIBSODIUM=OFF -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
mingw32-make -j8
mingw32-make install
mingw32-make test

REM Now "install" cppzmq (header only), to the zmq install dir
REM TODO in the future use cmake to install cppzmq, but that does not work right now since no "zeromq-config.cmake" file seem to be created.
REM For now lets just copy the file
set codedir=%basedir%\cppzmq-code
set installdir=%installdir%\include\

xcopy /Y %codedir%\*.hpp %installdir%

cd %basedir%
echo.
echo setupZeroMQ.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
