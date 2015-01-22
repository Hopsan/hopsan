TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../bin

INCLUDEPATH += $${PWD}/../msgpack-c/include/
INCLUDEPATH += $${PWD}/../HopsanServer/

QMAKE_CXXFLAGS += -std=c++11

#Determine debug extension, and debug output on/off
CONFIG(debug, debug|release) {
    !macx:DEBUG_EXT = _d
    macx:DEBUG_EXT =
    DEFINES *= DEBUGCOMPILING
} else {
    DEBUG_EXT =
    DEFINES *= QT_NO_DEBUG_OUTPUT
    DEFINES *= RELEASECOMPILING
}
#Add a defined value with the debug extension (that can be used in the code)
DEFINES *= DEBUG_EXT=$${DEBUG_EXT}

#Determine the dynamic link library file extension, typically .dll for Windows, .so for UNIX and .dylib for OSX
win32:DLL_EXT = .dll
unix:DLL_EXT = .so
macx:DLL_EXT = .dylib

#Add a defined value with the dll extension (that can be used in the code)
DEFINES *= DLL_EXT=$${DLL_EXT}

#Define the lib prefix for the different platforms, (not you can NOT change the prefix here)
unix:LIB_PREFIX = lib
win32:LIB_PREFIX =
DEFINES *= DLL_PREFIX=$${LIB_PREFIX}

# Set HopsanCore Paths
INCLUDEPATH *= /home/petno25/svn/hopsan/trunk/Utilities/
INCLUDEPATH *= /home/petno25/svn/hopsan/trunk/HopsanCore/include/
LIBS *= -L/home/petno25/svn/hopsan/trunk/bin -lHopsanCore$${DEBUG_EXT}

LIBS += -pthread -lzmq

unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

SOURCES += main.cpp

