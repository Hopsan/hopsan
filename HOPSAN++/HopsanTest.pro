# -------------------------------------------------
# Project created by QtCreator 2010-02-16T15:57:03
# -------------------------------------------------
QT -= core gui
TARGET = hopsantest
TEMPLATE = app
CONFIG += 
INCLUDEPATH += HopsanCore Utilities
DESTDIR = ./bin/debug
# win32:DEFINES += STATICCORE
#LIBS += -L./HopsanCore/bin/debug -lHopsanCore -Wl,-rpath,HopsanCore/bin/debug
LIBS += -L./bin/debug -lHopsanCore
unix:LIBS += -lrt -ldl

SOURCES += hopsan.cc \
    Utilities/TicToc.cc
HEADERS += Utilities/TicToc.h \
    HopsanCore/HopsanCore.h

