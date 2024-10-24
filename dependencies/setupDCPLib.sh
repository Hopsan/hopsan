#!/bin/bash

# Shell script building Hopsan dependency DCPLib

basedir=$(pwd)

name=dcplib
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}
xercesdir=${basedir}/xerces

# Download and verify
./download-dependencies.py ${name}

#Patch code to fix bug
patch dcplib-code/include/core/dcp/model/pdu/IpToStr.hpp dcplib-patch.txt

# Include general settings
source setHopsanBuildPaths.sh

# Create build dir and enter it
mkdir -p $builddir
cd $builddir

# Generate makefiles
cmake -Wno-dev -DLOGGING=ON -DASIO_ROOT="${basedir}/asio-code" -DXercesC_LIBRARY="${xercesdir}/bin/libxerces-c" -DXercesC_INCLUDE_DIR="${xercesdir}/include" -DXercesC_VERSION="3.2.2" -DZIP_LIBRARY="${basedir}/libzip/bin/libzip" -DZIP_INCLUDE_DIR="${basedir}/libzip/include" -DCMAKE_INSTALL_PREFIX="${installdir}" "${codedir}"

# Build and install
cmake --build . --parallel 8
cmake --build . --target install

# Return to basedir
cd $basedir
echo "setupDCPLib.sh done!"
