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

cd Dependencies

. unpackPatchAndBuildQwt.sh --force
. unpackAndBuildFMILibrary.sh --force

cd ..

qmake -r
make -j4 -w

cd bin

macdeployqt HopsanGUI.app
cp -prf libHopsanCore.1.dylib HopsanGUI.app/Contents/Frameworks/
cp -prf libSymHop.1.dylib HopsanGUI.app/Contents/Frameworks/
cp -prf ../Dependencies/qwt-6.1.2/lib/libqwt.6.dylib HopsanGUI.app/Contents/Frameworks/
mkdir -p HopsanGUI.app/Contents/Frameworks/componentLibraries/defaultLibrary
mkdir -p HopsanGUI.app/Contents/Frameworks/componentLibraries/autoLibs
mkdir -p HopsanGUI.app/Contents/Resources/doc/user/html
mkdir -p HopsanGUI.app/Contents/Resources/HopsanCore/include
mkdir -p HopsanGUI.app/Contents/Resources/MSVC2008_x86
mkdir -p HopsanGUI.app/Contents/Resources/MSVC2010_x86
mkdir -p HopsanGUI.app/Contents/Resources/MSVC2008_x64
mkdir -p HopsanGUI.app/Contents/Resources/MSVC2010_x64
cp -prf ../componentLibraries/defaultLibrary/libdefaultComponentLibrary.dylib HopsanGUI.app/Contents/Frameworks/componentLibraries/defaultLibrary
cp -prf ../componentLibraries/defaultLibrary/defaultComponentLibrary.xml HopsanGUI.app/Contents/Resources/
cp -prf Hopsan-release-notes.txt HopsanGUI.app/Contents/Resources/
cp -prf hopsandefaults HopsanGUI.app/Contents/Resources/
cp -prf licenseHeader HopsanGUI.app/Contents/Resources/

echo HopsanGUI.app built!

