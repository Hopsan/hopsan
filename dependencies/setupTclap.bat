@ECHO OFF
REM Bat script for setup tclap

set basedir=%~dp0
set name=tclap
set codedir=%basedir%\%name%-code
set installdir=%basedir%\%name%

REM Copy release files
if exist %installdir% (
  rmdir /S /Q %installdir%
)
mkdir %installdir%
xcopy %codedir%\include %installdir%\include /Y /I /F /S

echo "setupTclap.bat done"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
