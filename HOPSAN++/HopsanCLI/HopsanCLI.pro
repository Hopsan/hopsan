#-------------------------------------------------
#
# Project created by QtCreator 2011-03-28T13:31:46
#
#-------------------------------------------------

TARGET = HopsanCLI
TEMPLATE = app
DESTDIR = $${PWD}/../bin

QT       -= core gui

TARGET = $${TARGET}$${DEBUG_EXT}

CONFIG   += console
CONFIG   -= app_bundle

INCLUDEPATH *= $${PWD}/../HopsanCore

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += main.cpp
