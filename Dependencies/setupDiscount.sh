#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Discount automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2015-02-24

dirname="discount"
basepwd=`pwd`

# include general settings
#source setHopsanBuildPaths.sh

cd $dirname
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
    chmod u+x ./configure.sh
    ./configure.sh --shared
elif [ "$OSTYPE" == "darwin14" ]; then
    chmod u+x ./configure.sh
    ./configure.sh --shared
else
    echo "Unknown OS for Discount build"
fi
# Build
make -j4 -w

cd $basepwd
