# -------------------------------------------------
# Global project options
# -------------------------------------------------
TEMPLATE = app
CONFIG += console
CONFIG += thread
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hopsanserverclient
DESTDIR = $${PWD}/../../bin

#--------------------------------------------------------
# Set the tclap include path
INCLUDEPATH *= $${PWD}/../../Dependencies/tclap/include
#--------------------------------------------------------

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
QMAKE_CXXFLAGS *= -std=c++11
#--------------------------------------------------------



INCLUDEPATH += $${PWD}/../include

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
SOURCES += main.cpp \
    ../include/FileAccess.cpp \
    RemoteHopsanClient.cpp


HEADERS += \
    RemoteHopsanClient.h \
    ../include/FileReceiver.hpp \
    ../include/FileAccess.h

