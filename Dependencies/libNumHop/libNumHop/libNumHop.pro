#-------------------------------------------------
#
# Project created by QtCreator 2015-11-20T10:09:58
#
#-------------------------------------------------

CONFIG(debug, debug|release) {
    DEBUG_EXT = _d
} else {
    DEBUG_EXT =
}

QT       -= core gui

TARGET = NumHop$${DEBUG_EXT}
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = $${PWD}/

SOURCES += \
    VariableStorage.cc \
    Helpfunctions.cc \
    Expression.cc

HEADERS += \
    VariableStorage.h \
    Helpfunctions.h \
    Expression.h \
    numhop.h
