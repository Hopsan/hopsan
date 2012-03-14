@echo off
setlocal enabledelayedexpansion

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
set devversion=0.6.x
set tempDir=C:\temp_release
set inkscapeDir="C:\Program Files\Inkscape"
set inkscapeDir2="C:\Program Files (x86)\Inkscape"
set innoDir="C:\Program Files\Inno Setup 5"
set innoDir2="C:\Program Files (x86)\Inno Setup 5"      
set scriptFile="HopsanReleaseInnoSetupScript.iss"
set hopsanDir=%CD%
set qtsdkDir="C:\QtSDK"
set qmakeDir="%qtsdkDir%\Desktop\Qt\4.7.4\mingw\bin"
set mingwDir="%qtsdkDir%\mingw\bin"
set jomDir="%qtsdkDir%\QtCreator\bin"
set msvc2008Dir="C:\Program Files\Microsoft SDKs\Windows\v7.0\bin"
set msvc2010Dir="C:\Program Files\Microsoft SDKs\Windows\v7.1\bin"

:: Make sure Qt SDK exist
if not exist %qtsdkDir% (
  COLOR 04
  echo %qtsdkDir% could not be found, you may need to change default dir in this script!
  echo Aborting!
  pause
  exit
)

:: Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
IF NOT EXIST %innoDir% (
  IF NOT EXIST %innoDir2% (
    COLOR 04
    echo Inno Setup 5 is not installed in expected place
    echo Aborting!
    pause
    goto cleanup
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
    goto cleanup
  )
  set inkscapeDir=%inkscapeDir2%
)

set dodevrelease=false
set /P version="Enter release version number on the form a.b.c or leave blank for DEV build release: "
if "%version%"=="" (
  echo Building DEV release
  set version=%devversion%
  set dodevrelease=true
) else (
  echo Release version will be: %version% is this OK?
  set /P ans="Answer y or n: "
  echo isok:!ans!
  if not "!ans!"=="y" (
    COLOR 04
    echo Aborting!
    pause
    goto cleanup
  )
)

if "%dodevrelease%"=="false" (
  REM Set version numbers (by changing .h files) BEFORE build
  ThirdParty\sed-4.2.1\sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"%version%\"|g" -i HopsanCore\include\version.h
  ThirdParty\sed-4.2.1\sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"%version%\"|g" -i HopsanGUI\version_gui.h

  REM Set splash screen version number
  ThirdParty\sed-4.2.1\sed "s|X\.X\.X|%version%|g" -i HopsanGUI\graphics\splash2.svg
  %inkscapeDir%\inkscape.exe HopsanGUI/graphics/splash2.svg --export-background="#ffffff" --export-png HopsanGUI/graphics/splash.png
  REM Revert changes in svg
  svn revert HopsanGUI\graphics\splash2.svg

  REM Make sure development flag is not defined
  ThirdParty\sed-4.2.1\sed "s|.*#define DEVELOPMENT|//#define DEVELOPMENT|" -i HopsanGUI\common.h
)

:: Make sure we compile defaultLibrary into core
ThirdParty\sed-4.2.1\sed "s|.*DEFINES \*= INTERNALDEFAULTCOMPONENTS|DEFINES *= INTERNALDEFAULTCOMPONENTS|g" -i HopsanCore\HopsanCore.pro
ThirdParty\sed-4.2.1\sed "s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc \\|" -i HopsanCore\HopsanCore.pro
ThirdParty\sed-4.2.1\sed "s|componentLibraries||" -i HopsanNG.pro


