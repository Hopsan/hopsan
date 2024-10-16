#!/bin/bash

# Shell script building Hopsan dependency Libzip

basedir=$(pwd)

name=libzip
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
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=${installdir} ${codedir}

# Build and install
cmake --build . --parallel 8
cmake --build . --target install

# Return to basedir
cd $basedir
echo "setupLibzip.sh done!"
