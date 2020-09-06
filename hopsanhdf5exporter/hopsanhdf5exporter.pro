#-------------------------------------------------
#
# Project created by QtCreator 2020-08-27T10:38:46
#
#-------------------------------------------------
include( ../Common.prf )

QT       -= core gui
TEMPLATE = lib
CONFIG += staticlib

TARGET = hopsanhdf5exporter
DESTDIR = $${PWD}/../lib

INCLUDEPATH += $${PWD}/include

TARGET = $${TARGET}$${DEBUG_EXT}

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
  SOURCES += \
        hopsanhdf5exporter.cpp

  HEADERS += \
        hopsanhdf5exporter.h

  !build_pass:message("Compiling hopsanhdf5exporter with HDF5 support")
} else {
  # I have found no obvious way to make a subdir project do nothing, build fails if there is nothing here, at least on MACOS
  SOURCES += \
        hopsanhdf5exporter_dummy.cpp
  !build_pass:warning("Compiling hopsanhdf5exporter without HDF5 support (a stub library will be created)")
}
