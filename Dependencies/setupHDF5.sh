#!/bin/bash
# Shell script building HopsaGUI dependency HDF5 automatically

basedir=$(pwd)
name=hdf5
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}

# Include general settings
source setHopsanBuildPaths.sh

mkdir -p $builddir
cd $builddir
cmake -GNinja -Wno-dev -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=$installdir $codedir

ninja 
ninja install

cd $basedir
echo "setupHDF5.sh done!"
