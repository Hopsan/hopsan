@ECHO OFF

REM Bat script for building ssp4c dependency automatically

setlocal
set basedir=%~dp0
set name=ssp4c
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM build and install
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)
mkdir %builddir%
pushd %builddir%

if "%HOPSAN_BUILD_COMPILER%" == "msvc" (
  cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%installdir% -DSSP4C_BUILD_SHARED=OFF %codedir%
  cmake --build . --config Debug --parallel 8
  cmake --build . --config Debug --target install
  cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%installdir% -DSSP4C_BUILD_SHARED=OFF %codedir%
  cmake --build . --config Release --parallel 8
  cmake --build . --config Release --target install
) else (
  cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%installdir% -DSSP4C_BUILD_SHARED=OFF %codedir%
  cmake --build . --config Release --parallel 8
  cmake --build . --config Release --target install
)
popd

echo.
echo setupSsp4c.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
