#!/bin/sh
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtzipfile="qwt-6.1.0.zip"
qwtname="qwt-6.1.0"
basepwd=`pwd`

# Abort if dir already exist. When running release build script we dont want to build twice
if [ -d $qwtname ]; then
  echo "Directory $qwtname already exist. Remove it if you want (re)build using this script."
  exit 0
fi

# Clean old files
rm -rf $qwtname
rm -rf $qwtname\_shb
# Unzip
unzip -q $qwtzipfile
# Create Shadowbbuild directory
mkdir $qwtname\_shb
cd $qwtname\_shb
# Generate makefiles
qmake ../$qwtname/qwt.pro -r -spec linux-g++
# Build
make -w
cd $basepwd
