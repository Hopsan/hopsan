#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Asio automatically

basedir=`pwd`
name=asio
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Copy code to build dir, not sure if out-of-source build is possible
mkdir -p $builddir
pushd $builddir
cp -a $codedir/* .

# Generate makefiles
chmod u+x ./configure
./configure --prefix=$installdir --without-boost
cd include
make install

popd
echo "setupAsio.sh done!"
