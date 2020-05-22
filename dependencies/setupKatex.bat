@ECHO OFF
REM Bat script for setup katex

set basedir=%~dp0
set name=katex
set codedir=%basedir%\%name%-code
set installdir=%basedir%\%name%

REM Copy release files
if exist %installdir% (
  rmdir /S /Q %installdir%
)
mkdir %installdir%
xcopy %codedir%\* %installdir% /Y /I /F /S

echo "setupKatex.bat done"
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
