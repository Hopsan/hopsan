#-------------------------------------------------
#
# Project created by QtCreator 2018-04-03T08:53:00
#
#-------------------------------------------------
include( ../Common.prf )

TARGET = hopsangeneratorgui
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = $${PWD}/../lib
TARGET = $${TARGET}$${DEBUG_EXT}

QT     += core gui
isEqual(QT_MAJOR_VERSION, 5){
    QT += widgets
}

INCLUDEPATH += include

SOURCES += src/hopsangeneratorgui.cpp

HEADERS += include/hopsangeneratorgui/hopsangeneratorgui.h
