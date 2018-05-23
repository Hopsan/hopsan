#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se


basedir=$(pwd)
name=qwt
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

# Include general settings
source setHopsanBuildPaths.sh

# Adjust CRLF on Mac OS X
#if [[ $OSTYPE == darwin* ]]; then
#    find $codedir -name '*.pr?' -exec dos2unix {} \;
#fi

# Patch code
cd $codedir
patch --binary --forward -p1 < ../qwt-build.patch

mkdir -p $builddir
cd $builddir
# Generate makefiles on different platforms
qwt_qmake_spec=linux-g++
if [[ $OSTYPE == darwin* ]]; then
    qwt_qmake_spec=macx-clang
fi
${hopsan_qt_qmake} ${codedir}/qwt.pro -r -spec ${qwt_qmake_spec}

# Build
make -j4 -w

# Install
make install

cd $basedir
echo "setupQwt.sh done!"
