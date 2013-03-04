# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = HopsanGenerator
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT += xml core gui

TARGET = $${TARGET}$${DEBUG_EXT}


#--------------------------------------------------
# Add the include path to our self, (HopsanGenerator)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to (HopsanCore)
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lHopsanCore$${DEBUG_EXT}
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  DEFINES *= DEBUGCOMPILING
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {
  DEFINES *= RELEASECOMPILING
}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the generator into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the generator as a DLL or SO
    DEFINES -= UNICODE
}
unix { 
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    src/HopsanGeneratorLib.cc \
    src/GeneratorUtilities.cc \
    src/symhop/SymHop.cc \
    src/generators/HopsanGenerator.cc \
    src/generators/HopsanSimulinkGenerator.cc \
    src/generators/HopsanModelicaGenerator.cc \
    src/generators/HopsanFMIGenerator.cc \
    src/generators/HopsanLabViewGenerator.cc

HEADERS += \
    include/win32dll.h \
    include/GeneratorUtilities.h \
    include/symhop/SymHop.h \
    include/generators/HopsanGenerator.h \
    include/generators/HopsanModelicaGenerator.h \
    include/generators/HopsanSimulinkGenerator.h \
    include/generators/HopsanFMIGenerator.h \
    include/generators/HopsanLabViewGenerator.h

OTHER_FILES += \
    HopsanCoreBuild.prf

RESOURCES += \
    templates.qrc








