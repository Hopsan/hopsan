# -------------------------------------------------
# Project created by QtCreator 2011-04-01T11:38:12
# -------------------------------------------------

# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# The name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = exampleComponentLib

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# The location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component .hpp files
# You can also add additional paths for eg. your own Utility functions
INCLUDEPATH *= $${PWD}/../../HopsanCore

# The location of the HopsanCore .dll or .so file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin

# Special options for deug and release mode
# In debug mode HopsanCore has the debug extension _d
CONFIG(debug, debug|release) {
    LIBS *= -lHopsanCore_d
    DEFINES *= DEBUGCOMPILING
}
CONFIG(release, debug|release) {
    LIBS *= -lHopsanCore
    DEFINES *= RELEASECOMPILING
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    exampleComponentLib.cc

HEADERS += \
    component_code/MyExampleVolume.hpp \
    component_code/MyExampleOrifice.hpp

OTHER_FILES +=
