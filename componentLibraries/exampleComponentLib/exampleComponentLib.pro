# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = examplecomponentlib

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# The location to search for the Hopsan include files. By specifying the path here, you dont need to
# do this in each of your component .hpp files. You can also add additional paths for any custom Utility functions,
# just add additional INCLUDEPATH *= ... lines.
# *= Means append unique
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/
#INCLUDEPATH *= "C:/SomeDirectoryPath/Hopsan/HopsanCore/include/"

# The location of the HopsanCore .dll or .so file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin
#LIBS *= -L"C:/SomeDirectoryPath/Hopsan/bin/"

# Special options for deug and release mode. This will link the correct HopsanCore .dll or .so
# In debug mode HopsanCore has the debug extension _d
include(hopsanDebugReleaseCompile.pri)

# Reduce compile output clutter, but show warnings
CONFIG += silent warn_on

# The compiler should be pedantic to catch all errors (optional)
QMAKE_CXXFLAGS += -pedantic

# Enable C++14
CONFIG += c++14

# Enable the use of M_PI and such
DEFINES *= _USE_MATH_DEFINES

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    exampleComponentLib.cpp

HEADERS += \
    HydraulicComponents/MyExampleVolume.hpp \
    HydraulicComponents/MyExampleOrifice.hpp \
    SignalComponents/MyExampleSignalSum.hpp \
    HydraulicComponents/MyExampleVolume2.hpp \
    HydraulicComponents/MyExampleConstantOrifice.hpp

OTHER_FILES += \
    hopsanDebugReleaseCompile.pri
