@echo off

:: HOPSAN RELEASE COMPILATION SCRIPT
:: Written by Robert Braun 2011-10-30
:: Revised by Robert Braun and Peter Nordin 2012-03-05


:: MANUAL PART (performed by user before running the script)

:: Check out the release branch for current release number
:: Make sure the branch is updated, and that all changes are commited
:: Make sure the "DEVELOPMENT" flag is disabled in HopsanGUI project file
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

:: Define path variables
set version=0.5.3
set tempDir=c:\temp_release
set inkscapeDir="C:\Program Files\Inkscape"
set inkscapeDir2="C:\Program Files (x86)\Inkscape"
set innoDir="C:\Program Files\Inno Setup 5"
set innoDir2="C:\Program Files (x86)\Inno Setup 5"
set scriptFile="HopsanReleaseInnoSetupScript.iss"
set hopsanDir=%CD%


:: Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
IF NOT EXIST %innoDir% (
  IF NOT EXIST %innoDir2% (
    COLOR 04
    echo Inno Setup 5 is not installed in expected place
    echo Aborting!
    pause
    exit
  )
  set innoDir=%innoDir2%
)


:: Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
IF NOT EXIST %inkscapeDir% (
  IF NOT EXIST %inkscapeDir2% (
    COLOR 04
    echo Inkscape is not installed in expected place
    echo Aborting!
    pause
    exit
  )
  set inkscapeDir=%inkscapeDir2%
)


:: Set version numbers (by changing .h files) BEFORE build
ThirdParty\sed-4.2.1\sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"%version%\"|g" -i HopsanCore\include\version.h
ThirdParty\sed-4.2.1\sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"%version%\"|g" -i HopsanGUI\version_gui.h

:: Set splash screen version number
ThirdParty\sed-4.2.1\sed "s|X\.X\.X|%version%|g" -i HopsanGUI\graphics\splash2.svg
%inkscapeDir%\inkscape.exe HopsanGUI/graphics/splash2.svg --export-background=rgb(255,255,255) --export-png HopsanGUI/graphics/splash.png
:: Revert changes in svg
svn revert HopsanGUI\graphics\splash2.svg

:: Make sure development flag is not defined
ThirdParty\sed-4.2.1\sed "s|.*#define DEVELOPMENT|//#define DEVELOPMENT|" -i HopsanGUI\common.h

:: Rename TBB so it is not found when compiling with Visual Studio
IF NOT EXIST HopsanCore\Dependencies\tbb30_20110704oss (
  COLOR 04
  echo Cannot find correct TBB version, you must use tbb30_20110704oss!
  echo Aborting!
  pause
  exit
)

cd HopsanCore\Dependencies
rename tbb30_20110704oss tbb30_20110704oss_nope
cd ..
cd ..


:: BUILD WITH MSVC2008

:: Remove previous files
cd bin
del HopsanCore.dll
del HopsanCore.exp
del HopsanCore.lib
cd ..

:: Create build directory and enter it
mkdir HopsanCore_bd
cd HopsanCore_bd

:: Setup compiler and compile
call C:\"Program Files"\"Microsoft Visual Studio 9.0"\VC\bin\vcvars32.bat
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qtenv2.bat
call C:\Qt\QtCreator\bin\jom.exe clean
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2008 "CONFIG+=release"
call C:\Qt\QtCreator\bin\jom.exe

:: Create build directory
cd ..
rd /s/q HopsanCore_bd
cd bin

IF NOT EXIST HopsanCore.dll (
  COLOR 04
  echo Failed to build HopsanCore with Visual Studio 2008!
  echo Aborting!
  pause
  exit
)

:: Move files to MSVC2008 directory
mkdir MSVC2008
cd MSVC2008
del HopsanCore.dll
del HopsanCore.exp
del HopsanCore.lib
cd ..
copy HopsanCore.dll MSVC2008\HopsanCore.dll 
copy HopsanCore.exp MSVC2008\HopsanCore.exp 
copy HopsanCore.lib MSVC2008\HopsanCore.lib 
cd ..


::BUILD WITH MSVC2010

::Remove previous files
cd bin
del HopsanCore.dll
del HopsanCore.exp
del HopsanCore.lib
cd ..

::Create build directory and enter it
mkdir HopsanCore_bd
cd HopsanCore_bd

::Setup compiler and compile
call C:\"Program Files"\"Microsoft Visual Studio 10.0"\VC\bin\vcvars32.bat
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qtenv2.bat
call C:\Qt\QtCreator\bin\jom.exe clean
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2010 "CONFIG+=release"
call C:\Qt\QtCreator\bin\jom.exe

::Create build directory
cd ..
rd /s/q HopsanCore_bd
cd bin

IF NOT EXIST HopsanCore.dll (
  COLOR 04
  echo Failed to build HopsanCore with Visual Studio 2010!
  echo Aborting!
  pause
  exit
)

:: Move files to MSVC2010 directory
mkdir MSVC2010
cd MSVC2010
del HopsanCore.dll
del HopsanCore.exp
del HopsanCore.lib
cd ..
copy HopsanCore.dll MSVC2010\HopsanCore.dll 
copy HopsanCore.exp MSVC2010\HopsanCore.exp 
copy HopsanCore.lib MSVC2010\HopsanCore.lib 
cd ..

