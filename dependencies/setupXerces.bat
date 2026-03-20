@ECHO OFF
REM Bat script building libxerces dependency automatically

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
if "%HOPSAN_BUILD_COMPILER%" == "msvc" (
  cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% ^
        -DCMAKE_BUILD_TYPE=Debug ^
        -DCMAKE_INSTALL_PREFIX="%installdir%" ^
        %codedir% --fresh
  cmake --build . --config Debug --parallel 8
  cmake --build . --config Debug --target install
)
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_INSTALL_PREFIX="%installdir%" ^
      %codedir% --fresh
cmake --build . --config Release --parallel 8
cmake --build . --config Release --target install

cd %basedir%
echo.
echo setupXerces.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
