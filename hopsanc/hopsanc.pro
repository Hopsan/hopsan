# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsanc
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin
TARGET = $${TARGET}$${DEBUG_EXT}

QT -= core gui

#--------------------------------------------------
# Add the include path to our self, (HopsanC)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (HopsanCore)
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------

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
HEADERS = \
    include/hopsanc.h

SOURCES = \
   src/hopsanc.cpp
