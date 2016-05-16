@ECHO OFF
REM $Id$

REM Bat script building ZeroMQ and unpacking messagepack automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

set msgpackfile=msgpack-c-cpp-1.3.0.zip
set msgpackdir=msgpack-c-cpp-1.3.0
set cppzmqfile=cppzmq-master.zip
set cppzmqdir=cppzmq-master

set zmqfilename=zeromq4-1-4.1.3.zip
set zmqdirname=zeromq4-1-4.1.3

REM Automatic code begins here

REM Unpack or checkout
echo.
echo ======================
echo Unpack ZeroMQ
echo ======================
echo Removing old directories (if they exist)
if exist %msgpackdir% rd /s/q %msgpackdir%
if exist %cppzmqdir% rd /s/q %cppzmqdir%
if exist %zmqdirname% rd /s/q %zmqdirname%
mkdir %zmqdirname%
mkdir %msgpackdir%
mkdir %cppzmqdir%
REM Unpack 
..\ThirdParty\7z\7z.exe x %zmqfilename% -y > nul
..\ThirdParty\7z\7z.exe x %msgpackfile% -y > nul
..\ThirdParty\7z\7z.exe x %cppzmqfile% -y > nul

REM echo.
REM echo ======================
REM echo Patch libQWT
REM echo ======================
REM ..\ThirdParty\patch\doit.exe -p0 < %zmqfilename%.patch

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
