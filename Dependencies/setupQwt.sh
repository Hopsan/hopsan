#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se


basedir=`pwd`
name=qwt
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

# include general settings
source setHopsanBuildPaths.sh

# Adjust CRLF on Mac OS X
if [ "$OSTYPE" == "darwin14" ]; then
    find $codedir -name '*.pr?' -exec dos2unix {} \;
fi

# Patch code
cd $codedir
patch --binary --forward -p1 < ../qwt-build.patch

mkdir -p $builddir
cd $builddir
# Generate makefiles on different platforms
if [ "$OSTYPE" == "linux-gnu" ]; then
        $hopsan_qt_qmake ${codedir}/qwt.pro -r -spec linux-g++
elif [ "$OSTYPE" == "darwin14" ]; then
        $hopsan_qt_qmake ${codedir}/qwt.pro -r # This is a rather temporary ugly solution...
else
        echo "Unknown OS for qwt build and patch"
fi

# Build
make -j4 -w

# Install
make install

cd $basedir
echo "setupQwt.sh done!"
