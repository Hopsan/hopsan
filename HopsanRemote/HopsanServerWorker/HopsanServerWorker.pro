# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( $${PWD}/../HopsanRemoteBuild.pri )

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../../bin

#--------------------------------------------------------
# Set the ZMQ paths and dll/so post linking copy command
d = $$setZMQPathInfo($$(ZMQ_PATH), $$DESTDIR)
isEmpty(d):warning("ERROR: Failed to locate ZeroMQ libs, have you compiled them and put them in the expected location?")
LIBS *= $$magic_hopsan_libpath
INCLUDEPATH *= $$magic_hopsan_includepath
QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
QMAKE_CXXFLAGS *= -std=c++11
#--------------------------------------------------------

INCLUDEPATH *= $${PWD}/../include

# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../../Utilities/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/
LIBS *= -L$${PWD}/../../bin -lHopsanCore$${DEBUG_EXT}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
LIBS += -pthread

unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be relative to the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to handle the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}



# -------------------------------------------------
# Project files
# -------------------------------------------------

SOURCES += main.cpp \
    FileAccess.cpp

HEADERS += \
    FileAccess.h

