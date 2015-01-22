TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../bin

INCLUDEPATH += $${PWD}/../msgpack-c/include/
INCLUDEPATH += $${PWD}/../HopsanServer/

QMAKE_CXXFLAGS += -std=c++11

LIBS += -lzmq

SOURCES += main.cpp \
    RemoteHopsanClient.cpp

HEADERS += \
    RemoteHopsanClient.h \
    Packing.h

