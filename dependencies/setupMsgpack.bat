@ECHO OFF
REM Bat script for installing msgpack-c dependency automatically

setlocal
set basedir=%~dp0
set name=msgpack-c
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

REM Setup PATH
call setHopsanBuildPaths.bat

REM build
mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_SH="CMAKE_SH-NOTFOUND" -DCMAKE_INSTALL_PREFIX=%installdir% %codedir%
cmake --build . --parallel 8
cmake --build . --target install
if not "%HOPSAN_BUILD_DEPENDENCIES_TEST%" == "false" (
  ctest --parallel 8
)


cd %basedir%
echo.
echo setupMsgpack.bat done!"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
