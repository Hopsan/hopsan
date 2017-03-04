@echo off
REM Windows batch script wrapper around the writeGitVersionHeader.sh bash script
REM Author: Peter Nordin
REM Date:   2017-02-07

set filepath="%~1"
set name="%~2"
set rev="%~3"
set shorthash="%~4"

REM Check where Git for Windows and bundled bash is installed
if defined ProgramFiles(x86) (
	REM Check for 32-bit version, even though system is 64-bit
	if exist "%ProgramFiles(x86)%\Git\bin" (
		set "PATH=%ProgramFiles(x86)%\Git\bin;%PATH%"
	) else (
		REM Assume 64-bit version installed
		set "PATH=%ProgramFiles%\Git\bin;%PATH%"
	)
) else (
	REM Assume 32-bit version installed
	set "PATH=%ProgramFiles%\Git\bin;%PATH%"
)

REM If the default Git for Windows path is not available then assume that git and bash are in the system PATH

REM Check if found else return GITNOTFOUND
where /Q git.exe
if ERRORLEVEL 1 goto NOTFOUND

where /Q bash.exe
if ERRORLEVEL 1 goto NOTFOUND

REM Execute the bash script to do the work
cd %~dp0
bash.exe -i -c "exec ./writeGitVersionHeader.sh %filepath% %name% %rev% %shorthash%"
exit /B %ERRORLEVEL%

:NOTFOUND
	echo GITNOTFOUND
	exit /B 1