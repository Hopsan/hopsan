#!/bin/sh                                                                                                                                                                    
#                                                                                                                                                                            
# @file   makeMacRelease.sh                                                                                                                                                  
# @author Magnus Sethson <magnus.sethson@liu.se>                                                                                                                             
# @date   2015-02-27                                                                                                                                                         
#                                                                                                                                                                            
# @brief Script for building, configuring, compiling and adjusting HopsanGUI for Mac OS X
#


buildRoot="packaging/mac-app/"
name="hopsan"
baseversion=2.23.1
releaserevision=20190827.1035 # TODO use getGitInfoScript
fullversionname=${baseversion}.${releaserevision}
doDevRelease=true
doBuildInComponents=false

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
HOPSAN_CODE=$(pwd)/../../
STAGE_DIR=$(pwd)/stage
../prepareSourceCode.sh ${HOPSAN_CODE} ${STAGE_DIR} ${baseversion} ${releaserevision} ${fullversionname} ${doDevRelease} ${doBuildInComponents}
cd ..

qmake $QMAKE_OPTIONS
make -j4 -w

cd bin

macdeployqt HopsanGUI$LIBTAG.app $DEPLOY

# TODO Use ../copyInstallHopsan.sh

cp -prfX libhopsancore$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prfX libhopsangenerator$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/
cp -prfX libsymhop$LIBTAG.1.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

cp -prfX ../dependencies/qwt-6.1.2/lib/libqwt.6.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/HopsanCore/include

mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary
mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/exampleComponentLib
mkdir -p HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/autoLibs
cp -prfX ${STAGE_DIR}/componentLibraries/defaultLibrary/libdefaultcomponentlibrary$LIBTAG.dylib HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary/
cp -prfX ${STAGE_DIR}/componentLibraries/exampleComponentLib/[HMSeh]* HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/exampleComponentLib/

cp -prfX ${STAGE_DIR}/Hopsan-release-notes.txt HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prfX ${STAGE_DIR}/hopsan-default-configuration.xml HopsanGUI$LIBTAG.app/Contents/Resources/
cp -prfX ${STAGE_DIR}/licenseHeader HopsanGUI$LIBTAG.app/Contents/Resources/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/Models
cp -prfX ${STAGE_DIR}/Models/[BEC]* HopsanGUI$LIBTAG.app/Contents/Resources/Models/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/Scripts
cp -prfX ${STAGE_DIR}/Scripts/[HOp]* HopsanGUI$LIBTAG.app/Contents/Resources/Scripts/

cp -prfX ${STAGE_DIR}/HopsanCore HopsanGUI$LIBTAG.app/Contents/Resources/

cp -prfX ${STAGE_DIR}/componentLibraries/defaultLibrary/* HopsanGUI$LIBTAG.app/Contents/Frameworks/componentLibraries/defaultLibrary/

mkdir -p HopsanGUI$LIBTAG.app/Contents/Resources/doc/user
cp -prfX ${STAGE_DIR}/doc/user/html HopsanGUI$LIBTAG.app/Contents/Resources/doc/user

cd ..
