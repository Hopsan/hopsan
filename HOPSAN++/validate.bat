@echo off

SETLOCAL ENABLEDELAYEDEXPANSION

cd Models\"Example Models"
copy *.hmf ..\"Validation Models"
cd  ..\"Validation Models"

for %%x in (*.txt) do (
    cd ..\..\

    set "name=..\Models\Validation Models\%%x"
    set name=!name:~0,-4!
    call :performComponentTest "!name!"
    cd Models\"Validation Models"
)
goto:finished



:performComponentTest
cd bin
FOR /F "tokens=*" %%R IN ('HopsanCLI -t "%~1"') DO SET MY_OUTPUT_VAR=%%R

echo "!MY_OUTPUT_VAR!"

IF NOT "!MY_OUTPUT_VAR:~,15!"=="Test successful" (
    echo Aborting!
    pause
)
cd ..

goto:eof




:finished

pause