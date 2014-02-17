REM $Id$
@ECHO OFF

REM Bat script building HopsaGUI dependency PythonQt automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2012-12-18

REM TODO: Hardcoded to use python27 in the patch file

set pythonqtVersion="PythonQt2.1_Qt4.8"

REM first unpack
echo ====================
echo Unpacking sourcecode
echo ====================
rd \s\q %pythonqtVersion%
..\..\ThirdParty\7z\7z.exe x %pythonqtVersion%.zip -y

REM Now apply patch
echo.
echo =====================
echo Applying Hopsan patch
echo =====================
cd %pythonqtVersion%
..\..\..\ThirdParty\patch\doit.exe -i ..\\%pythonqtVersion%_winMinGW44.patch -p1
cd ..

REM Now build
echo.
echo Todo: Implement this, for now build it manually using QtCreator
REM rd \s\q %pythonqtVersion%-BuildDir
REM mkdir %pythonqtVersion%-BuildDir
REM cd %pythonqtVersion%-BuildDir

echo.
pause