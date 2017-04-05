#!/bin/bash
# $Id$

# Shell script for copying "Installing" the necessary files from a pre-build hopsan root dir
# The root dir is assumed to have been exported from svn
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-04

E_BADARGS=65
if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir}"
  exit $E_BADARGS
fi

srcDir=$1
dstDir=$2
echo "Copy installing Hopsan from $srcDir to $dstDir"

# Copy whole directories
# ======================
mkdir -p                                                   $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/include                        $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/src                            $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/dependencies                   $dstDir/HopsanCore

mkdir -p                                                   $dstDir/Dependencies
cp -a    $srcDir/Dependencies/katex                        $dstDir/Dependencies

cp -a    $srcDir/Dependencies/FMILibrary     $dstDir/Dependencies

mkdir -p                                                   $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/defaultLibrary         $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/exampleComponentLib    $dstDir/componentLibraries

mkdir -p                                                   $dstDir/Models
cp -a    $srcDir/Models/Example\ Models                    $dstDir/Models
cp -a    $srcDir/Models/Component\ Test                    $dstDir/Models

mkdir -p                                                   $dstDir/doc
cp -a    $srcDir/doc/html                                  $dstDir/doc
cp -a    $srcDir/doc/graphics                              $dstDir/doc

cp -a    $srcDir/Scripts                                   $dstDir

# Copy compiled libs and exec files
# =================================
cp -a    $srcDir/bin                                       $dstDir

# Copy additional files
# =====================
cp -a    $srcDir/hopsandefaults                            $dstDir
cp -a    $srcDir/Hopsan-release-notes.txt                  $dstDir