:: Rename TBB so it is not found when compiling with Visual Studio
IF NOT EXIST HopsanCore\Dependencies\tbb30_20110704oss (
  if not exist HopsanCore\Dependencies\tbb30_20110704oss_nope (
    COLOR 04
    echo Cannot find correct TBB version, you must use tbb30_20110704oss!
    echo Aborting!
    pause
    goto cleanup
  )
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
call %msvc2008Dir%\SetEnv.cmd /x86
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2008 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe

:: Create build directory
cd ..
rd /s/q HopsanCore_bd
cd bin

IF NOT EXIST HopsanCore.dll (
  COLOR 04
  echo Failed to build HopsanCore with Visual Studio 2008!
  echo Aborting!
  pause
  goto cleanup
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
call %msvc2010Dir%\SetEnv.cmd /x86
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2010 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe

::Create build directory
cd ..
rd /s/q HopsanCore_bd
cd bin

IF NOT EXIST HopsanCore.dll (
  COLOR 04
  echo Failed to build HopsanCore with Visual Studio 2010!
  echo Aborting!
  pause
  goto cleanup
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
call %qmakeDir%\qtenv2.bat
call %mingwDir%\mingw32-make.exe clean
call %qmakeDir%\qmake.exe %hopsanDir%\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
call %mingwDir%\mingw32-make.exe

cd %hopsanDir%\bin

IF NOT EXIST HopsanGUI.exe (
  COLOR 04
  echo Failed to build Hopsan with MinGW32!
  echo Aborting!
  pause
  goto cleanup
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
  goto cleanup
)
mkdir %tempDir%\models
mkdir %tempDir%\scripts
mkdir %tempDir%\bin
mkdir %tempDir%\componentLibraries
mkdir %tempDir%\componentLibraries\defaultLibrary
mkdir %tempDir%\doc
mkdir %tempDir%\doc\user
mkdir %tempDir%\doc\user\html
mkDir output
IF NOT EXIST output (
  COLOR 04
  echo Failed to create output folder!
  echo Aborting!
  pause
  goto cleanup
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
  goto cleanup
)


:: Build user documentation
call buildUserDocumentation

IF NOT EXIST doc\user\html\index.html (
  COLOR 04
  echo Failed to build user documentation!
  echo Aborting!
  pause
  goto cleanup
)


:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore\include %tempDir%\include


:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\models\Example Models"


:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\models\Benchmark Models"


:: Export and copy "componentData" SVN directory to temporary directory
svn export componentLibraries\defaultLibrary\components %tempDir%\componentLibraries\defaultLibrary\components
REM xcopy componentLibraries\defaultLibrary\components\defaultComponentLibrary.dll %tempDir%\componentLibraries\defaultLibrary\components


::Export "exampleComponentLib" SVN directory to temporary directory
svn export componentLibraries\exampleComponentLib %tempDir%\exampleComponentLib


::Change the include and lib paths to be correct
REM ThirdParty\sed-4.2.1\sed.exe "s|$${PWD}/../../HopsanCore/include/|$${PWD}/../include/|" -i %tempDir%\exampleComponentLib\exampleComponentLib.pro
REM ThirdParty\sed-4.2.1\sed.exe "s|$${PWD}/../../bin|$${PWD}/../bin|" -i %tempDir%\exampleComponentLib\exampleComponentLib.pro


:: Export "help" SVN directory to temporary directory
xcopy doc\user\html\* %tempDir%\doc\user\html\ /s
xcopy doc\graphics\* %tempDir%\doc\graphics\ /s


:: Export "Scripts" folder to temporary directory
xcopy Scripts\HopsanOptimization.py %tempDir%\scripts\ /s
xcopy Scripts\OptimizationObjectiveFunctions.py %tempDir%\scripts\ /s
xcopy Scripts\OptimizationObjectiveFunctions.xml %tempDir%\scripts\ /s


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
  goto cleanup
)


:: Execute Inno compile script
echo Executing Inno Setup installer creation
%innoDir%\iscc.exe /o"output" /f"Hopsan-%version%-win32-installer" /dMyAppVersion=%version% %scriptFile%

IF NOT EXIST "output/Hopsan-%version%-win32-installer.exe" (
  COLOR 04
  echo Failed to compile installer executable!
  echo Aborting!
  pause
  goto cleanup
)

:: Move release notse to output directory
copy Hopsan-release-notes.txt "output/"

pause

echo Finished!



:cleanup 

:: Remove temporary directory
rd /s/q %tempDir%

cd HopsanCore\Dependencies
rename tbb30_20110704oss_nope tbb30_20110704oss
cd ..
cd ..

echo Performed cleanup.

pause