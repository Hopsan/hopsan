QT -= core \
    gui
TARGET = atlascopcoLib
TEMPLATE = lib

CONFIG(debug, debug|release) {
    #DESTDIR = ../bin/debug
    LIBS += -L../../bin/debug -lHopsanCore
}
CONFIG(release, debug|release) {
    DESTDIR = ./
    LIBS += -L../../bin/release -lHopsanCore
}

INCLUDEPATH += ../../HopsanCore

HEADERS += \
    vsrc.hpp \
    sep2.hpp \
    fsrc.hpp \
    con2.hpp \
    bar.hpp \
    ubar.hpp \
    reflfree.hpp \
    dturb.hpp \
    con3.hpp \
    aturb.hpp

SOURCES += \
    atlascopco.cc
