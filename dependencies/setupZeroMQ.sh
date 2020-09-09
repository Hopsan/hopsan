#!/bin/bash

# Shell script to build zeromq dependency automatically

basedir=`pwd`
name=zeromq
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Include general settings
source setHopsanBuildPaths.sh

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
cmake --build .
cmake --build . --target install
if [[ "$HOPSAN_BUILD_DEPENDENCIES_TEST" == "true" ]]; then
  ctest
fi

cd $basedir

# Now "install" cppzmq (header only), to the zmq install dir
# TODO in the future use cmake to install cppzmq, but that does not work right now since no "zeromq-config.cmake" file seem to be created.
# For now lets just copy the file

name=cppzmq
codedir=${basedir}/${name}-code
installdir=${installdir}/include/

./download-dependencies.py ${name}

cp -a ${codedir}/*.hpp ${installdir}

cd ${basedir}
echo "setupZeroMQ.sh done!"
