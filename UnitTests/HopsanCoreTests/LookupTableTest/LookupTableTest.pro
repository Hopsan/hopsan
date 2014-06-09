#-------------------------------------------------
#
# Project created by QtCreator 2014-06-09T09:36:40
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../../Common.prf )

TARGET = tst_LookupTableTest$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../../bin

TEMPLATE = app

INCLUDEPATH += $${PWD}/../../../HopsanCore/include/
LIBS += -L$${PWD}/../../../bin -lHopsanCore$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += tst_LookupTableTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
