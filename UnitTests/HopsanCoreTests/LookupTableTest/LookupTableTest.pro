#-------------------------------------------------
#
# Project created by QtCreator 2014-06-09T09:36:40
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../../Common.prf )

TARGET = tst_lookuptabletest$${DEBUG_EXT}
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

SOURCES += \
    tst_lookuptabletest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
