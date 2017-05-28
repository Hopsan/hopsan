@ECHO OFF
REM $Id: buildDevDocumentation.bat 4117 2012-03-02 12:33:23Z petno25 $

REM Now we need to add the Ghostscript bin folder to path so that formulas can be built
REM We need 32-bit Ghostscript version as it seems to be hard-coded in Doxygen (using alias does not work)
REM Lets add default path for installation on both 32 and 64 bit Windows
REM TODO: It would be nice if we would not need to rely on a specific version here, (search for available version by itself)
set PATH=C:\Program Files\gs\gs9.18\bin;C:\Program Files (x86)\gs\gs9.18\bin;%PATH%
set PATH=C:\Program Files\gs\gs9.21\bin;C:\Program Files (x86)\gs\gs9.21\bin;%PATH%
set PATH=C:\Program Files (x86)\Graphviz2.38\bin;%PATH%

where /q gswin32c.exe
if ERRORLEVEL 1 (
  echo Error: gswin32c.exe could not be found
  echo You need to install the 32.bit version of Ghostscript
)

cd doc
if "%~1"=="" (
  echo Building user documentation!
  REM First remove output folders to be sure documentation is clean
  REM We can afford to do this as user documentation builds relatively fast
  if exist html rmdir html /s /q
  if exist latex rmdir latex /s /q

  REM Run doxygen
  doxygen Doxyfile

  echo.
  echo You can read the documentation by opening the file ./doc/html/index.html
  echo.
)

if "%~1"=="full" (
  echo Building full documentation!
  
  where /q dot.exe
  if ERRORLEVEL 1 (
    echo Error: dot.exe could not be found
    echo You need to install Graphviz2.38
  )


  REM Run doxygen
  doxygen Doxyfile_full

  echo.
  echo You can read the documentation by opening the file ./doc/html/index.html
  echo.
  pause
)
cd..
