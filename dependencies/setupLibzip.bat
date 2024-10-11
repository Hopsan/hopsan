@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=libzip
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

set zlibdir=%basedir:\=/%/zlib
echo %zlibdir%

call setHopsanBuildPaths.bat

mkdir %builddir%
cd %builddir%
if "%HOPSAN_BUILD_COMPILER%" == "msvc" (
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DZLIB_INCLUDE_DIR="%zlibdir%/include" -DZLIB_LIBRARY="%zlibdir%/lib/zlib.lib" -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
) else (
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DZLIB_INCLUDE_DIR="%zlibdir%/include" -DZLIB_LIBRARY="%zlibdir%/bin/libzlib.dll" -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
)
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupLibzip.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
