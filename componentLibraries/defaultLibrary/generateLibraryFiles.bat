@echo off
python.exe generateLibraryFiles.py .
if "%~1" neq "-nopause" pause
