#!/bin/bash
# $Id$

# Shell script for exporting and preparingn the Hopsan src code before RELASE build
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-01
# For use in Hopsan, requires "subversion commandline" installed (apt-get install subversion)

if [ $# -lt 5 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir version doDevRelease doBuildInComponents}"
  exit $E_BADARGS
fi

srcDir="$1"
dstDir="$2"
version="$3"
doDevRelease="$4"
doBuildInComponents="$5"

# -----------------------------------------------------------------------------
# Determine the Core Gui and CLI svn rev numbers for this relase
#
cd $srcDir/HopsanCore; coresvnrev=`../getSvnRevision.sh`; cd $OLDPWD
cd $srcDir/HopsanGUI; guisvnrev=`../getSvnRevision.sh`; cd $OLDPWD
cd $srcDir/HopsanCLI; clisvnrev=`../getSvnRevision.sh`; cd $OLDPWD

# -----------------------------------------------------------------------------
# Export source dirs and files
#
echo "Exporting $srcDir to $dstDir for preparation"
rm -rf $dstDir
svn export $srcDir $dstDir

# -----------------------------------------------------------------------------
# Prepare source dirs and files
#
cd $dstDir

# Clean bin folder
rm -rf ./bin/*

# Remove the inclusion of the svnrevnum file in core. It is only usefull in for dev trunk use
sed "s|.*#include \"svnrevnum.h\"|//#include \"svnrevnum.h\"|g" -i HopsanCore/include/version.h

# Set the Core Gui and CLI svn rev numbers for this relase
sed "s|#define HOPSANCORESVNREVISION.*|#define HOPSANCORESVNREVISION \"$coresvnrev\"|g" -i HopsanCore/include/version.h
sed "s|#define HOPSANGUISVNREVISION.*|#define HOPSANGUISVNREVISION \"$guisvnrev\"|g" -i HopsanGUI/version_gui.h
sed "s|#define HOPSANCLISVNREVISION.*|#define HOPSANCLISVNREVISION \"$clisvnrev\"|g" -i HopsanCLI/main.cpp

if [ $doDevRelease = "false" ]; then
  # Set version numbers (by changing .h files) BEFORE build
  sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"$version\"|g" -i HopsanCore/include/version.h
  sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"$version\"|g" -i HopsanGUI/version_gui.h

  # Set splash screen version number
  sed "s|X\.X\.X|$version|g" -i HopsanGUI/graphics/splash2.svg
  inkscape ./HopsanGUI/graphics/splash2.svg --export-background=rgb\(255,255,255\) --export-png ./HopsanGUI/graphics/splash.png
  
  # Make sure development flag is not defined
  sed "s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|g" -i HopsanGUI/HopsanGUI.pro
fi

# Make sure we compile defaultLibrary into core
if [ "$doBuildInComponents" = "true" ]; then
  sed 's|.*DEFINES \*= INTERNALDEFAULTCOMPONENTS|DEFINES *= INTERNALDEFAULTCOMPONENTS|g' -i HopsanCore/HopsanCore.pro
  sed 's|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/code/defaultComponentLibraryInternal.cc \\|g' -i HopsanCore/HopsanCore.pro
  sed 's|componentLibraries||g' -i HopsanNG.pro
fi

# Build user documentation
./buildDocumentation.sh user

