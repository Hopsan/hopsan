@ECHO OFF

REM Bat script building FMILibrary dependency automatically

setlocal
set basedir=%~dp0
set name=fmilibrary
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

call setHopsanBuildPaths.bat

REM build
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DFMILIB_FMI_PLATFORM="win64" -DFMILIB_INSTALL_PREFIX=%installdir% %codedir%
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupFMILibrary.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
