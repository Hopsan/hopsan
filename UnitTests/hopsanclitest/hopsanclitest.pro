QT       += testlib
QT       -= gui

#Determine debug extension
include( ../../Common.prf )

TARGET = tst_hopsancli$${DEBUG_EXT}
CONFIG   += console
CONFIG   -= app_bundle
DESTDIR = $${PWD}/../../bin


TEMPLATE = app

INCLUDEPATH += $${PWD}/../../HopsanCore/include/
INCLUDEPATH += $${PWD}/../../HopsanCLI/
LIBS += -L$${PWD}/../../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

unix{
QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

}

SOURCES += \
    tst_hopsancli.cpp \
    $${PWD}/../../HopsanCLI/ModelUtilities.cpp \
    $${PWD}/../../HopsanCLI/CliUtilities.cpp
