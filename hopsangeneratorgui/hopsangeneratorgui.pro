#-------------------------------------------------
#
# Project created by QtCreator 2018-04-03T08:53:00
#
#-------------------------------------------------
include( ../Common.prf )

TARGET = hopsangeneratorgui
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = $${PWD}/../bin
TARGET = $${TARGET}$${DEBUG_EXT}

QT     += core gui
isEqual(QT_MAJOR_VERSION, 5){
    QT += widgets
}

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++14
}

INCLUDEPATH += include/hopsangeneratorgui

SOURCES += src/hopsangeneratorgui.cpp

HEADERS += include/hopsangeneratorgui/hopsangeneratorgui.h
