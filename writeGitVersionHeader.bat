@echo off
REM Windows batch script wrapper around the writeGitVersionHeader.sh bash script
REM Author: Peter Nordin
REM Date:   2017-02-07

set filepath="%~1"
set name="%~2"
set rev="%~3"
set shorthash="%~4"

REM Check where git bash is installed and use it
if defined ProgramFiles(x86) (
	REM Check for 32-bit version, even though system is 64-bit
	if exist "%ProgramFiles(x86)%\Git\bin" (
		set "bash=%ProgramFiles(x86)%\Git\bin\bash.exe"
	) else (
		REM Use 64-bit version
		set "bash=%ProgramFiles%\Git\bin\bash.exe"
	)
) else (
	REM Use 32-bit version
	set "bash=%ProgramFiles%\Git\bin\bash.exe"
)
REM Execute the bash script to do the work
cd %~dp0
"%bash%" --login -i -c "exec ./writeGitVersionHeader.sh %filepath% %name% %rev% %shorthash%"
