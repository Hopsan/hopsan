@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=dcplib
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

call setHopsanBuildPaths.bat

set asiodir=%basedir%\asio-code
set libzipdir=%basedir%\libzip
set xercesdir=%basedir%\xerces

set PATH=%PATH_WITH_MSYS%
patch.exe --forward dcplib-code/include/core/dcp/model/pdu/IpToStr.hpp dcplib-patch.txt
set PATH=%PATH_WITHOUT_MSYS%

mkdir %builddir%
cmake -B"%builddir%" ^
      -S"%codedir%" ^
      -G %HOPSAN_BUILD_CMAKE_GENERATOR% ^
      -Wno-dev ^
      -DLOGGING=ON ^
      -DASIO_ROOT="%asiodir%" ^
      -DCMAKE_PREFIX_PATH="%xercesdir%;%libzipdir%" ^
      -DCMAKE_INSTALL_PREFIX="%installdir%"
cmake --build %builddir% --parallel 8
cmake --build %builddir% --target install

echo.
echo setupDCPLib.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
