#!/bin/bash
# $Id$

# Shell script for copying "Installing" the necessary files from a pre-build hopsan root dir
# The root dir is assumed to have been exported from svn
# Author: Peter Nordin peter.nordin@liu.se

E_BADARGS=65
if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir}"
  exit $E_BADARGS
fi

srcDir=${1%/}
dstDir=${2%/}
echo "Copy installing Hopsan from $srcDir to $dstDir"

# Copy whole directories
# ======================
mkdir -p                                                   $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/include                        $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/src                            $dstDir/HopsanCore
cp -a    $srcDir/HopsanCore/dependencies                   $dstDir/HopsanCore

mkdir -p                                                   $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/defaultLibrary         $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/exampleComponentLib    $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/extensionLibrary       $dstDir/componentLibraries
cp -a    $srcDir/componentLibraries/autoLibs               $dstDir/componentLibraries

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

# Copy dependencies files
# =======================
srcDeps=${srcDir}/Dependencies

mkdir -p                                                   $dstDir/Dependencies
cp -a    ${srcDeps}/katex                                  $dstDir/Dependencies
cp -a    ${srcDeps}/FMILibrary                             $dstDir/Dependencies

cp -a    ${srcDeps}/qwt/lib/libqwt.so*                     $dstDir/bin
cp -a    ${srcDeps}/zeromq/lib/libzmq.so*                  $dstDir/bin
cp -a    ${srcDeps}/FMILibrary/lib/libfmilib_shared.so     $dstDir/bin
cp -a    ${srcDeps}/discount/lib/libmarkdown.so*           $dstDir/bin
cp -a    ${srcDeps}/pythonqt/lib/libPythonQt*.so*          $dstDir/bin
cp -a    ${srcDeps}/hdf5/lib/libhdf5*-shared.so*           $dstDir/bin

# Install additional files
# =====================
install -m664 -t $dstDir                                   $srcDir/hopsandefaults
install -m664 -t $dstDir                                   $srcDir/Hopsan-release-notes.txt
install -m664 -t $dstDir                                   $srcDir/README.md

# Strip any runpaths to Dependencies directory
# from ELF binaries. Note! ($ORIGIN/./) will remain.
# By first moving the source dependencies directory
# the runpaths will no longer be valid and patchelf
# will remove them.
# ==================================================
if [[ $(command -v patchelf) ]]; then
  mv ${srcDeps} ${srcDeps}_temporary
  find ${dstDir}/bin -type f -executable -exec patchelf --shrink-rpath {} \;
  mv ${srcDeps}_temporary ${srcDeps}
fi
