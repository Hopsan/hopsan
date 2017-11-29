@echo off
REM Arg1: remoteclient
REM Arg2: nodelist
REM Arg3: userid
REM Arg4: libfile
REM Arg5: libname
for /F "eol=# tokens=*" %%A in (%~2) do (
  echo %%A
  echo "%~1" -s %%A -u "%~3" -a "%~4" -a buildcomplib.sh --shellexec "/bin/bash buildcomplib.sh %~5"
  "%~1" -s %%A -u "%~3" -a "%~4" -a buildcomplib.sh --shellexec "/bin/bash buildcomplib.sh %~5"
)
echo.
echo Done
pause