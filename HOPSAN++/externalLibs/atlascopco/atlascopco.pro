QT -= core \
    gui
TARGET = atlascopcoLib
TEMPLATE = lib

CONFIG(debug, debug|release) {
#DESTDIR = ../../bin/debug
LIBS += -L../../bin/debug \
    -lHopsanCore
}
CONFIG(release, debug|release) {
#DESTDIR = ../../bin/debug
LIBS += -L../../bin/release \
    -lHopsanCore
}

INCLUDEPATH += ../../HopsanCore

HEADERS += \
    vsrc.hpp \
    sep2.hpp \
    fsrc.hpp \
    con2.hpp \
    bar.hpp \
    atlascopco.h

SOURCES += \
    atlascopco.cc
