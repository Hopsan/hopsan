::$Id
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
set version=0.5.1
set tempDir=c:\temp_release
set innoDir="C:\Program Files\Inno Setup 5"
set innoDir2="C:\Program Files (x86)\Inno Setup 5"
set scriptFile="HopsanReleaseInnoSetupScript.iss"

:: Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
IF NOT EXIST %innoDir% (
  IF NOT EXIST %innoDir2% (
    echo Inno Setup 5 is not installed in expected place
    echo Aborting!
    pause
    exit
  )
  set innoDir=%innoDir2%
)

:: Create a temporary release directory
mkdir %tempDir%
mkdir %tempDir%\models
mkdir %tempDir%\bin
mkdir %tempDir%\doc
mkdir %tempDir%\doc\user
mkdir %tempDir%\doc\user\html
mkDir output

:: Copy "bin" folder to temporary directory
xcopy bin\*.exe %tempDir%\bin /s
xcopy bin\*.dll %tempDir%\bin /s
xcopy bin\*.a %tempDir%\bin /s
xcopy bin\*.lib %tempDir%\bin /s
del %tempDir%\bin\HopsanCLI.*
del %tempDir%\bin\HopsanGUI_d.exe
del %tempDir%\bin\HopsanCore_d.dll
del %tempDir%\bin\libHopsanCore_d.a
xcopy bin\MSVC2008 %tempDir%\bin\MSVC2008 /s

:: Build user documentation
call buildUserDocumentation

:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore\include %tempDir%\include

:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\models\Example Models"
del "%tempDir%\Models\Example Models\AircraftActuationSystem.hmf"
del "%tempDir%\Models\Example Models\ElectricVehicle2.hmf"
del "%tempDir%\Models\Example Models\ElectricVehicleSystem.hmf"

:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\models\Benchmark Models"

:: Export "componentData" SVN directory to temporary directory
svn export componentLibraries\defaultLibrary\components %tempDir%\components

::Export "exampleComponentLib" SVN directory to temporary directory
svn export componentLibraries\exampleComponentLib %tempDir%\exampleComponentLib

:: Export "help" SVN directory to temporary directory
xcopy doc\user\html\* %tempDir%\doc\user\html\ /s
xcopy doc\graphics\* %tempDir%\doc\graphics\ /s

:: Export "Scripts" folder to temporary directory
xcopy Scripts\* %tempDir%\Scripts\ /s
del "%tempDir%\Scripts\benchmark.py"
del "%tempDir%\Scripts\opttest.py"
del "%tempDir%\Scripts\plot.py"
del "%tempDir%\Scripts\speedtest.py"
del "%tempDir%\Scripts\speedtest1.py"
del "%tempDir%\Scripts\speedtest2.py"
del "%tempDir%\Scripts\speedtest4.py"
del "%tempDir%\Scripts\speedtest8.py"

:: Copy "hopsandefaults" file to temporary directory
copy hopsandefaults %tempDir%\hopsandefaults

:: Create zip package
echo Creating zip package
 ThirdParty\7z\7z.exe a -tzip Hopsan-%version%-win32-zip.zip %tempDir%\*
 move Hopsan-%version%-win32-zip.zip output/

:: Execute Inno compile script
echo Executing Inno Setup installer creation
%innoDir%\iscc.exe /o"output" /f"Hopsan-%version%-win32-installer" /dMyAppVersion=%version% %scriptFile%

:: Move release notse to output directory
copy Hopsan-release-notes.txt "output/"

:: Remove temporary directory
rd /s/q %tempDir%

echo Finished!

pause