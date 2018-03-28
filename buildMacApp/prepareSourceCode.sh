#!/bin/bash
# $Id$

# Shell script for exporting and preparing the Hopsan source code before RELEASE build
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-04-01
# For use in Hopsan, requires "subversion commandline" installed (apt-get install subversion)

E_BADARGS=65
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
Inkscape="/Applications/Inkscape.app/Contents/MacOS/Inkscape"

# -----------------------------------------------------------------------------
# Determine the Core Gui and CLI svn rev numbers for this release
#
cd $srcDir/HopsanCore; coresvnrev=`../getSvnRevision.sh`; cd $OLDPWD
cd $srcDir/HopsanGUI; guisvnrev=`../getSvnRevision.sh`; cd $OLDPWD
cd $srcDir/HopsanCLI; clisvnrev=`../getSvnRevision.sh`; cd $OLDPWD

echo $coresvnrev
echo $guisvnrev
echo $clisvnrev

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

# Remove the inclusion of the svnrevnum file in core. It is only useful for dev trunk use
sed "s|.*#include \"svnrevnum.h\"|//#include \"svnrevnum.h\"|g" -i bak HopsanCore/include/version.h

# Set the Core Gui and CLI svn rev numbers for this release
sed "s|#define HOPSANCORESVNREVISION.*|#define HOPSANCORESVNREVISION \"$coresvnrev\"|g" -i bak HopsanCore/include/version.h
sed "s|#define HOPSANGUISVNREVISION.*|#define HOPSANGUISVNREVISION \"$guisvnrev\"|g" -i bak HopsanGUI/version_gui.h
sed "s|#define HOPSANCLISVNREVISION.*|#define HOPSANCLISVNREVISION \"$clisvnrev\"|g" -i bak HopsanCLI/version_cli.h

if [ $doDevRelease = "false" ]; then
  # Set version numbers (by changing .h files) BEFORE build
  sed "s|#define HOPSANCOREVERSION.*|#define HOPSANCOREVERSION \"$version\"|g" -i bak HopsanCore/include/version.h
  sed "s|#define HOPSANGUIVERSION.*|#define HOPSANGUIVERSION \"$version\"|g" -i bak HopsanGUI/version_gui.h

  # Set splash screen version number
  sed "s|0\.0\.0|$version|g" -i bak HopsanGUI/graphics/splash.svg
  $Inkscape ./HopsanGUI/graphics/splash.svg --export-background=rgb\(255,255,255\) --export-dpi=90 --export-png ./HopsanGUI/graphics/splash.png
  
  # Make sure development flag is not defined
  sed "s|.*DEFINES \*= DEVELOPMENT|#DEFINES *= DEVELOPMENT|g" -i bak HopsanGUI/HopsanGUI.pro
fi

# Make sure we compile defaultLibrary into core
if [ "$doBuildInComponents" = "true" ]; then
  sed 's|.*DEFINES \*= HOPSAN_INTERNALDEFAULTCOMPONENTS|DEFINES *= HOPSAN_INTERNALDEFAULTCOMPONENTS|g' -i bak Common.prf
  sed 's|#INTERNALCOMPLIB.CC#|../componentLibraries/defaultLibrary/defaultComponentLibraryInternal.cc \\|g' -i bak HopsanCore/HopsanCore.pro
  sed '/.*<lib>.*/d' -i bak componentLibraries\defaultLibrary\defaultComponentLibrary.xml
  sed 's|componentLibraries||g' -i bak HopsanNG.pro
fi

# Build user documentation
./buildDocumentation.sh user
