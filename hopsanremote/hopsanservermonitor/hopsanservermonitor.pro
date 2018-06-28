include( $${PWD}/../../Common.prf )

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = hopsanservermonitor
DESTDIR = $${PWD}/../../bin

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}

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
# Set the tclap include path
INCLUDEPATH *= $${PWD}/../../Dependencies/tclap/include
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



