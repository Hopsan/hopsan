:: 1 = mingwDir
:: 2 = qmakeDir
:: 3 = hopsanDir

call %2\qtenv2.bat
call %1\mingw32-make.exe clean
call %2\qmake.exe %3\HopsanNG.pro -r -spec win32-g++ \"CONFIG+=release\"
call %1\mingw32-make.exe