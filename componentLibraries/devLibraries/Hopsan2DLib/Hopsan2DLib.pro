# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = Hopsan2DLib

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# The location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component .hpp files
# See myLocalPathTemplate.prf, copy this file (do not change it, change the copy to suit your needs)
!exists(myLocalPaths.prf){
    error("myLocalPaths.prf does not exist. You need to COPY the template file. DO NOT commit myLocalPaths.prf")
}
include(myLocalPaths.prf)

# Special options for debug and release mode. This will link the correct HopsanCore .dll or .so
# In debug mode HopsanCore has the debug extension _d
include(hopsanDebugReleaseCompile.pri)

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    Hopsan2DLib.cc

HEADERS += \
    Hopsan2DBody2.hpp \
    Hopsan2DBodyTest.hpp \
    Hopsan2DJoint.hpp \
    Hopsan2DFixedAttachment.hpp \
    Hopsan2DForceTorqueSource.hpp

OTHER_FILES += \
    hopsanDebugReleaseCompile.prf \
    Hopsan2DLib.xml \
    Hopsan2DBody2.xml \
    Hopsan2DBodyTest.xml \
    Hopsan2DJoint.xml \
    Hopsan2DFixedAttachment.xml \
    Hopsan2DForceTorqueSource.xml
