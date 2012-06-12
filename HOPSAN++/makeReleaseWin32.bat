@echo off
setlocal enabledelayedexpansion

::$Id$
:: HOPSAN RELEASE COMPILATION SCRIPT
:: Written by Robert Braun 2011-10-30
:: Revised by Robert Braun and Peter Nordin 2012-03-05


:: MANUAL PART (performed by user before running the script)

:: Check out the release branch for current release number
:: Make sure the branch is updated, and that all changes are commited
:: Make sure release notes are correctly updated
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
set devversion=0.6.
set tbbversion=tbb30_20110704oss
set tempDir=C:\temp_release
set inkscapeDir="C:\Program Files\Inkscape"
set inkscapeDir2="C:\Program Files (x86)\Inkscape"
set innoDir="C:\Program Files\Inno Setup 5"
set innoDir2="C:\Program Files (x86)\Inno Setup 5"
set scriptFile="HopsanReleaseInnoSetupScript.iss"
set hopsanDir=%CD%
set qtsdkDir="C:\Qt"
set qtsdkDir2="C:\QtSDK"
set msvc2008Dir="C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin"
set msvc2010Dir="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin"
set dependecyBinFiles=hopsan_bincontents_Qt474_MinGW_Py27.7z
                
:: Make sure Qt SDK exist
if not exist %qtsdkDir% (
  call :abortIfNotExist %qtsdkDir2% "Qt SDK could not be found in one of the expected locations."
  set qtsdkDir=%qtsdkDir2%
)

set jomDir="%qtsdkDir%\QtCreator\bin"
set qmakeDir="%qtsdkDir%\Desktop\Qt\4.7.4\mingw\bin"
set mingwDir="%qtsdkDir%\mingw\bin"

:: Make sure the correct inno dir is used, 32 or 64 bit computers (Inno Setup is 32-bit)
IF NOT EXIST %innoDir% (
  call :abortIfNotExist %innoDir2% "Inno Setup 5 is not installed in expected place."
  set innoDir=%innoDir2%
)


:: Make sure the correct incskape dir is used, 32 or 64 bit computers (Inkscape is 32-bit)
IF NOT EXIST %inkscapeDir% (
  call :abortIfNotExist %inkscapeDir2% "Inkscape is not installed in expected place"
  set inkscapeDir=%inkscapeDir2%
)

:: Make sure the 3d party dependency file exists
call :abortIfNotExist %dependecyBinFiles% "The %dependecyBinFiles% file containing needed bin files is NOT present. Get it from alice/fluid/programs/hopsan"
set dependecyBinFiles=%dependecyBinFiles%

set dodevrelease=false
set /P version="Enter release version number on the form a.b.c or leave blank for DEV build release: "
if "%version%"=="" (
  echo Building DEV release
  call getSvnRevision.bat
  set /P revnum="Enter the revnum shown above: "
  call set version=%devversion%r!revnum!
  set dodevrelease=true
)
echo.
echo ---------------------------------------
echo This is a DEV release: %dodevrelease%
echo Release version number: %version%
echo ---------------------------------------
echo Is this OK?
set /P ans="Answer y or n: "
call :abortIfStrNotMatch "%ans%" "y"

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
  ThirdParty\sed-4.2.1\sed "s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|" -i HopsanGUI\HopsanGUI.pro
)

:: Make sure we compile defaultLibrary into core
ThirdParty\sed-4.2.1\sed "s|.*DEFINES \*= BUILTINDEFAULTCOMPONENTLIB|DEFINES *= BUILTINDEFAULTCOMPONENTLIB|g" -i Common.prf
ThirdParty\sed-4.2.1\sed "s|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc \\|" -i HopsanCore\HopsanCore.pro
ThirdParty\sed-4.2.1\sed "s|componentLibraries||" -i HopsanNG.pro


:: Rename TBB so it is not found when compiling with Visual Studio
IF NOT EXIST HopsanCore\Dependencies\%tbbversion% (
  call :abortIfNotExist HopsanCore\Dependencies\%tbbversion%_nope "Cannot find correct TBB version, you must use %tbbversion%"
)

cd HopsanCore\Dependencies
rename %tbbversion% %tbbversion%_nope
cd ..
cd ..


:: BUILD HOPSANCORE WITH MSVC2008 32-bit

:: Remove previous files
del /q bin\HopsanCore*.*

:: Create build directory and enter it
rd \s\q HopsanCore_bd
mkdir HopsanCore_bd
cd HopsanCore_bd

:: Setup compiler and compile
call "C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin"\SetEnv.cmd /Release /x86
pause
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2008 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe
cd ..

:: Remove build directory
rd /s/q HopsanCore_bd

cd bin
call :abortIfNotExist HopsanCore.dll "Failed to build HopsanCore with Visual Studio 2008 32-bit"

