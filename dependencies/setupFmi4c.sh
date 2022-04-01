#!/bin/bash

# Shell script building Hopsan dependency fmi4c

basedir=$(pwd)

name=fmi4c
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
cmake -Wno-dev -G %HOPSAN_BUILD_CMAKE_GENERATOR% -DCMAKE_INSTALL_PREFIX=${installdir} ${codedir}

# Build and install
cmake --build . --parallel 8
cmake --build . --target install


cp -R ${codedir}/include ${installdir}/
cp -R ${codedir}/3rdparty/fmi ${installdir}/3rdparty/

# Return to basedir
cd $basedir
echo "setupFMILibrary.sh done!"
