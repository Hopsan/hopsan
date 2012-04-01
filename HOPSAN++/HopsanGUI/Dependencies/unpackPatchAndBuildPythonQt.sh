#!/bin/sh
# $Id$

# Shell script building HopsaGUI dependency PythonQt automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

pythonqtname="PythonQt2.0.1"
pyversion="2.7"
basepwd=`pwd`

if [ "$2" != "" ]; then
  pyversion="$2"  
fi

# Abort if dir already exist. When running release build script we dont want to build twice
if [ -d $pythonqtname ]; then
  echo Directory $pythonqtname already exist. Remove it if you want build using this script.
  exit 0
fi

rm -rf $pythonqtname
unzip -q $pythonqtname.zip
cd $pythonqtname
  
echo "Applying Hopsan fixes to code"
# Fix cocoa thing
sed "s|CocoaRequestModal = QEvent::CocoaRequestModal,|/\*CocoaRequestModal = QEvent::CocoaRequestModal,\*/|" -i generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h

# Set build mode
if [ "$1" != "release" ]; then
  sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
else
  #Remove tests and examples in release build
  sed "s|tests examples||" -i PythonQt.pro
fi

# Set python version
sed "s|unix:PYTHON_VERSION=2.6|unix:PYTHON_VERSION=$pyversion|" -i build/python.prf

qmake PythonQt.pro -r -spec linux-g++
make -w
cd $basepwd

