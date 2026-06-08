#!/bin/bash

# Shell script building Hopsan dependency DCPLib

basedir=$(pwd)

name=dcplib
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}
xercesdir=${basedir}/xerces
libzipdir=${basedir}/libzip
asiodir=${basedir}/asio-code

# Download and verify
./download-dependencies.py ${name}

#Patch code to fix bug
patch --forward dcplib-code/include/core/dcp/model/pdu/IpToStr.hpp dcplib-patch.txt

# Include general settings
source setHopsanBuildPaths.sh

cmake -B"${builddir}" \
      -S"${codedir}" \
      -Wno-dev \
      -DASIO_ROOT="${asiodir}" \
      -DCMAKE_PREFIX_PATH="${xercesdir};${libzipdir}" \
      -DCMAKE_INSTALL_PREFIX="${installdir}"

# Build and install
cmake --build "${builddir}" --parallel 8
cmake --build "${builddir}" --target install

echo "setupDCPLib.sh done!"
