@ECHO OFF

REM Bat script for building fmi4c dependency automatically

setlocal
set basedir=%~dp0
set name=fmi4c
set codedir=%basedir%\%name%-code
set builddir_r=%basedir%\%name%-build-release
set builddir_d=%basedir%\%name%-build-debug
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM build and install Debug
if exist %builddir_d% (
  echo Removing existing build directory %builddir_d%
  rmdir /S /Q %builddir_d%
)
mkdir %builddir_d%
pushd %builddir_d%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_DEBUG_POSTFIX="d" -DCMAKE_INSTALL_PREFIX=%installdir% -DFMI4C_BUILD_SHARED=OFF -DFMI4C_USE_SYSTEM_ZIP=OFF -DFMI4C_USE_INCLUDED_ZLIB=ON %codedir%
cmake --build . --config Debug --parallel 8
cmake --build . --config Debug --target install
popd

REM build and install Release
if exist %builddir_r% (
  echo Removing existing build directory %builddir_r%
  rmdir /S /Q %builddir_r%
)
mkdir %builddir_r%
pushd %builddir_r%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_DEBUG_POSTFIX="d" -DCMAKE_INSTALL_PREFIX=%installdir% -DFMI4C_BUILD_SHARED=OFF -DFMI4C_USE_SYSTEM_ZIP=OFF -DFMI4C_USE_INCLUDED_ZLIB=ON %codedir%
cmake --build . --config Release --parallel 8
cmake --build . --config Release --target install
popd

echo.
echo setupFmi4c.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
