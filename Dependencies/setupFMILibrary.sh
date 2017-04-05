#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
basedir=`pwd`
name=FMILibrary
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

codedir=${basedir}/
builddir=${codedir}_build
installdir=${codedir}_install

source setHopsanBuildPaths.sh

# Create build dir and enter it
mkdir -p $builddir
cd $builddir

# Generate makefiles
cmake -DFMILIB_INSTALL_PREFIX=$installdir -Wno-dev $codedir

# Build and install 
make -j4
make install test

# Return to basedir
cd $basedir
echo "setupFMILibrary.sh done!"
