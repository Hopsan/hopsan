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

rm -rf $pythonqtname
unzip $pythonqtname.zip
cd $pythonqtname
  
echo "Applying Hopsan fixes to code"
# Fix cocoa thing
sed "s|CocoaRequestModal = QEvent::CocoaRequestModal,|/\*CocoaRequestModal = QEvent::CocoaRequestModal,\*/|" -i generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h

# Set build mode
if [ "$1" != "release" ]; then
  sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
fi

# Set python version
sed "s|unix:PYTHON_VERSION=2.6|unix:PYTHON_VERSION=$pyversion|" -i build/python.prf

qmake PythonQt.pro -r -spec linux-g++
make -w
cd $basepwd

