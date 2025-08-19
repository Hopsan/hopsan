# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsandcp
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin
TARGET = $${TARGET}$${DEBUG_EXT}
DEFINES += LOGGING
#DEFINES += DEBUG
QMAKE_CXXFLAGS += -Wno-comment -Wno-switch -Wno-ignored-qualifiers -Wno-sign-compare

QT -= core gui

win32:LIBS *= -lws2_32

#--------------------------------------------------
# Add the include path to our self, (hopsandcp)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (HopsanCore)
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to xerces
INCLUDEPATH *= $${PWD}/../dependencies/asio-code/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include and lib path to libzip
include($${PWD}/../dependencies/libzip.pri)
#--------------------------------------------------

#--------------------------------------------------
# Add the include and lib path to xerces
include($${PWD}/../dependencies/xerces.pri)
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to DCPLib
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/bluetooth
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/core
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/ethernet
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/master
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/slave
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/xml
INCLUDEPATH *= $${PWD}/../dependencies/dcplib-code/include/zip
#--------------------------------------------------

#--------------------------------------------------------
# Set the tclap include path
INCLUDEPATH *= $${PWD}/../dependencies/tclap/include
#--------------------------------------------------------

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    DEFINES += HOPSANGENERATOR_DLLEXPORT
    DEFINES -= UNICODE
}
unix {
     # Add runtime search path so that dynamically loaded libraries in the same directory can be found.
    # Note! QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem hande $$ORIGIN, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES = \
        src/dcpmaster.cpp \
        src/dcpserver.cpp \
        src/utilities.cpp

HEADERS += \
    ../dependencies/dcplib-code/include/master/dcp/logic/DcpManagerMaster.hpp \
    ../dependencies/dcplib-code/include/slave/dcp/logic/DcpManagerSlave.hpp \
    include/dcpmaster.h \
    include/dcpserver.h \
    include/hopsandcp_win32dll.h \
    include/utilities.h

