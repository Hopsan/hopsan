# -------------------------------------------------
# Global project options
# -------------------------------------------------
TEMPLATE = lib
CONFIG += staticlib
QT       -= core gui

CONFIG += thread

TARGET = hopsanremoteclient
DESTDIR = $${PWD}/../../lib

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}

#--------------------------------------------------------
# Set the ZeroMQ paths
include($${PWD}/../../Dependencies/zeromq.pri)
!have_zeromq() {
  !build_pass:error("Failed to locate ZeroMQ libs, have you compiled them in the expected location?")
}
include($${PWD}/../../Dependencies/msgpack.pri)
!have_msgpack() {
  !build_pass:error("Failed to locate msgpack-c library")
}
#--------------------------------------------------------

#--------------------------------------------------------
# Depend on the common remote lib
INCLUDEPATH += $${PWD}/../libhopsanremotecommon/include
LIBS += -L$${PWD}/../../lib -lhopsanremotecommon
#--------------------------------------------------------

INCLUDEPATH += $${PWD}/include

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += src/RemoteHopsanClient.cpp

HEADERS += include/hopsanremoteclient/RemoteHopsanClient.h
