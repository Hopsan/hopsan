#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency PythonQt automatically
# Author: Peter Nordin peter.nordin@liu.se

#set -u
set -e

pyversion="2.7"
basedir=`pwd`
pythonqtname="PythonQt3.1"
pythonqtfile=releases/${pythonqtname}.zip
name=pythonqt
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}
E_BADARGS=65

if [ $# -lt 1 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {release, debug} [pyversion]"
  echo "       pyversion is optional, (2.7 default)"
  exit $E_BADARGS
fi

if [ "$2" != "" ]; then
  pyversion="$2"  
fi

#ubuntuversion=$(echo `lsb_release -rs` | sed 's|\.||')

# Abort if dir already exist. When running release build script we dont want to build twice
#if [ -d $pythonqtname ]; then
#  echo "Directory $pythonqtname already exist. Remove it if you want to (re)build using this script."
#  exit 0
#fi

if [ -d $codedir ]; then
    echo "$codedir Already exists, not replacing files!"
else
    if [ -f ${pythonqtfile} ]; then
        unzip -q ${pythonqtfile}
        mv $pythonqtname $codedir
    else
	echo "Warning: ${pythonqtfile} is missing, you need to download it"
	exit 0
    fi
fi

cd $codedir
echo "Applying Hopsan related fixes to code"

# Apply patch to remove some qt extensions that are not needed
#if [ "$1" = "release" ]; then
#  patch -p1 < ../$pythonqtname\_reducebuild.patch
#fi

# Remove extensions tests and examples to speedup build
sed "s|extensions tests examples||" -i PythonQt.pro

# Set build mode
if [ "$1" != "release" ]; then
  sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
fi

# Set python version
#sed "s|unix:PYTHON_VERSION=2.6|unix:PYTHON_VERSION=$pyversion|" -i build/python.prf

# Build in build dir
mkdir -p $builddir
cd $builddir

qmake ${codedir}/PythonQt.pro -r -spec linux-g++
make -j4 -w

# Install manually since PythonQt code does not have install target configured
mkdir -p $installdir/include
cp -a lib $installdir
cd $codedir/src
find -name "*.h" -exec cp -a --parents {} $installdir/include \;

cd $basedir
echo "setupPythonQt.sh done!"
