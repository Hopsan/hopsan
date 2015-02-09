#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtzipfile="qwt-6.1.2.zip"
qwtname="qwt-6.1.2"
basepwd=`pwd`

# If arg 1 is --force then override regardless
if [ "$1" != "--force" ]; then
    # Abort if dir already exist. When running release build script we dont want to build twice
    if [ -d $qwtname ]; then
        echo "Directory $qwtname already exist. Remove it or give argument --force if you want (re)build using this script."
        exit 0
    fi
fi

# Clean old files
rm -rf $qwtname
rm -rf $qwtname\_shb
# Unzip
unzip -q $qwtzipfile
#Patch
patch --binary -p0 < $qwtname.patch
# Create Shadowbbuild directory
mkdir $qwtname\_shb
cd $qwtname\_shb
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
        qmake ../$qwtname/qwt.pro -r -spec linux-g++
elif [ "$OSTYPE" == "darwin13" ]; then
        $HOME/Qt/5.2.1/clang_64/bin/qmake ../$qwtname/qwt.pro -r # This is a rather temporary ugly solution...
else
        echo "Unknown OS for qwt build and patch"
fi
# Build
make -j4 -w
cd $basepwd
