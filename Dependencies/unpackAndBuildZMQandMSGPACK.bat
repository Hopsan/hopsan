@ECHO OFF
REM $Id$

REM Bat script building ZeroMQ and unpacking messagepack automatically 
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-02-07

set msgpackfile=msgpack-c-master.zip
set msgpackdir=msgpack-c-master
set cppzmqfile=cppzmq-master.zip
set cppzmqdir=cppzmq-master

set filename=zeromq-4.1.0-rc1.zip
set dirname=zeromq-4.1.0

REM Automatic code begins here
set dirname64=%dirname%_x64

REM Unpack or checkout
echo.
echo ======================
echo Unpack ZeroMQ
echo ======================
echo Removing old directories (if they exist)
rd /s/q %msgpackdir%
rd /s/q %cppzmqdir%
rd /s/q %dirname%
rd /s/q %dirname64%
mkdir %dirname%
mkdir %msgpackdir%
mkdir %cppzmqdir%
REM Unpack using tar
..\ThirdParty\7z\7z.exe x %filename% -y > nul
..\ThirdParty\7z\7z.exe x %msgpackfile% -y > nul
..\ThirdParty\7z\7z.exe x %cppzmqfile% -y > nul

REM echo.
REM echo ======================
REM echo Patch libQWT
REM echo ======================
REM ..\ThirdParty\patch\doit.exe -p0 < %filename%.patch

REM Copy to 64-bit dir
echo.
echo ======================
echo Copying to %dirname64%
echo ======================
robocopy /e /NFL /NDL /NJH /NJS /nc /ns /np  %dirname% %dirname64%

REM Build
echo.
echo ======================
echo Building 64-bit ZeroMQ
echo ======================
call setHopsanBuildPaths.bat 0.7.x x64

REM We also need msys
if not exist "%CD%\msys" (
echo Unpacking MSYS...
 ..\ThirdParty\7z\7z.exe x MSYS-20111123.zip -y > nul
)
set PATH=%CD%\msys\bin;%PATH%
cd %dirname64%
bash.exe -c "./configure --host=x86_64-w64-mingw32; mingw32-make -j4"

echo.
echo Done
