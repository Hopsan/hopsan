#!/bin/bash
# $Id$

# Shell script building HopsaGUI dependency PythonQt automatically
# Author: Peter Nordin peter.nordin@liu.se

set -e

pyqtversion="3.0"
pyversion="2.7"
basedir=$(pwd)

if [ $# -lt 1 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {release, debug} [pyversion] [pyqtversion]"
  echo "       pyversion is optional, (2.7 default)"
  echo "       pyqtversion is optional, (3.0 default)"
  exit $E_BADARGS
fi

if [ $# -gt 1 ]; then
  pyversion="$2"
fi

if [ $# -gt 2 ]; then
  pyqtversion="$3"
fi

pythonqtname="PythonQt${pyqtversion}"
pythonqtfile=releases/${pythonqtname}.zip
name=pythonqt
codedir=${basedir}/${name}_code
builddir=${basedir}/${name}_build
installdir=${basedir}/${name}
E_BADARGS=65

# Include general settings
source setHopsanBuildPaths.sh

if [ -d ${codedir} ]; then
    echo "$codedir Already exists, not replacing files!"
else
    if [ -f ${pythonqtfile} ]; then
        unzip -q ${pythonqtfile}
        mv ${pythonqtname} ${codedir}
    else
        echo "Warning: ${pythonqtfile} is missing, not building PythonQt"
        exit 0
    fi
fi

cd ${codedir}

# Remove extensions tests and examples to speedup build
sed "s|extensions tests examples||" -i PythonQt.pro

# Set build mode
if [ "$1" != "release" ]; then
  sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
fi

# Set python version
sed "s|unix:PYTHON_VERSION=.*|unix:PYTHON_VERSION=${pyversion}|" -i build/python.prf

# Build in build dir
mkdir -p $builddir
cd $builddir

${HOPSAN_BUILD_QT_QMAKE} ${codedir}/PythonQt.pro -r -spec linux-g++
make -j$(getconf _NPROCESSORS_ONLN) -w

# Install manually since PythonQt code does not have install target configured
mkdir -p ${installdir}/include
cp -a lib ${installdir}
cd ${codedir}/src
find -name "*.h" -exec cp -a --parents {} ${installdir}/include \;

cd $basedir
echo "setupPythonQt.sh done!"
