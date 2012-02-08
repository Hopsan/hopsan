::$Id$
@ECHO OFF
cd doc
doxygen devDoxyfile
cd..
echo.
echo You can read the documentation by opening the file ./doc/dev/html/index.html
echo.
pause