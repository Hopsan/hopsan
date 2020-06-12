#!/bin/bash
# $Id$

# Shell script to build Sundials dependency automatically
# Author: Robert Braun robert.braun@liu.se

basedir=`pwd`
name=sundials
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Make and enter build dir
mkdir -p $builddir
cd $builddir

sundials_cmake_args="-Wno-dev -DBUILD_STATIC_LIBS=OFF -DBUILD_ARKODE=OFF -DBUILD_IDAS=OFF -DBUILD_IDA=OFF -DBUILD_CVODE=OFF -DBUILD_CVODES=OFF -DBUILD_KINSOL=ON -DBUILD_EXAMPLES_C=OFF -DEXAMPLES_INSTALL=OFF -DCMAKE_INSTALL_LIBDIR=lib"

# Configure
cmake ${sundials_cmake_args} -DCMAKE_INSTALL_PREFIX=${installdir} ${codedir}

# Build and install
make -j$(getconf _NPROCESSORS_ONLN)
make install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  make test
fi

# Return to basedir
cd $basedir
echo "setupSundials.sh done!"
