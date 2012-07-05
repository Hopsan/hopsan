:: 1 = mingwDir
:: 2 = qmakeDir
:: 3 = hopsanDir

setlocal enabledelayedexpansion

echo %PATH%

echo off

call %2\qtenv2.bat
echo on
call %1\mingw32-make.exe clean
echo on
call %2\qmake.exe %3\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
echo on
call %1\mingw32-make.exe
