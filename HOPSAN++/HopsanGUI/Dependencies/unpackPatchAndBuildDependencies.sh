#!/bin/sh
# $Id: buildDocumentation.sh 4057 2012-02-15 14:25:20Z petno25 $

# Shell script building HopsaGUI dependencies automatically
# Author: Peter Nordin peter.nordin@liu.se
# Date:   2012-03-29

qwtname="qwt-6.0.1"
pythonqtname="PythonQt2.0.1"

basepwd=`pwd`

doRelease="false"
if [ "$1" = "release" ]; then
  doRelease="true"    
  echo Building release versions only
fi

buildQWT()
{
  rm -rf $qwtname
  rm -rf $qwtname\_shb
  unzip $qwtname.zip
  mkdir $qwtname\_shb #Shadowbbuild directory
  cd $qwtname\_shb
  qmake ../$qwtname/qwt.pro -r -spec linux-g++
  make -w
  cd $basepwd
}

buildPythonQt()
{
  rm -rf $pythonqtname
  unzip $pythonqtname.zip
  cd $pythonqtname
  
  echo "Applying Hopsan fixes to code"
  # Fix cocoa thing
  sed "s|CocoaRequestModal = QEvent::CocoaRequestModal,|/\*CocoaRequestModal = QEvent::CocoaRequestModal,\*/|" -i generated_cpp/com_trolltech_qt_core/com_trolltech_qt_core0.h

  # Set build mode
  if [ "$1" = "false" ]; then
    sed "s|#CONFIG += debug_and_release build_all|CONFIG += debug_and_release build_all|" -i build/common.prf
  fi

  # Set python version
  sed "s|unix:PYTHON_VERSION=2.6|unix:PYTHON_VERSION=$2|" -i build/python.prf
  
  qmake PythonQt.pro -r -spec linux-g++
  make -w
  cd $basepwd
}


# First build QWT
buildQWT
echo "Sleeping 5 seconds to let you view output from QWT build"; sleep 5
# TODO should ask for pyversion or use commandline
buildPythonQt $doRelease 2.7
echo "Sleeping 5 seconds to let you view output from PythonQt build" sleep 5
