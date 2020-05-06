@echo off
SETLOCAL EnableDelayedExpansion

set failed=0

cd bin
REM Run test programs
for /r "." %%a in (tst_*.exe) do (
  "%%~fa"
  if !errorlevel! neq 0 (
    echo Test FAILED: "%%~fa"
    set failed=1
  )
)

REM Run HopsanGUI built-in tests
hopsangui.exe --test --platform offscreen
if !errorlevel! neq 0 (
  echo HopsanGUI test FAILED
  set failed=1
)

cd ..
if !failed! equ 1 (
  echo ERROR: At least one unit test failed!
  if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
    pause
  )
  exit /B 1
)

echo All tests passed
if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
  pause
)
