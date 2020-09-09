#!/bin/bash
# Shell script building HopsaGUI dependency HDF5 automatically

basedir=$(pwd)
name=hdf5
codedir=${basedir}/${name}-code
builddir=${basedir}/${name}-build
installdir=${basedir}/${name}

# Download and verify
./download-dependencies.py ${name}

# Include general settings
source setHopsanBuildPaths.sh

# Handle code sub dir
cd $codedir
cd hdf5-*
codedir=$(pwd)

mkdir -p $builddir
cd $builddir
cmake -Wno-dev -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DBUILD_TESTING=OFF -DHDF5_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=$installdir $codedir
cmake --build .
cmake --build . --target install

cd $basedir
echo "setupHDF5.sh done!"
