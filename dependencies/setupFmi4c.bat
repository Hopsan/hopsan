@ECHO OFF

REM Bat script for building fmi4c dependency automatically

setlocal
set basedir=%~dp0
set name=fmi4c
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
cmake --build . --parallel 8
cmake --build . --target install

xcopy /S /E /Y %codedir%\include %installdir%\include\
xcopy /S /E /Y %codedir%\3rdparty\fmi %installdir%\3rdparty\fmi\

cd %basedir%
echo.
echo setupFmi4c.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
