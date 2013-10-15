#-------------------------------------------------
#
# Project created by QtCreator 2013-10-11T09:51:13
#
#-------------------------------------------------

QT       += xml testlib
QT       -= gui


TARGET = tst_defaultlibraryxmltest
DESTDIR = $${PWD}/../../bin
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#Determine debug extension
CONFIG(debug, debug|release) {
  DEBUG_EXT = _d
} else {
  DEBUG_EXT =
}

# Include and link to hopsan core
INCLUDEPATH += $${PWD}/../../HopsanCore/include/
LIBS += -L$${PWD}/../../bin -lHopsanCore$${DEBUG_EXT}

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}


SOURCES += \
    tst_defaultlibraryxmltest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
