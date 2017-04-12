@echo off
REM Windows batch script wrapper around the getGitInfo.sh bash script
REM Author: Peter Nordin
REM Date:   2017-02-04

set what="%~1"
set directory="%~2"

REM Check where Git for Windows and bundled bash is installed
if defined ProgramFiles(x86) (
	REM Check for 32-bit version, even though system is 64-bit
	if exist "%ProgramFiles(x86)%\Git\bin" (
		set "PATH=%ProgramFiles(x86)%\Git\bin;%PATH%"
	) else (
		REM Assume 64-bit version installed
		set "PATH=%ProgramW6432%\Git\bin;%PATH%"
	)
) else (
	REM Assume 32-bit version installed
	set "PATH=%ProgramFiles%\Git\bin;%PATH%"
)

REM If the default Git for Windows path is not available then assume that git and bash are in the system PATH

REM Check if found else return GITNOTFOUND
where /Q git.exe
if ERRORLEVEL 1 goto GITNOTFOUND

where /Q bash.exe
if ERRORLEVEL 1 goto BASHNOTFOUND

REM Execute the bash script to do the work
cd %~dp0
bash.exe -c "exec ./getGitInfo.sh %what% %directory%"
exit /B %ERRORLEVEL%

:GITNOTFOUND
	echo GITNOTFOUND
	exit /B 1
	
:BASHNOTFOUND
	echo BASHNOTFOUND
	exit /B 1