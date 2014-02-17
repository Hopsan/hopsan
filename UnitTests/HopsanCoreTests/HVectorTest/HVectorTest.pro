#-------------------------------------------------
#
# Project created by QtCreator 2013-10-16T14:48:58
#
#-------------------------------------------------

QT       += testlib
QT       -= gui

TARGET = tst_hvectortest
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../../bin

TEMPLATE = app

#Determine debug extension
CONFIG(debug, debug|release) {
  DEBUG_EXT = _d
} else {
  DEBUG_EXT =
}

INCLUDEPATH += $${PWD}/../../../HopsanCore/include/
LIBS += -L$${PWD}/../../../bin -lHopsanCore$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += tst_hvectortest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
