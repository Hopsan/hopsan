# -------------------------------------------------
# Global project options
# -------------------------------------------------
QT -= core \
    gui

DESTDIR = ../lib
TARGET = HopsanCore
TEMPLATE = lib
CONFIG += shared

include( ../Common.prf )
TARGET = $${TARGET}$${DEBUG_EXT}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE
    DEFINES += DOCOREDLLEXPORT
    DEFINES -= UNICODE

    #Set default tbb path alternatives, higher up is prefered
    TBB_PATHS *= ../ExternalDependencies/tbb30_20101215oss
    TBB_PATHS *= ../ExternalDependencies/tbb30_20100406oss
    #Try environment variable first $$(ENVVARNAME)if it exists, then default paths listed above
    TBB_PATH = $$selectPath($$(TBB_PATH), $$TBB_PATHS, "tbb")

    INCLUDEPATH += $${TBB_PATH}/include/tbb

    CONFIG(debug, debug|release) {
        LIBS += -L$${TBB_PATH}/build/windows_ia32_gcc_mingw_debug
        LIBS += -ltbb_debug
    }
    CONFIG(release, debug|release) {
        LIBS += -L$${TBB_PATH}/build/windows_ia32_gcc_mingw_release
        LIBS += -ltbb
    }

    #Debug output
    #message(Includepath is $$INCLUDEPATH)
    #message(Libs is $${LIBS})
}
unix { 
    LIBS += -ltbb
    INCLUDEPATH += /usr/include/tbb/
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
include(HopsanCore.pri)
