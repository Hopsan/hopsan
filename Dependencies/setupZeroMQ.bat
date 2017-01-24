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
call setHopsanBuildPaths.bat

cd %zmqdirname%
bash.exe -c "./autogen.sh; ./configure --without-libsodium --host=x86_64-w64-mingw32; mingw32-make -j4"

echo.
echo Done
pause
