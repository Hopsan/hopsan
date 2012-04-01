#!/bin/bash
# $Id: makeSourceCodePackage.sh 4161 2012-03-09 16:50:04Z petno25 $

# Shell script for exporting and taringin the Hopsan src code into an orig file for deb package building
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-01
# For use in Hopsan, requires "subversion commandline" installed (apt-get install subversion)

if [ $# -lt 3 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstSrcPackageName tmpDir}"
  exit $E_BADARGS
fi

srcDir="$1"
dstSrcPackage="$2"
tmpDir="3"

# -----------------------------------------------------------------------------
# Export source dirs and files
#
echo Exporting $srcDir to $tmpDir taring to $dstSrcPackage
rm -rf $tmpDir
svn export srcDir $tmpDir
cd $tmpDir

# -----------------------------------------------------------------------------
# Prepare source dirs and files
#

# Clean bin folder
rm -rf ./bin/*

# Remove the inclusion of the svnrevnum file in core. It is only usefull in for dev trunk use
sed "s|.*#include \"svnrevnum.h\"|//#include \"svnrevnum.h\"|g" -i HopsanCore/include/version.h

# Determine the Core Gui and CLI svn rev numbers for this relase
cd HopsanCore; coresvnrev=`../getSvnRevision.sh`; cd ..
cd HopsanGUI; guisvnrev=`../getSvnRevision.sh`; cd ..
cd HopsanCLI; clisvnrev=`../getSvnRevision.sh`; cd ..
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

# Build user documentation
./buildDocumentation.sh user

#------------------------------------------------------------------------------
# gzTar into source package
# 

tar -czf ../$dstSrcPackage *
cd ..

