::$Id
@ECHO OFF
cd doc
:: First remove output folders to be sure documenation is clean
:: We can afford to do this as user documentation builds relatively fast
rmdir /s /q user\html
rmdir /s /q user\latex
doxygen userDoxyfile
cd..
echo.
echo You can read the documentation by opening the file ./doc/user/html/index.html
echo.