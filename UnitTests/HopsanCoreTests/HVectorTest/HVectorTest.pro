#-------------------------------------------------
#
# Project created by QtCreator 2013-10-16T14:48:58
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../../Common.prf )

TARGET = tst_hvectortest$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../../bin

TEMPLATE = app

INCLUDEPATH += $${PWD}/../../../HopsanCore/include/
LIBS += -L$${PWD}/../../../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += tst_hvectortest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
