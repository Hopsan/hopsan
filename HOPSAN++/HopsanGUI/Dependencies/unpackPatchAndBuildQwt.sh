#!/bin/sh
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtname="qwt-6.0.1"
basepwd=`pwd`

rm -rf $qwtname
rm -rf $qwtname\_shb
unzip $qwtname.zip
mkdir $qwtname\_shb #Shadowbbuild directory
cd $qwtname\_shb
qmake ../$qwtname/qwt.pro -r -spec linux-g++
make -w
cd $basepwd

