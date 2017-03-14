#!/bin/bash
# $Id$

# Shell script to build zeromq dependency automatically
# Author: Peter Nordin peter.nordin@liu.se

basedir=`pwd`
codedir=${basedir}/zeromq
builddir=${codedir}_build
installdir=${codedir}_install

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
