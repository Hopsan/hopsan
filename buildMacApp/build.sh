#!/bin/sh                                                                                                                                                                    
#                                                                                                                                                                            
# @file   makeMacRelease.sh                                                                                                                                                         
# @author Magnus Sethson <magnus.sethson@liu.se>                                                                                                                             
# @date   2015-02-27                                                                                                                                                         
#                                                                                                                                                                            
# @brief Script for building, configuring, compiling and adjusting HopsanGUI for Mac OS X
#                                                                                                                                                                            
#$Id$                                                                                                                   
#  


# If arg 1 is --debug then set for debug build
if [ "$1" == "--debug" ]; then
echo "DEBUG build"
QMAKE_OPTIONS="-r -spec macx-clang CONFIG+=debug CONFIG+=x86_64"
LIBTAG="_d"
#DEPLOY="-always-overwrite -use-debug-libs"
DEPLOY="-always-overwrite"
else
echo "RELEASE build"
QMAKE_OPTIONS="-r -spec macx-clang CONFIG+=x86_64"
LIBTAG=
DEPLOY="-always-overwrite"
fi

qmake $QMAKE_OPTIONS
make -j4 -w

cd bin

macdeployqt HopsanGUI$LIBTAG.app $DEPLOY

cp -prf libHopsanCore$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prf libHopsanGenerator$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prf libSymHop$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

cp -prf ../Dependencies/qwt-6.1.2/lib/libqwt.6.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/doc/user/html
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/HopsanCore/include
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2008_x86
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2010_x86
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2008_x64
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2010_x64

mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary
mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/autoLibs
cp -prf ../componentLibraries/defaultLibrary/libdefaultComponentLibrary$LIBTAG.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary

cp -prf ../componentLibraries/defaultLibrary/defaultComponentLibrary.xml HopsanGUI$LIBTAG.app/Contents/Resources/

cp -prf ../Hopsan-release-notes.txt HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prf ../hopsandefaults HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prf ../licenseHeader HopsanGUI$LIBTAG.app/Contents/Resources/

cd ..

