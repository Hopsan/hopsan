#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../../Common.prf )

TARGET = tst_utilitiestesttest$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../../bin

TEMPLATE = app

INCLUDEPATH += $${PWD}/../../../HopsanCore/include/
LIBS += -L$${PWD}/../../../bin -lhopsancore$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    tst_utilitiestesttest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
