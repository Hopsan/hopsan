:: 1 = mingwDir
:: 2 = qmakeDir
:: 3 = hopsanDir

echo off
SET PATH=%~1;%~2;%PATH%
mingw32-make.exe clean
qmake.exe %3\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
mingw32-make.exe
