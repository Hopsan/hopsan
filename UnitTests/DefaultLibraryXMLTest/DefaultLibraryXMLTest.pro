#-------------------------------------------------
#
# Project created by QtCreator 2013-10-11T09:51:13
#
#-------------------------------------------------

QT       += xml testlib
QT       -= gui

#Determine debug extension
include( ../../Common.prf )

TARGET = tst_defaultlibraryxmltest$${DEBUG_EXT}
DESTDIR = $${PWD}/../../bin
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Include and link to hopsan core
INCLUDEPATH += $${PWD}/../../HopsanCore/include/
LIBS += -L$${PWD}/../../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_defaultlibraryxmltest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
