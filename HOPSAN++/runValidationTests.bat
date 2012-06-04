::$Id$
:: Hopsan model validation script
:: This script calls HopsanCLI to validate models for all hvc files found.
:: Author: Peter Nordin 2012-05-31
@echo off
SETLOCAL EnableDelayedExpansion

set failed=0
set okPause=1
if "%~1"=="nopause" (
  set okPause=0
)

for /F "delims==" %%x in ('dir /B /S *.hvc') do (
  cd bin
  if not exist HopsanCLI_d.exe (
    if not exist HopsanCLI.exe (
      echo HopsanCLI not found!
      set failed=1
    )
  )
  if exist HopsanCLI_d.exe (
    echo "Evaluating with HopsanCLI_d: %%x"
    HopsanCLI_d.exe -t "%%x"
    if ERRORLEVEL 1 set failed=1 
  )
  if exist HopsanCLI.exe (
    echo "Evaluating with HopsanCLI: %%x"
    HopsanCLI.exe -t "%%x"
    if ERRORLEVEL 1 set failed=1 
  )
  cd ..
)
if %failed% EQU 1 (
  echo ERROR: There was at least one failure!
  pause
  exit /B 1
)
if %okPause% EQU 1 pause
exit /B 0
