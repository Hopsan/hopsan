# -------------------------------------------------
# Global project options
# -------------------------------------------------
QT -= core gui
TARGET = hopsantest
DESTDIR = ./bin/debug
TEMPLATE = app

#Compiler flags
INCLUDEPATH += HopsanCore Utilities
#DEFINES += STATICCORE

#Linker flags
LIBS += -L./bin/debug -lHopsanCore

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    CONFIG += console
    DEFINES -= UNICODE
}

unix {
    LIBS += -lrt -ldl -Wl,-rpath,./
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += hopsan.cc \
    Utilities/TicToc.cc
HEADERS += Utilities/TicToc.h \
    HopsanCore/HopsanCore.h

