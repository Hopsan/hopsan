#-------------------------------------------------
#
# Project created by QtCreator 2010-08-25T11:38:12
#
#-------------------------------------------------

QT       -= core gui

TARGET = myLib
TEMPLATE = lib

#DESTDIR = ../bin/debug
LIBS += -L../../bin/debug \
    -lHopsanCore
INCLUDEPATH += ../HopsanCore

DEFINES += MYLIB_LIBRARY

SOURCES += myLib.cc

HEADERS += myLib.h \
    myWickedOrifice.hpp
