include( ../../Common.prf )

# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = defaultcomponentlibrary
TARGET = $${TARGET}$${DEBUG_EXT}

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# The location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component .hpp files
# You can also add additional paths for eg. your own Utility functions, just add additional INCLUDEPATH *= ... lines.
# *= Means append unique
INCLUDEPATH *= $${PWD}/code/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/

# The location of the HopsanCore .dll or .so file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin

# In debug mode HopsanCore has the debug extension _d
LIBS *= -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

# Reduce compile output clutter, but show warnings
CONFIG += silent warn_on plugin

# The compiler should be pedantic to catch all errors (optional)
QMAKE_CXXFLAGS += -pedantic -Wno-long-long -Wconversion

# Enable C++14
CONFIG += c++14

# Enable the use of M_PI and such
DEFINES *= _USE_MATH_DEFINES

# -------------------------------------------------
# Project files
# -------------------------------------------------
HEADERS += \

SOURCES += \
    defaultComponentLibraryInternal.cpp \
    defaultComponentLibrary.cpp

OTHER_FILES += \

include($${PWD}/../../dependencies/fmi4c.pri)
include(Components.pri)
