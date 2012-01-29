@echo off

:: HOPSAN RELEASE COMPILATION SCRIPT
:: Written by Robert Braun 2011-10-30


:: MANUAL PART (performed by user before running the script)

:: Check out the release branch for current release number
:: Make sure the branch is updated, and that all changes are commited
:: Make sure the "DEVELOPMENT" flag is disabled in HopsanGUI project file
:: Make sure version numbers are correct (in version.h files, both HopsanGUI and HopsanCore)
:: Make sure Splash screen shows correct version
:: Build HopsanCore in release mode using using Visual Studio (without TBB!)
:: Move "HopsanCore.dll", "HopsanCore.lib" and "HopsanCore.exp" to the binVC directory
:: Build Hopsan in release mode using MingW32 (with TBB!)
:: Remove any non-release files from "bin" directory
:: Make sure release notes are correctly updated
:: Update version number in this file
:: --- Run this script ---
:: Validate critical functions in the program:
::  - Loading models
::  - Adding components
::  - Connecting
::  - Simulating
::  - Multi-threaded simulation
::  - Plotting 
::  - Exporting models to Simulink
::  - Example models
:: Upload new files to Polopoly page
:: Place a copy in //alice/fluid/Programs/Hopsan NG/Hopsan-x.y.z/
:: Update version number on Hopsan front page
:: Update version number in hopsannews.html and upload it
:: Update latest version number in Wikipedia
:: If major release: Post it in Flumes news and make a news post in Redmine


:: AUTOMATED PART (performed by this script):

:: Define search path variables
set version=0.4.5
set tempDir=c:\temp_release
set innoDir="C:\Program Files\Inno Setup 5"
set scriptFile="HopsanReleaseInnoSetupScript.iss"

:: Create a temporary release directory
mkdir %tempDir%
mkdir %tempDir%\Models
mkDir output

:: Copy "bin" folder to temporary directory
xcopy bin %tempDir%\bin\ /s
del %tempDir%\bin\hopsan_logfile.txt

:: Copy "binVC" folder to temporary directory
svn export binVC %tempDir%\binVC\

:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore %tempDir%\include
rd /s/q %tempDir%\include\Components\Compgen
rd /s/q %tempDir%\include\Components\Electric
rd /s/q %tempDir%\include\Components\Hydraulic
rd /s/q %tempDir%\include\Components\Mechanic
rd /s/q %tempDir%\include\Components\Signal
::rd /s/q %tempDir%\include\Dependencies

:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\Models\Example Models"
del "%tempDir%\Models\Example Models\AircraftActuationSystem.hmf"

:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\Models\Benchmark Models"

:: Export "componentData" SVN directory to temporary directory
svn export HopsanGUI\componentData %tempDir%\componentData

:: Export "help" SVN directory to temporary directory
svn export docs/help %tempDir%\help

::Export "exampleComponentLib" SVN directory to temporary directory
svn export externalLibs/exampleComponentLib %tempDir%\exampleComponentLib

:: Copy "hopsandefaults" file to temporary directory
copy hopsandefaults %tempDir%\hopsandefaults

:: Create zip package
 ThirdParty\7z\7z.exe a -tzip Hopsan-%version%-win32-zip.zip %tempDir%\*
 move Hopsan-%version%-win32-zip.zip output/

:: Execute Inno compile script
%innoDir%\iscc.exe /o"output" /f"Hopsan-%version%-win32-installer" /dMyAppVersion=%version% %scriptFile%

:: Move release notse to output directory
copy Hopsan-release-notes.txt "output/"

:: Remove temporary directory
rd /s/q %tempDir%

echo Finished!

pause