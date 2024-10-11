@ECHO OFF
REM Bat script building libzip dependency automatically

setlocal
set basedir=%~dp0
set name=dcplib
set codedir=%basedir%\%name%-code
set builddir=%basedir%\%name%-build
set installdir=%basedir%\%name%

set xercesdir=%basedir:\=/%xerces

call setHopsanBuildPaths.bat

"%git_path%\..\usr\bin\patch" dcplib-code/include/core/dcp/model/pdu/IpToStr.hpp dcplib-patch.txt

mkdir %builddir%
cd %builddir%
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DLOGGING=ON -DASIO_ROOT="%basedir%\asio-code" -DXercesC_LIBRARY="%xercesdir%/bin/libxerces-c.dll" -DXercesC_INCLUDE_DIR="%xercesdir%/include" -DXercesC_VERSION="3.2.2" -DZIP_LIBRARY="%basedir%\libzip\bin\libzip.dll" -DZIP_INCLUDE_DIR="%basedir%\libzip\include" -DCMAKE_INSTALL_PREFIX="%installdir%" %codedir%
cmake --build . --parallel 8
cmake --build . --target install

cd %basedir%
echo.
echo setupDCPLib.bat done
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
endlocal
