# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = symhop
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT += core
QT -= gui

message(QT=$$QT_MAJOR_VERSION)

TARGET = $${TARGET}$${DEBUG_EXT}

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++14
}

#--------------------------------------------------
# Add the include path to our self, (SymHop)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {

}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    DEFINES += SYMHOP_DLLEXPORT
    DEFINES -= UNICODE
}

# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += src/SymHop.cpp

HEADERS += include/SymHop.h \
    include/symhop_win32dll.h
