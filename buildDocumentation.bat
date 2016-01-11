REM $Id: buildDevDocumentation.bat 4117 2012-03-02 12:33:23Z petno25 $
@ECHO OFF

REM Now we need to add the ghostscript bin folder to path so that formulas can be built
REM Lets add default path for installation on both 32 and 64 bit Windows
set PATH=C:\Program Files\gs\gs9.18\bin;C:\Program Files (x86)\gs\gs9.18\bin;%PATH%

cd doc
if "%~1"=="" (
  echo Building user documentation!
  REM First remove output folders to be sure documentation is clean
  REM We can afford to do this as user documentation builds relatively fast
  rmdir "html\" /s /q
  rmdir "latex\" /s /q

  REM Run doxygen
  doxygen Doxyfile

  echo.
  echo You can read the documentation by opening the file ./doc/html/index.html
  echo.
)

if "%~1"=="full" (
  echo Building full documentation!

  REM Run doxygen
  doxygen Doxyfile_full

  echo.
  echo You can read the documentation by opening the file ./doc/html/index.html
  echo.
  pause
)
cd..
