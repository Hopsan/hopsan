#!/bin/bash
# $Id$

# Shell script for copying "Installing" the necessary files from a prebuild hopsan root dir
# The root dir is assumed to have been exported from svn
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-04

srcDir=$1
dstDir=$2

echo Copy installing Hopsan from $srcDir to $dstDir

if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir}"
  exit $E_BADARGS
fi

# Create needed dst directories
mkdir -p $dstDir/HopsanCore
mkdir -p $dstDir/componentLibraries/defaultLibrary
mkdir -p $dstDir/Models
mkdir -p $dstDir/doc/user
mkdir -p $dstDir/bin


# Copy whole directories
cp -a $srcDir/HopsanCore/include $dstDir/HopsanCore/include
cp -a $srcDir/componentLibraries/defaultLibrary/components $dstDir/componentLibraries/defaultLibrary/components
cp -a $srcDir/componentLibraries/exampleComponentLib $dstDir/componentLibraries/exampleComponentLib
cp -a $srcDir/Models/Example\ Models $dstDir/Models/Example\ Models
cp -a $srcDir/doc/graphics $dstDir/doc/graphics
cp -a $srcDir/Scripts $dstDir/Scripts 
cp -a $srcDir/doc/user/html $dstDir/doc/user/

# copy files
cp -a $srcDir/bin/*.so* $dstDir/bin/
cp -a $srcDir/bin/Hopsan* $dstDir/bin/
cp -a $srcDir/componentLibraries/defaultLibrary/components/*.so* $dstDir/componentLibraries/defaultLibrary/components/
cp -a $srcDir/hopsandefaults $dstDir/
cp -a $srcDir/Hopsan-release-notes.txt $dstDir/