cd HopsanCore\Dependencies
rename tbb30_20110704oss_nope tbb30_20110704oss
cd ..
cd ..


::BUILD WITH MINGW32

::Remove previous files
cd bin
del HopsanGUI.exe
del HopsanCore.dll
del HopsanCore.exp
del HopsanCore.lib
cd ..
cd ..

::Create build directory and enter it
mkdir HopsanGUI_bd
cd HopsanGUI_bd

::Setup compiler and compile
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qtenv2.bat
call C:\Qt\mingw\bin\mingw32-make.exe clean
call C:\Qt\Desktop\Qt\4.7.4\mingw\bin\qmake.exe %hopsanDir%\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
call C:\Qt\mingw\bin\mingw32-make.exe

cd %hopsanDir%\bin

IF NOT EXIST HopsanGUI.exe (
  COLOR 04
  echo Failed to build Hopsan with MinGW32!
  echo Aborting!
  pause
  exit
)
cd ..
cd ..

ECHO Success!

::Remove temporary build files
rd /s/q HopsanGUI_bd
cd %hopsanDir%


:: Create a temporary release directory

mkdir %tempDir%
IF NOT EXIST %tempDir% (
  COLOR 04
  echo Failed to build temporary directory!
  echo Aborting!
  pause
  exit
)
mkdir %tempDir%\models
mkdir %tempDir%\bin
mkdir %tempDir%\doc
mkdir %tempDir%\doc\user
mkdir %tempDir%\doc\user\html
mkDir output
IF NOT EXIST output (
  COLOR 04
  echo Failed to create output folder!
  echo Aborting!
  pause
  exit
)


:: Copy "bin" folder to temporary directory
xcopy bin\*.exe %tempDir%\bin /s
xcopy bin\*.dll %tempDir%\bin /s
xcopy bin\*.a %tempDir%\bin /s
xcopy bin\*.lib %tempDir%\bin /s
xcopy bin\*.exp %tempDir%\bin /s
xcopy bin\python26.zip %tempDir%\bin /s
xcopy bin\python27.zip %tempDir%\bin /s
del %tempDir%\bin\HopsanCLI*
del %tempDir%\bin\HopsanGUI_d.exe
del %tempDir%\bin\HopsanCore_d.dll
del %tempDir%\bin\libHopsanCore_d.a
del %tempDir%\bin\*_d.dll
del %tempDir%\bin\tbb_debug.dll
del %tempDir%\bin\qwtd.dll

set pythonFailed=false
IF NOT EXIST %tempDir%\bin\python26.zip set pythonFailed=true
IF NOT EXIST %tempDir%\bin\python27.zip set pythonFailed=true
IF "%res%" == "true" (
  COLOR 04
  echo Failed to compile installer executable!
  echo Aborting!
  pause
  exit
)


:: Build user documentation
call buildUserDocumentation

IF NOT EXIST doc\user\html\index.html (
  COLOR 04
  echo Failed to build user documentation!
  echo Aborting!
  pause
  exit
)


:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore\include %tempDir%\include


:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\models\Example Models"
del "%tempDir%\Models\Example Models\AircraftActuationSystem.hmf"
del "%tempDir%\Models\Example Models\ElectricVehicle2.hmf"
del "%tempDir%\Models\Example Models\ElectricVehicleSystem.hmf"
del "%tempDir%\Models\Example Models\newtontest.hmf"


:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\models\Benchmark Models"


:: Export "componentData" SVN directory to temporary directory
svn export componentLibraries\defaultLibrary\components %tempDir%\components


::Export "exampleComponentLib" SVN directory to temporary directory
svn export componentLibraries\exampleComponentLib %tempDir%\exampleComponentLib


::Change the include and lib paths to be correct
ThirdParty\sed-4.2.1\sed.exe "s|$${PWD}/../../HopsanCore/include/|$${PWD}/../include/|" -i %tempDir%\exampleComponentLib\exampleComponentLib.pro
ThirdParty\sed-4.2.1\sed.exe "s|$${PWD}/../../bin|$${PWD}/../bin|" -i %tempDir%\exampleComponentLib\exampleComponentLib.pro


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
del "%tempDir%\Scripts\random.py"


:: Copy "hopsandefaults" file to temporary directory
copy hopsandefaults %tempDir%\hopsandefaults


:: Create zip package
echo Creating zip package
 ThirdParty\7z\7z.exe a -tzip Hopsan-%version%-win32-zip.zip %tempDir%\*
 move Hopsan-%version%-win32-zip.zip output/

IF NOT EXIST "output/Hopsan-%version%-win32-zip.zip" (
  COLOR 04
  echo Failed to create zip package!
  echo Aborting!
  pause
  exit
)


:: Execute Inno compile script
echo Executing Inno Setup installer creation
%innoDir%\iscc.exe /o"output" /f"Hopsan-%version%-win32-installer" /dMyAppVersion=%version% %scriptFile%

IF NOT EXIST "output/Hopsan-%version%-win32-installer.exe" (
  COLOR 04
  echo Failed to compile installer executable!
  echo Aborting!
  pause
  exit
)

:: Move release notse to output directory
copy Hopsan-release-notes.txt "output/"

pause

:: Remove temporary directory
rd /s/q %tempDir%

echo Finished!

pause