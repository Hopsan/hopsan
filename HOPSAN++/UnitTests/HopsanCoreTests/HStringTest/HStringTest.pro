#-------------------------------------------------
#
# Project created by QtCreator 2013-06-04T08:23:49
#
#-------------------------------------------------
QT       += testlib
QT       -= gui

TARGET = $${PWD}/../../../bin/tst_hstringtest
CONFIG   += console
CONFIG   -= app_bundle

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


SOURCES += \
    tst_hstringtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
