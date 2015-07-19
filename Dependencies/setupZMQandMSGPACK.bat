@ECHO OFF
REM $Id$

REM Bat script building ZeroMQ and unpacking messagepack automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

set msgpackfile=msgpack-c-cpp-1.0.1.zip
set msgpackdir=msgpack-c-cpp-1.0.1
set cppzmqfile=cppzmq-master.zip
set cppzmqdir=cppzmq-master

set zmqfilename=zeromq4-1-4.1.2.zip
set zmqdirname=zeromq4-1-4.1.2

REM Automatic code begins here
set zmqdirname64=%zmqdirname%_x64

REM Unpack or checkout
echo.
echo ======================
echo Unpack ZeroMQ
echo ======================
echo Removing old directories (if they exist)
rd /s/q %msgpackdir%
rd /s/q %cppzmqdir%
rd /s/q %zmqdirname%
rd /s/q %zmqdirname64%
mkdir %zmqdirname%
mkdir %msgpackdir%
mkdir %cppzmqdir%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %zmqfilename% -y > nul
..\ThirdParty\7z\7z.exe x %msgpackfile% -y > nul
..\ThirdParty\7z\7z.exe x %cppzmqfile% -y > nul

REM echo.
REM echo ======================
REM echo Patch libQWT
REM echo ======================
REM ..\ThirdParty\patch\doit.exe -p0 < %zmqfilename%.patch

REM Copy to 64-bit dir
echo.
echo ======================
echo Copying to %zmqdirname64%
echo ======================
robocopy /e /NFL /NDL /NJH /NJS /nc /ns /np  %zmqdirname% %zmqdirname64%

REM Build
echo.
echo ======================
echo Building 64-bit ZeroMQ
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64

cd %zmqdirname64%
bash.exe -c "./autogen.sh; ./configure --without-libsodium --host=x86_64-w64-mingw32; mingw32-make -j4"

echo.
echo Done
pause
