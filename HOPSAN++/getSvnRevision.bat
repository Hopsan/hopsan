@echo off
set PATH=%PATH%;"C:\Program Files\TortoiseSVN\bin"
for /F "tokens=5" %%i in ('SubWCRev .^|find "Last committed at revision"') do set version=%%i 
echo %version%
