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

# Configure
cmake -Wno-dev -DCMAKE_INSTALL_PREFIX=$installdir $codedir 
#./autogen.sh
#./configure --without-libsodium         # Configure

# Build and install
make -j4
make install
make test

cd $basedir

# Now "install" cppzmq (header only), to the zmq install dir
# TODO in the future use cmake to install cppzmq, but that does not work right now since no "zeromq-config.cmake" file seem to be created.
# For now lets just copy the file
codedir=${basedir}/cppzmq
installdir=${installdir}/include/

cp -a ${codedir}/zmq.hpp $installdir

cd $basedir
echo "setupZeroMQ.sh done!"
