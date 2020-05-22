#!/bin/bash

# Shell script building Hopsan dependency FMILibrary

basedir=$(pwd)

name=fmilibrary
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Patch
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
cmake --build . --parallel $(getconf _NPROCESSORS_ONLN)
cmake --build . --target install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  ctest --parallel $(getconf _NPROCESSORS_ONLN)
fi

# Return to basedir
cd $basedir
echo "setupFMILibrary.sh done!"
