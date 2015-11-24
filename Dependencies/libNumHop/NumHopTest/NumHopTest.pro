CONFIG(debug, debug|release) {
    DEBUG_EXT = _d
} else {
    DEBUG_EXT =
}

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = numhopTest$${DEBUG_EXT}
DESTDIR = $${PWD}/
INCLUDEPATH += $${PWD}/../libNumHop
LIBS += -L$${PWD}/../libNumHop -lNumHop$${DEBUG_EXT}

SOURCES += main.cpp

