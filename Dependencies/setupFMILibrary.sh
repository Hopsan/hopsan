#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

fminame="FMILibrary-2.0.1"
fmizipfile="$fminame-src.zip"

basepwd=`pwd`

source setHopsanBuildPaths.sh

# If arg 1 is --force then override regardless
if [ "$1" != "--force" ]; then
    # Abort if dir already exist. When running release build script we dont want to build twice
    if [ -d $fminame ]; then
        echo "Directory $fminame already exist. Remove it if you want (re)build using this script, or give --force argument."
        exit 0
    fi
fi

# Clean old files
rm -rf $fminame
# Unzip
unzip -q $fmizipfile
# Create build dir
mkdir $fminame/build-fmil
cd $fminame/build-fmil
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]
then
	cmake ../
elif [ "$OSTYPE" == "darwin14" ]
then
        cmake ../
else
        echo "Unknown OS for qwt build and patch"
fi
# Build and install (local dir install)
make -j4
make install test
cd $basepwd

