# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = SymHop
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT += core
QT -= gui

TARGET = $${TARGET}$${DEBUG_EXT}


#--------------------------------------------------
# Add the include path to our self, (SymHop)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  DEFINES *= DEBUGCOMPILING
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {
  DEFINES *= RELEASECOMPILING
}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    DEFINES += SYMHOPDLLEXPORT
    DEFINES -= UNICODE
}

# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += src/SymHop.cc

HEADERS += include/SymHop.h \
    include/win32dll.h









