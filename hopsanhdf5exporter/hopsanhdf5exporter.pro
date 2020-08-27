#-------------------------------------------------
#
# Project created by QtCreator 2020-08-27T10:38:46
#
#-------------------------------------------------
include( ../Common.prf )

QT       -= gui
TEMPLATE = lib
CONFIG += staticlib

TARGET = hopsanhdf5exporter
DESTDIR = $${PWD}/../lib

INCLUDEPATH += $${PWD}/include

TARGET = $${TARGET}$${DEBUG_EXT}

DEFINES += HOPSANHDF5EXPORTER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#--------------------------------------------------------
# Set hopsan core paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------------

# Set hdf5 paths
include($${PWD}/../dependencies/hdf5.pri)
have_hdf5(){
  DEFINES *= USEHDF5
  !build_pass:message("Compiling with HDF5 support")
} else {
  !build_pass:message("Compiling without HDF5 support")
}


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        hopsanhdf5exporter.cpp

HEADERS += \
        hopsanhdf5exporter.h
