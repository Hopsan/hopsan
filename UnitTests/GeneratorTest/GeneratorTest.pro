#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../Common.prf )

TARGET = tst_generatortest$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../bin

TEMPLATE = app

INCLUDEPATH += $${PWD}/../../HopsanCore/include
INCLUDEPATH *= $${PWD}/../../HopsanGenerator/include
LIBS += -L$${PWD}/../../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
LIBS *= -lhopsangenerator$${DEBUG_EXT}
LIBS *= -lsymhop$${DEBUG_EXT}
include($${PWD}/../../dependencies/fmilibrary.pri)
DEFINES *= HOPSANGENERATOR_DLLIMPORT



unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_generatortest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
