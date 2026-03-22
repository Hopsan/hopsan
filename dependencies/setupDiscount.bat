@ECHO OFF
REM Bat script for building Discount automatically

setlocal
set basedir=%~dp0
set name=discount
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%


call setHopsanBuildPaths.bat

REM Configure with CMake and then build and install
if exist %builddir% (
  echo Removing existing build directory %builddir%
  rmdir /S /Q %builddir%
)

REM Apply build patch
set PATH=%PATH_WITH_MSYS%
REM nothing to patch
set PATH=%PATH_WITHOUT_MSYS%

REM Since libmarkdown does not export any symbols explicitly, a lib file is never created, so let cmake export all symbols
set export_symbols_arg=""
if "%HOPSAN_BUILD_COMPILER%" == "msvc" (
  set export_symbols_arg=-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON
)

cmake -S %codedir%\cmake
      -B %builddir%
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ^
      -G %HOPSAN_BUILD_CMAKE_GENERATOR% ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DBUILD_SHARED_LIBS=ON ^
      -DDISCOUNT_ONLY_LIBRARY=ON ^
      %export_symbols_arg% ^
      -DCMAKE_INSTALL_PREFIX=%installdir%

cmake --build %builddir% --config Release --parallel 8
cmake --build %builddir% --config Release --target install

echo.
echo setupDiscount.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
