#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../Common.prf )

TARGET = tst_symhoptest$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../bin


TEMPLATE = app

INCLUDEPATH += $${PWD}/../../SymHop/include/
LIBS += -L$${PWD}/../../bin -lsymhop$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_symhoptest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
