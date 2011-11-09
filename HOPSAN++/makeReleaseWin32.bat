@echo off

:: HOPSAN RELEASE COMPILATION SCRIPT
:: Written by Robert Braun 2011-10-30


:: MANUAL PART (performed by user before running the script)

:: Make sure the branch is updated, and that all changes are commited
:: Check out the release branch for current release number
:: Make sure the "DEVELOPMENT" flag is disabled in HopsanGUI project file
:: Make sure version numbers are correct (in version.h files)
:: Make sure Splash screen shows correct version
:: Build HopsanCore in release mode using using Visual Studio
:: Move "HopsanCore.dll", "HopsanCore.lib" and "HopsanCore.exp" to the binVC directory
:: Build Hopsan in release mode using MingW32
:: Remove any non-release files from "bin" directory
:: Make sure release notes are correctly updated


:: AUTOMATED PART (performed by this script):

:: Define search path variables
set tempDir=c:\temp_release
set innoDir="C:\Program Files\Inno Setup 5
set scriptFile="Hopsan_0.4_update.iss"

:: Create a temporary release directory
mkdir %tempDir%
mkdir %tempDir%\Models
mkDir output

:: Copy "bin" folder to temporary directory
svn export bin %tempDir%\bin\ /s

pause

:: Copy "binVC" folder to temporary directory
svn export binVC %tempDir%\binVC\ /s

:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore %tempDir%\include

:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\Models\Example Models"

:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\Models\Benchmark Models"

:: Export "componentData" SVN directory to temporary directory
svn export HopsanGUI\componentData %tempDir%\componentdata

:: Export "help" SVN directory to temporary directory
svn export docs/help %tempDir%\help

:: Copy "hopsandefaults" file to temporary directory
copy hopsandefaults %tempDir%\hopsandefaults

:: Create zip package
ThirdParty\7z\7z.exe a -tzip release.zip %tempDir%\*
move release.zip output/

pause

:: Execute Inno compile script
%innoDir%\Compil32.exe /cc %scriptFile%

:: Move release notse to output directory
copy Hopsan-release-notes.txt output/

:: Remove temporary directory
rd /s/q %tempDir%

echo Finished!
pause