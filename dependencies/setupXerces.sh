#!/bin/bash

# Shell script building Hopsan dependency Xerces

basedir=$(pwd)

name=xerces
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Include general settings
source setHopsanBuildPaths.sh

# Create build dir and enter it
mkdir -p $builddir
cd $builddir

# Generate makefiles
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=$installdir ${codedir}

# Build and install
cmake --build .
cmake --build . --target install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  ctest
fi

# Return to basedir
cd $basedir
echo "setupXerces.sh done!"
