# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( $${PWD}/../../Common.prf )

TEMPLATE = app
CONFIG += console
CONFIG += thread
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hopsanremoteclient
DESTDIR = $${PWD}/../../bin

#--------------------------------------------------------
# Set the tclap include path
INCLUDEPATH *= $${PWD}/../../dependencies/tclap/include
#--------------------------------------------------------

#--------------------------------------------------------
# Depend on the remoteclient lib
INCLUDEPATH += $${PWD}/../libhopsanremoteclient/include
LIBS += -L$${PWD}/../../lib -lhopsanremoteclient
#--------------------------------------------------------

#--------------------------------------------------------
# Depend on the common remote lib
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
SOURCES += main.cpp


HEADERS +=

