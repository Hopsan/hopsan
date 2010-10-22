QT -= core \
    gui
TARGET = atlascopcoLib
TEMPLATE = lib

#DESTDIR = ../../bin/debug
LIBS += -L../../bin/debug \
    -lHopsanCore
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
