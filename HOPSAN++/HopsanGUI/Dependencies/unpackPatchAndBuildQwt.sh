#!/bin/sh
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtname="qwt-6.0.1"
basepwd=`pwd`

# Abort if dir already exist. When running release build script we dont want to build twice
if [ -d $qwtname ]; then
  echo "Directory $qwtname already exist. Remove it if you want (re)build using this script."
  exit 0
fi

rm -rf $qwtname
rm -rf $qwtname\_shb
unzip -q $qwtname.zip
mkdir $qwtname\_shb #Shadowbbuild directory
cd $qwtname\_shb
qmake ../$qwtname/qwt.pro -r -spec linux-g++
make -w
cd $basepwd

