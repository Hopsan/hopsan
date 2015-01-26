# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( $${PWD}/../../Common.prf )
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

#INCLUDEPATH += $${PWD}/../../ThirdParty/msgpack-0.5.9/src/
#INCLUDEPATH += $${PWD}/../../ThirdParty/zeromq-4.1.0/include/
#INCLUDEPATH += $${PWD}/../../ThirdParty/cppzeromq-master/
#QMAKE_CXXFLAGS += -std=c++11

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
LIBS += -pthread

unix {
    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
}

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += main.cpp

HEADERS += \
    global.h \
    Messages.h \
    MessageUtilities.h

