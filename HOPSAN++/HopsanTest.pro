# -------------------------------------------------
# Project created by QtCreator 2010-02-16T15:57:03
# -------------------------------------------------
QT -= core gui
TARGET = hopsantest
TEMPLATE = app
CONFIG += 
INCLUDEPATH += HopsanCore Utilities
# win32:DEFINES += STATICCORE
LIBS += HopsanCore/bin/debug/libHopsanCore.so -Wl,-rpath,HopsanCore/bin/debug
unix:LIBS += -lrt -ldl

SOURCES += hopsan.cc \
    Utilities/TicToc.cc
HEADERS += Utilities/TicToc.h \
    HopsanCore/HopsanCore.h

