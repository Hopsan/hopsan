#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Discount automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2015-02-24

filename="discount-2.1.8.zip"
dirname="discount-2.1.8"
basepwd=`pwd`

# include general settings
#source setHopsanBuildPaths.sh

# If arg 1 is --force then override regardless
if [ "$1" != "--force" ]; then
    # Abort if dir already exist. When running release build script we dont want to build twice
    if [ -d $qwtname ]; then
        echo "Directory dirname already exist. Remove it or give argument --force if you want (re)build using this script."
        exit 0
    fi
fi

# Clean old files
rm -rf $dirname
# Unzip
unzip -q $filename
# We do not need the patch, it is for windows

# Create Shadowbbuild directory
cd $dirname
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
    chmod u+x ./configure.sh
    ./configure.sh --shared
elif [ "$OSTYPE" == "darwin14" ]; then
    echo "Not implemented for MAC yet"
else
    echo "Unknown OS for Discount build"
fi
# Build
make -j4 -w
cd $basepwd
