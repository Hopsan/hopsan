#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtname="qwt"
basepwd=`pwd`

# include general settings
source setHopsanBuildPaths.sh

# Adjust CRLF on Mac OS X
if [ "$OSTYPE" == "darwin14" ]; then
    find $qwtname -name '*.pr?' -exec dos2unix {} \;
fi
#Patch
#patch --binary -p0 < $qwtpatch.patch
cd $qwtname
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
        $hopsan_qt_qmake qwt.pro -r -spec linux-g++
elif [ "$OSTYPE" == "darwin14" ]; then
        $hopsan_qt_qmake qwt.pro -r # This is a rather temporary ugly solution...
else
        echo "Unknown OS for qwt build and patch"
fi
# Build
make -j4 -w
cd $basepwd
