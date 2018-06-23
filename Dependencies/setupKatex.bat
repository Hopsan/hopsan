@ECHO OFF
REM Bat script for extracting katex release files from git tag 
REM Author: Peter Nordin peter.nordin@liu.se

set basedir=%~dp0
set name=katex
set codedir=%basedir%\%name%_code
set installdir=%basedir%\%name%

REM Copy release files
mkdir %installdir%
xcopy %codedir%\dist\* %installdir% /Y

echo "setupKatex.bat done"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
