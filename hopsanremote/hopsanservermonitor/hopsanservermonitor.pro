include( $${PWD}/../../Common.prf )

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hopsanservermonitor
DESTDIR = $${PWD}/../../bin

#--------------------------------------------------------
# Depend on the remoteclient lib
INCLUDEPATH += $${PWD}/../libhopsanremoteclient/include
LIBS += -L$${PWD}/../../lib -lhopsanremoteclient
#--------------------------------------------------------

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

#--------------------------------------------------------
# Set the tclap include path
INCLUDEPATH *= $${PWD}/../../dependencies/tclap/include
#--------------------------------------------------------

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
LIBS += -pthread

win32 {
    DEFINES -= UNICODE
}

unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

SOURCES += main.cpp \

HEADERS +=



