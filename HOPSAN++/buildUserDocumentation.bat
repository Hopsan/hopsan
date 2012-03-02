::$Id$
@ECHO OFF
cd doc
:: First remove output folders to be sure documenation is clean
:: We can afford to do this as user documentation builds relatively fast
rmdir /s /q user\html
rmdir /s /q user\latex

:: Now we need to add the ghostscript bin folder to path so that formulas can be built, (the 32 bit version MUST be used)
:: Lets add defualt path for installation on both 32 and 64 bit Windows
set PATH=%PATH%;"C:\Program Files (x86)\gs\gs9.05\bin";"C:\Program Files\gs\gs9.05\bin"

:: Run doxygen
doxygen userDoxyfile
cd..
echo.
echo You can read the documentation by opening the file ./doc/user/html/index.html
echo.