#-------------------------------------------------
#
# Project created by QtCreator 2018-06-28T21:51:57
#
#-------------------------------------------------

QT       -= core gui
TEMPLATE = lib
CONFIG += staticlib

TARGET = hopsanremotecommon
DESTDIR = $${PWD}/../../lib

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}


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
