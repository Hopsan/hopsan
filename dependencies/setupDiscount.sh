#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency Discount automatically

basedir=`pwd`
name=discount
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Copy code to build dir, not sure if out-of-source build is possible
mkdir -p $builddir
cd $builddir
cp -a $codedir/* .

# Generate makefiles
chmod u+x ./configure.sh
./configure.sh --shared --prefix=$installdir --confdir=$installdir/etc
# Fix non-root ldconfig cache file location (permission) problem
sed -e 's/ldconfig "$1"/ldconfig -C .\/ld.so.cache "$1"/' -i librarian.sh

# Build
make -j$(getconf _NPROCESSORS_ONLN) -w

# Install
make install

cd $basepwd
echo "setupDiscount.sh done!"
