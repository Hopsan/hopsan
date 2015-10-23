@echo off
for /F "eol=# tokens=*" %%A in (%~2) do (
  echo %%A
  echo "%~1" -s %%A -a "%~3" -a buildcomplib.sh --shellexec "/bin/sh buildcomplib.sh %~4"
  "%~1" -s %%A -a "%~3" -a buildcomplib.sh --shellexec "/bin/sh buildcomplib.sh %~4"
)
echo.
echo Done
pause