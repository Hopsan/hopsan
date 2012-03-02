::$Id$
@ECHO OFF

:: Now we need to add the ghostscript bin folder to path so that formulas can be built, (the 32 bit version MUST be used)
:: Lets add defualt path for installation on both 32 and 64 bit Windows
set PATH=%PATH%;"C:\Program Files (x86)\gs\gs9.05\bin";"C:\Program Files\gs\gs9.05\bin"

cd doc
doxygen devDoxyfile
cd..
echo.
echo You can read the documentation by opening the file ./doc/dev/html/index.html
echo.
pause