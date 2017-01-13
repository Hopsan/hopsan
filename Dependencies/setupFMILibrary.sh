#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

fminame="FMILibrary"
basepwd=`pwd`

source setHopsanBuildPaths.sh



# Create build dir
mkdir -p $fminame/build
cd $fminame/build
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
	cmake -Wno-dev ../
elif [ "$OSTYPE" == "darwin14" ]; then
        cmake -Wno-dev ../
else
        echo "Unknown OS for qwt build and patch"
fi
# Build and install (local dir install)
make -j4
make install test
cd $basepwd

