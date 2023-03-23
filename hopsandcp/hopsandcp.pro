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

LIBS *= -lws2_32

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
# Add the include and lib path to xerces
INCLUDEPATH *= $${PWD}/../dependencies/xerces/include/
LIBS *= -L$${PWD}/../dependencies/xerces/lib/
LIBS *= -lxerces-c
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to xerces
INCLUDEPATH *= $${PWD}/../dependencies/asio/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include and lib path to libzip
INCLUDEPATH *= $${PWD}/../dependencies/libzip/include/
LIBS *= -L$${PWD}/../dependencies/libzip/lib/
LIBS *= -lzip
#--------------------------------------------------

#--------------------------------------------------
# Add the include and lib path to xerces
INCLUDEPATH *= $${PWD}/../dependencies/xerces/include/
LIBS *= -L$${PWD}/../dependencies/xerces/lib/
LIBS *= -lxerces-c
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to DCPLib
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/bluetooth
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/core
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/ethernet
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/master
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/slave
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/xml
INCLUDEPATH *= $${PWD}/dependencies/DCPLib/include/zip
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
    include/dcpmaster.h \
    include/dcpserver.h \
    include/hopsandcp_win32dll.h \
    include/utilities.h

