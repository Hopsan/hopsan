#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

TARGET = ../../../../bin/tst_symhoptest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#Determine debug extension
CONFIG(debug, debug|release) {
  DEBUG_EXT = _d
} else {
  DEBUG_EXT =
}

INCLUDEPATH += $${PWD}/../../../HopsanGenerator/include/
LIBS += -L$${PWD}/../../../bin -lHopsanGenerator$${DEBUG_EXT}
LIBS += -L$${PWD}/../../../bin -lHopsanCore$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_symhoptest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
