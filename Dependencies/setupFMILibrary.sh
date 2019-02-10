#!/bin/bash
# $Id$

# Shell script building Hopsan dependency FMILibrary
# Author: Peter Nordin peter.nordin@liu.se

basedir=$(pwd)

name=fmilibrary
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}


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
make -j$(getconf _NPROCESSORS_ONLN)
make install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  make test
fi

# Return to basedir
cd $basedir
echo "setupFMILibrary.sh done!"