:: Move files to MSVC2008 directory, clear it first
mkdir MSVC2008_x86
del /q MSVC2008_x86\*.*
move HopsanCore.dll MSVC2008_x86\HopsanCore.dll 
move HopsanCore.exp MSVC2008_x86\HopsanCore.exp 
move HopsanCore.lib MSVC2008_x86\HopsanCore.lib 
cd ..


:: BUILD HOPSANCORE WITH MSVC2008 64-bit

:: Remove previous files
del /q bin\HopsanCore*.*

:: Create build directory and enter it
rd \s\q HopsanCore_bd
mkdir HopsanCore_bd
cd HopsanCore_bd

:: Setup compiler and compile
call %msvc2008Dir%\SetEnv.cmd /Release /x64
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2008 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe
cd ..

:: Remove build directory
rd /s/q HopsanCore_bd

cd bin
call :abortIfNotExist HopsanCore.dll "Failed to build HopsanCore with Visual Studio 2008 64-bit"

:: Move files to MSVC2008 directory
mkdir MSVC2008_x64
del /q MSVC2008_x64\*.*
move HopsanCore.dll MSVC2008_x64\HopsanCore.dll 
move HopsanCore.exp MSVC2008_x64\HopsanCore.exp 
move HopsanCore.lib MSVC2008_x64\HopsanCore.lib 
cd ..


::BUILD WITH MSVC2010 (32-bit)

::Remove previous files
del /q bin\HopsanCore*.*

::Create build directory and enter it
rd \s\q HopsanCore_bd
mkdir HopsanCore_bd
cd HopsanCore_bd

::Setup compiler and compile
call %msvc2010Dir%\SetEnv.cmd /Release /x86
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2010 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe
cd ..

:: Remove build directory
rd /s/q HopsanCore_bd

cd bin
call :abortIfNotExist HopsanCore.dll "Failed to build HopsanCore with Visual Studio 2010 32-bit"

:: Move files to MSVC2010 directory
mkdir MSVC2010_x86
del /q MSVC2010_x86\*.*
move HopsanCore.dll MSVC2010_x86\HopsanCore.dll 
move HopsanCore.exp MSVC2010_x86\HopsanCore.exp 
move HopsanCore.lib MSVC2010_x86\HopsanCore.lib 
cd ..

::BUILD WITH MSVC2010 (64-bit)

::Remove previous files
del /q bin\HopsanCore*.*

::Create build directory and enter it
rd \s\q HopsanCore_bd
mkdir HopsanCore_bd
cd HopsanCore_bd

::Setup compiler and compile
call %msvc2010Dir%\SetEnv.cmd /Release /x64
call %qmakeDir%\qtenv2.bat
call %jomDir%\jom.exe clean
call %qmakeDir%\qmake.exe ..\HopsanCore\HopsanCore.pro -r -spec win32-msvc2010 "CONFIG+=release" "QMAKE_CXXFLAGS_RELEASE += -wd4251"
call %jomDir%\jom.exe
cd ..

::Remove build directory
rd /s/q HopsanCore_bd

cd bin
call :abortIfNotExist HopsanCore.dll "Failed to build HopsanCore with Visual Studio 2010 64-bit"

:: Move files to MSVC2010 directory
mkdir MSVC2010_x64
del /q MSVC2010_x64\*.*
move HopsanCore.dll MSVC2010_x64\HopsanCore.dll 
move HopsanCore.exp MSVC2010_x64\HopsanCore.exp 
move HopsanCore.lib MSVC2010_x64\HopsanCore.lib 
cd ..


::Activate TBB
cd HopsanCore\Dependencies
rename %tbbversion%_nope %tbbversion%
cd ..
cd ..


::BUILD WITH MINGW32

::Remove previous files
del /q bin\HopsanCore*.*
del /q bin\HopsanGUI*.*
del /q bin\HopsanCLI*.*

::Create build directory and enter it
rd /s/q HopsanNG_bd
mkdir HopsanNG_bd
cd HopsanNG_bd

::Setup compiler and compile
call %qmakeDir%\qtenv2.bat
call %mingwDir%\mingw32-make.exe clean
call %qmakeDir%\qmake.exe %hopsanDir%\HopsanNG.pro -r -spec win32-g++ "CONFIG+=release"
call %mingwDir%\mingw32-make.exe
cd ..

cd bin
call :abortIfNotExist HopsanCore.dll "Failed to build HopsanCore with MinGW32"
call :abortIfNotExist HopsanGUI.exe "Failed to build HopsanGUI with MinGW32"
call :abortIfNotExist HopsanCLI.exe "Failed to build HopsanCLI with MinGW32"
cd ..

ECHO Success!

::Remove temporary build files
rd /s/q HopsanNG_bd
cd %hopsanDir%


