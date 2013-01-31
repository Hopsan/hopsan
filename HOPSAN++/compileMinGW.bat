:: 1 = mingwDir
:: 2 = qmakeDir
:: 3 = hopsanDir


REM setlocal enabledelayedexpansion

echo off
echo %PATH%
SET PATH=%PATH%;%1;
SETX PATH "%PATH%;%1;"
echo.
echo %PATH%
echo.
echo on

%1\mingw32-make.exe clean
%2\qmake.exe %3\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
%1\mingw32-make.exe
