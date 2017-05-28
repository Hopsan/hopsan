# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( $${PWD}/../../Common.prf )

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../../bin

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

INCLUDEPATH *= $${PWD}/../include

# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../../Utilities/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/
LIBS *= -L$${PWD}/../../bin -lHopsanCore$${DEBUG_EXT}

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

SOURCES += main.cpp \
    ../include/FileAccess.cpp

HEADERS += \
    ../include/FileAccess.h \
    ../include/FileReceiver.hpp
