REM $Id$
@ECHO OFF

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

REM TODO: Hardcoded to use python27 in the patch file

set dst="qwt-trunk"

REM Unpack or checkout
echo.
echo ======================
echo Checking out qwt trunk (will unpack later when stable relase are availible)
echo ======================
rd \s\q %dst%
mkdir %dst%
svn co svn://svn.code.sf.net/p/qwt/code/trunk/qwt %dst%

REM Build
echo.
echo ======================
echo Todo: Build part not yet implemented, build on your own from QtCreator
echo ======================

echo.
pause