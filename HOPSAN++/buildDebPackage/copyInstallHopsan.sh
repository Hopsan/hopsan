#!/bin/bash
# $Id$

# Shell script for copying "Installing" the necessary files from a prebuild hopsan root dir
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-04
# For use in Hopsan, requires "subversion commandline" installed (apt-get install subversion)

srcDir=$1
dstDir=$2

echo $srcDir
echo $dstDir

if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir}"
  exit $E_BADARGS
fi

# Create needed dst directories
mkdir -p $dstDir/HopsanCore
mkdir -p $dstDir/componentLibraries
mkdir -p $dstDir/Models
#mkdir -p $dstDir/Scripts
mkdir -p $dstDir/doc/user
mkdir -p $dstDir/bin


# Do svn export of svn directories
svn export $srcDir/HopsanCore/include $dstDir/HopsanCore/include
svn export $srcDir/componentLibraries/exampleComponentLib $dstDir/componentLibraries/exampleComponentLib
svn export $srcDir/Models/Example\ Models $dstDir/Models/Example\ Models
svn export $srcDir/doc/graphics $dstDir/doc/graphics
svn export $srcDir/Scripts $dstDir/Scripts 


# Copy files not under version control (the build binary files and documentation)
cp -a $srcDir/doc/user/html $dstDir/doc/user/
cp -a $srcDir/bin/*.so* $dstDir/bin/
cp -a $srcDir/bin/Hopsan* $dstDir/bin/
