#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically

basedir=$(pwd)
name=qwt
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Include general settings
source setHopsanBuildPaths.sh

# Adjust CRLF on Mac OS X
#if [[ $OSTYPE == darwin* ]]; then
#    find $codedir -name '*.pr?' -exec dos2unix {} \;
#fi

# Patch code
cd $codedir
patch --binary --forward -p0 < ../qwt-build.patch

mkdir -p $builddir
cd $builddir
# Generate makefiles on different platforms
qwt_qmake_spec=linux-g++
if [[ $OSTYPE == darwin* ]]; then
    qwt_qmake_spec=macx-clang
fi
${HOPSAN_BUILD_QT_QMAKE} ${codedir}/qwt.pro -r -spec ${qwt_qmake_spec}

# Build
make -j$(getconf _NPROCESSORS_ONLN) -w

# Install
make install

cd $basedir
echo "setupQwt.sh done!"
