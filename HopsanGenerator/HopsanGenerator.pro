# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsangenerator
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin
TARGET = $${TARGET}$${DEBUG_EXT}

QT -= gui
QT += core xml

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++14
}

#--------------------------------------------------------
# Set the FMILibrary include and library paths
include($${PWD}/../dependencies/fmilibrary.pri)
# On Windows, since RPATH is ignored by LoadLibrary(), copy the fmi library file to the bin directory after build,
# so that libHopsanGenerator may find it when loaded at runtime
# (this is only necessary for dev builds, on release all DLLs will be copied anyway)
# TODO: It would be better if this could be handled by the fmilibrary.pri somehow
win32 {
  src_file = $$quote($${PWD}/../dependencies/fmilibrary/lib/libfmilib_shared.dll)
  dst_dir = $$quote($${DESTDIR})
  # Replace slashes in paths with backslashes for Windows
  src_file ~= s,/,\\,g
  dst_dir ~= s,/,\\,g
  QMAKE_POST_LINK *= $$QMAKE_COPY $${src_file} $${dst_dir}
}

#--------------------------------------------------
# Add the include path to our self, (HopsanGenerator)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (HopsanCore)
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (SymHop)
INCLUDEPATH *= $${PWD}/../SymHop/include/
LIBS *= -L$${PWD}/../bin -lsymhop$${DEBUG_EXT}
DEFINES *= SYMHOP_DLLIMPORT
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {

}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    DEFINES += HOPSANGENERATOR_DLLEXPORT
    DEFINES -= UNICODE
}
unix {
    # Add runtime search path so that dynamically loaded libraries in the same directory can be found.
    # Note! QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem hande $$ORIGIN, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    src/HopsanGeneratorLib.cpp \
    src/GeneratorUtilities.cpp \
    src/generators/HopsanSimulinkGenerator.cpp \
    src/generators/HopsanModelicaGenerator.cpp \
    src/generators/HopsanFMIGenerator.cpp \
    src/generators/HopsanLabViewGenerator.cpp \
    src/GeneratorTypes.cpp \
    src/generators/HopsanGeneratorBase.cpp \
    src/generators/HopsanExeGenerator.cpp

HEADERS += \
    include/hopsangenerator_win32dll.h \
    include/GeneratorUtilities.h \
    include/generators/HopsanModelicaGenerator.h \
    include/generators/HopsanSimulinkGenerator.h \
    include/generators/HopsanFMIGenerator.h \
    include/generators/HopsanLabViewGenerator.h \
    include/GeneratorTypes.h \
    include/generators/HopsanGeneratorBase.h \
    include/hopsangenerator.h \
    include/generators/HopsanExeGenerator.h

RESOURCES += \
    templates.qrc
