#-------------------------------------------------
#
# Project created by QtCreator 2018-06-28T21:51:57
#
#-------------------------------------------------
include( $${PWD}/../../Common.prf )

QT       -= core gui
TEMPLATE = lib
CONFIG += staticlib

TARGET = hopsanremotecommon
DESTDIR = $${PWD}/../../lib

INCLUDEPATH += $${PWD}/include

SOURCES += \
    src/FileAccess.cpp


HEADERS += \
    include/hopsanremotecommon/DataStructs.h \
    include/hopsanremotecommon/FileAccess.h \
    include/hopsanremotecommon/FileReceiver.hpp \
    include/hopsanremotecommon/Messages.h \
    include/hopsanremotecommon/MessageUtilities.h \
    include/hopsanremotecommon/StatusInfoStructs.h
