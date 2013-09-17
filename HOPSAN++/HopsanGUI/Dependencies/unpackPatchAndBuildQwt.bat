REM $Id$
@ECHO OFF

REM Bat script building HopsaGUI dependency QWT automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

REM TODO: Hardcoded to use python27 in the patch file

set filename="qwt-6.1.0"

REM Unpack or checkout
echo.
echo ======================
echo Unpack libQWT
echo ======================
rd \s\q %filename%
mkdir %filename%
REM Use this for svn version
REM svn co svn://svn.code.sf.net/p/qwt/code/trunk/qwt %dst%
REM Unpack using tar
..\..\ThirdParty\7z\7z.exe x %filename%.zip -y

echo.
echo ======================
echo Patch libQWT
echo ======================
..\..\ThirdParty\patch\doit.exe -p0 < %filename%.patch

REM Build
echo.
echo ======================
echo Todo: Build part not yet implemented, build on your own from QtCreator
echo ======================

echo.
pause