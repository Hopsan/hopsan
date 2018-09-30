#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Qwt automatically
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
zipname=FMILibrary-2.0.2
zipfile=releases/${zipname}-src.zip

name=FMILibrary
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}


if [ -d $codedir ]; then
    echo "$codedir Already exists, not replacing files!"
else
    if [ -f ${zipfile} ]; then
        unzip -q ${zipfile}
        mv ${zipname} ${codedir}
    else
        echo "Warning: ${zipfile} is missing, you need to download it"
        exit 0
    fi
fi

pushd ${codedir}
patch -p0 --forward < ../fmilibrary-c99.patch
popd

set -e

source setHopsanBuildPaths.sh

# Create build dir and enter it
mkdir -p $builddir
cd $builddir

# Generate makefiles
cmake -DFMILIB_INSTALL_PREFIX=${installdir} -Wno-dev ${codedir}

# Build and install
make -j4
make install test

# Return to basedir
cd $basedir
echo "setupFMILibrary.sh done!"
