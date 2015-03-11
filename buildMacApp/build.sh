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

buildRoot="buildMacApp/"
name="hopsan"
version=0.7.

# If arg 1 is --debug then set for debug build
if [ "$1" == "--debug" ]; then
echo "DEBUG build"
QMAKE_OPTIONS="-r -spec macx-clang CONFIG+=debug CONFIG+=x86_64"
LIBTAG="_d"
#DEPLOY="-always-overwrite -use-debug-libs"
DEPLOY="-always-overwrite -verbose=3"
else
echo "RELEASE build"
QMAKE_OPTIONS="-r -spec macx-clang CONFIG+=x86_64"
LIBTAG=
DEPLOY="-always-overwrite -verbose=3"
fi

cd $buildRoot
./prepareSourceCode.sh ../  stage $version true false
cd ..

qmake $QMAKE_OPTIONS
make -j4 -w

cd bin

macdeployqt HopsanGUI$LIBTAG.app $DEPLOY

cp -prfX libHopsanCore$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prfX libHopsanGenerator$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prfX libSymHop$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

cp -prfX ../Dependencies/qwt-6.1.2/lib/libqwt.6.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/HopsanCore/include
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2008_x86
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2010_x86
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2008_x64
mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/MSVC2010_x64

mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary
mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/exampleComponentLib
mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/autoLibs
cp -prfX ../componentLibraries/defaultLibrary/libdefaultComponentLibrary$LIBTAG.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary/
cp -prfX ../componentLibraries/exampleComponentLib/[HMSeh]* HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/exampleComponentLib/

cp -prfX ../Hopsan-release-notes.txt HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prfX ../hopsandefaults HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prfX ../licenseHeader HopsanGUI$LIBTAG.app/Contents/Resources/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/Models
cp -prfX ../Models/[BEC]* HopsanGUI$LIBTAG.app/Contents/Resources/Models/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/Scripts
cp -prfX ../Scripts/[HOp]* HopsanGUI$LIBTAG.app/Contents/Resources/Scripts/

cp -prfX ../$buildRoot/stage/HopsanCore HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prfX ../$buildRoot/stage/ThirdParty HopsanGUI$LIBTAG.app/Contents/Resources/

cp -prfX ../$buildRoot/stage/componentLibraries/defaultLibrary/* HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/doc/user
cp -prfX ../$buildRoot/stage/doc/user/html HopsanGUI$LIBTAG.app/Contents/Resources/doc/user

cd ..

#rm -fr $buildRoot/stage

