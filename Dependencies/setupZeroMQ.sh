#!/bin/bash
# $Id$

# Shell script to build zeromq dependency automatically
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
name=zeromq
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

# Make and enter build dir
mkdir -p $builddir
cd $builddir

zmq_cmake_args="-Wno-dev -DBUILD_STATIC=OFF -DZMQ_HAVE_TIPC=OFF -DCMAKE_INSTALL_LIBDIR=lib"
if [[ ${OSTYPE} == darwin* ]]; then
    zmq_cmake_args="${zmq_cmake_args} -DZMQ_BUILD_FRAMEWORK=OFF"
fi

# Configure
cmake ${zmq_cmake_args} -DCMAKE_INSTALL_PREFIX=${installdir} ${codedir}

# Build and install
make -j$(getconf _NPROCESSORS_ONLN)
make install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  make test
fi

cd $basedir

# Now "install" cppzmq (header only), to the zmq install dir
# TODO in the future use cmake to install cppzmq, but that does not work right now since no "zeromq-config.cmake" file seem to be created.
# For now lets just copy the file
codedir=${basedir}/cppzmq
installdir=${installdir}/include/

cp -a ${codedir}/zmq.hpp $installdir

cd $basedir
echo "setupZeroMQ.sh done!"
