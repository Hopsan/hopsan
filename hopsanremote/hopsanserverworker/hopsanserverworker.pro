# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( $${PWD}/../../Common.prf )

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../../bin
TARGET = hopsanserverworker

#--------------------------------------------------------
# Depend on the remote common lib
INCLUDEPATH += $${PWD}/../libhopsanremotecommon/include
LIBS += -L$${PWD}/../../lib -lhopsanremotecommon
#--------------------------------------------------------

#--------------------------------------------------------
# Set the ZeroMQ paths
include($${PWD}/../../dependencies/zeromq.pri)
!have_zeromq() {
  !build_pass:error("Failed to locate ZeroMQ libs, have you compiled them in the expected location?")
}
include($${PWD}/../../dependencies/msgpack.pri)
!have_msgpack() {
  !build_pass:error("Failed to locate msgpack-c library")
}
#--------------------------------------------------------

# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../../Utilities/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/
LIBS *= -L$${PWD}/../../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
LIBS += -pthread

unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be relative to the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to handle the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}



# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += main.cpp
HEADERS +=