::Perform model testing
call runValidationTests.bat nopause
if ERRORLEVEL 1 (
    echo Validation failed Aborting!
    pause
    goto cleanup
)

:: Create a temporary release directory
mkdir %tempDir%
call :abortIfNotExist %tempDir% "Failed to build temporary directory"

mkdir %tempDir%\models
mkdir %tempDir%\scripts
mkdir %tempDir%\bin
mkdir %tempDir%\HopsanCore
mkdir %tempDir%\componentLibraries
mkdir %tempDir%\componentLibraries\defaultLibrary
mkdir %tempDir%\doc
mkdir %tempDir%\doc\user
mkdir %tempDir%\doc\user\html

:: Unpack depedency bin files to bin folder
ThirdParty\7z\7z.exe x %dependecyBinFiles% -o%tempDir%\bin

::Clear old output folder
rd /s/q output
IF EXIST output (
  COLOR 0C
  echo Unable to clear old output folder. Continue?
  set /P ans="Answer y or n: "
  call :abortIfStrNotMatch "!ans!" "y"
  COLOR 07
)
  
::Create new output folder
mkDir output
call :abortIfNotExist output "Failed to create output folder"


:: Copy "bin" folder to temporary directory
xcopy bin\*.exe %tempDir%\bin /s
xcopy bin\*.dll %tempDir%\bin /s
xcopy bin\*.a %tempDir%\bin /s
xcopy bin\*.lib %tempDir%\bin /s
xcopy bin\*.exp %tempDir%\bin /s
xcopy bin\python27.zip %tempDir%\bin /s
del %tempDir%\bin\HopsanCLI_d.exe
del %tempDir%\bin\HopsanGUI_d.exe
del %tempDir%\bin\HopsanCore_d.dll
del %tempDir%\bin\libHopsanCore_d.a
del %tempDir%\bin\*_d.dll
del %tempDir%\bin\tbb_debug.dll
del %tempDir%\bin\qwtd.dll

set pythonFailed=true
IF EXIST %tempDir%\bin\python27.zip set pythonFailed=false
IF "%pythonFailed%" == "true" (
  echo Failed to find python27.zip.
)
call :abortIfStrNotMatch %pythonFailed% "false"
:: TODO build in OPTIONAL theird argument message support in abortIf subroutine


:: Build user documentation
call buildUserDocumentation
call :abortIfNotExist doc\user\html\index.html "Failed to build user documentation"

:: Export "HopsanCore" SVN directory to "include" in temporary directory
svn export HopsanCore\include %tempDir%\HopsanCore\include
:: Copy the svnrevnum.h file Assume it exist, ONLY for DEV builds
if "%dodevrelease%"=="true" (
  xcopy HopsanCore\include\svnrevnum.h %tempDir%\HopsanCore\include\ /s
)


:: Export "Example Models" SVN directory to temporary directory
svn export "Models\Example Models" "%tempDir%\models\Example Models"


:: Export "Benchmark Models" SVN directory to temporary directory
svn export "Models\Benchmark Models" "%tempDir%\models\Benchmark Models"


:: Export and copy "componentData" SVN directory to temporary directory
svn export componentLibraries\defaultLibrary\components %tempDir%\componentLibraries\defaultLibrary\components
REM xcopy componentLibraries\defaultLibrary\components\defaultComponentLibrary.dll %tempDir%\componentLibraries\defaultLibrary\components


::Export "exampleComponentLib" SVN directory to temporary directory
svn export componentLibraries\exampleComponentLib %tempDir%\componentLibraries\exampleComponentLib


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
call :abortIfNotExist "output/Hopsan-%version%-win32-zip.zip" "Failed to create zip package"


:: Execute Inno compile script
echo Executing Inno Setup installer creation
%innoDir%\iscc.exe /o"output" /f"Hopsan-%version%-win32-installer" /dMyAppVersion=%version% %scriptFile%
call :abortIfNotExist "output/Hopsan-%version%-win32-installer.exe" "Failed to compile installer executable"

:: Move release notse to output directory
copy Hopsan-release-notes.txt "output/"

echo Finished
pause
goto cleanup 

:abortIfStrNotMatch
if not "%~1"=="%~2" (
    COLOR 0C
    echo Aborting
    pause
    goto cleanup
)
goto:eof

:abortIfNotExist
if not exist %1 (
  COLOR 0C
  echo %~2
  echo Aborting
  pause
  goto cleanup
)
goto:eof


:cleanup 
echo.
echo Performing cleanup
:: Remove temporary directory
rd /s/q %tempDir%

cd %hopsanDir%
cd HopsanCore\Dependencies
rename %tbbversion%_nope %tbbversion%
echo Performed cleanup
cd %hopsanDir%
echo.
echo This script have changed the contents of some .pro .h .png files. You SHOULD REVERT them. Do NOT commit these automatic changes.
echo.
pause
exit
