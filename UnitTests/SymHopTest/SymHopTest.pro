#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

TARGET = tst_symhoptest
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../bin


TEMPLATE = app

#Determine debug extension
CONFIG(debug, debug|release) {
  DEBUG_EXT = _d
} else {
  DEBUG_EXT =
}

INCLUDEPATH += $${PWD}/../../SymHop/include/
LIBS += -L$${PWD}/../../bin -lSymHop$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_symhoptest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
