::$Id: buildDevDocumentation.bat 4117 2012-03-02 12:33:23Z petno25 $
@ECHO OFF

:: Now we need to add the ghostscript bin folder to path so that formulas can be built, (the 32 bit version MUST be used)
:: Lets add defualt path for installation on both 32 and 64 bit Windows
set PATH=%PATH%;"C:\Program Files (x86)\gs\gs9.05\bin";"C:\Program Files\gs\gs9.05\bin"

if "%~1" == "" (
  echo To few argumnets, requires one argument: user or dev
  echo Or run one of the other .bat files that calls this one
  pause
  exit
)

cd doc
if %~1==user (
  echo Building user documentation!
  REM First remove output folders to be sure documenation is clean
  REM We can afford to do this as user documentation builds relatively fast
  rmdir "user\html\" /s /q
  rmdir "user\latex\" /s /q

  REM Run doxygen
  doxygen userDoxyfile
  
  echo.
  echo You can read the documentation by opening the file ./doc/user/html/index.html
  echo.
)

if %~1==dev (
  echo Building dev documentation!

  REM Run doxygen
  doxygen devDoxyfile
  
  echo.
  echo You can read the documentation by opening the file ./doc/dev/html/index.html
  echo.
  pause
)
cd..
