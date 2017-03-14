#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

basedir=`pwd`
codedir=${basedir}/FMILibrary
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
