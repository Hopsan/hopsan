# -------------------------------------------------
# Global project options
# -------------------------------------------------
TARGET = HopsanGenerator
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

#QT -= gui

TARGET = $${TARGET}$${DEBUG_EXT}


#--------------------------------------------------
# Add the include path to our self, (HopsanCore)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCompGen options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  DEFINES *= DEBUGCOMPILING
  QMAKE_CXXFLAGS += -pedantic
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
    src/HopsanComponentGenerator.cc \
    src/SymHop.cc \
    src/HopsanGeneratorLib.cc
HEADERS += \
    include/HopsanComponentGenerator.h \
    include/SymHop.h \
    include/win32dll.h
OTHER_FILES += \
    HopsanCoreBuild.prf








