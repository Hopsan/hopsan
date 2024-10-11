@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=xerces
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

set zlibdir=%basedir:\=/%/zlib
echo %zlibdir%

call setHopsanBuildPaths.bat

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupXerces.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
