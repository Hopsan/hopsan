:: 1 = version
:: 2 = architecture
:: 3 = msvcDir
:: 4 = qmakeDir
:: 5 = buildDir
:: 6 = jomDir
:: 7 = hopsanDir

call %3\SetEnv.cmd /Release /%2
echo %3\SetEnv.cmd /Release /%2
pause
COLOR 07
call %4\qtenv2.bat
pause
cd %5
call %6\jom.exe clean
pause
echo %4\qmake.exe %7\HopsanCore\HopsanCore.pro -r -spec win32-msvc%1 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %4\qmake.exe %7\HopsanCore\HopsanCore.pro -r -spec win32-msvc%1 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %6\jom.exe
cd ..
