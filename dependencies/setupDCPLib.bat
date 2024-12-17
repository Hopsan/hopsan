@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=dcplib
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

call setHopsanBuildPaths.bat

if "%HOPSAN_BUILD_COMPILER%" == "mingw" (
  echo Setting up MinGW expected paths
  set xerces_lib=%basedir:\=/%xerces/bin/libxerces-c.dll
  set xerces_include=%basedir:\=/%xerces/include
  set zip_lib=%basedir%\libzip\bin\libzip.dll
) else (
  echo Setting up MSVC expected paths
  set xerces_lib=%basedir%xerces\lib\xerces-c_3D.lib
  set xerces_include=%basedir%xerces\include
  set zip_lib=%basedir%\libzip\lib\zip.lib
)

set PATH=%PATH_WITH_MSYS%
patch.exe --forward dcplib-code/include/core/dcp/model/pdu/IpToStr.hpp dcplib-patch.txt
set PATH=%PATH_WITHOUT_MSYS%

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DLOGGING=ON ^
      -DASIO_ROOT="%basedir%\asio-code" ^
      -DXercesC_LIBRARY="%xerces_lib%" ^
      -DXercesC_INCLUDE_DIR="%xerces_include%" ^
      -DXercesC_VERSION="3.2.2" ^
      -DZIP_LIBRARY="%zip_lib%" ^
      -DZIP_INCLUDE_DIR="%basedir%\libzip\include" ^
      -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupDCPLib.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
