#-------------------------------------------------
#
# Project created by QtCreator 2011-03-28T13:31:46
#
#-------------------------------------------------
# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = HopsanCLI
TEMPLATE = app
DESTDIR = $${PWD}/../bin

QT       -= core gui

TARGET = $${TARGET}$${DEBUG_EXT}

CONFIG   += console
CONFIG   -= app_bundle

#--------------------------------------------------------
# Set the tclap and rapidxml include path
INCLUDEPATH *= $${PWD}/../Dependencies/tclap-1.2.1/include
INCLUDEPATH *= $${PWD}/../Dependencies/rapidxml-1.13
#--------------------------------------------------------

#--------------------------------------------------------
# Set the TicToc include path
INCLUDEPATH *= $${PWD}/../Utilities
#--------------------------------------------------------

#--------------------------------------------------------
# Set hopsan core paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include
LIBS *= -L$${PWD}/../bin -lHopsanCore$${DEBUG_EXT}
#--------------------------------------------------------

CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long -Wconversion
}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
unix {
    # This will add runtime so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

    # Is the following really needed on Linux? /magse
    !macx:LIBS *= -lrt

    # Get the svn revision in here if script succeed, we dont care about the external file generated,
    system($${PWD}/../getSvnRevision.sh) {
        DEFINES *= "HOPSANCLISVNREVISION=$$system($${PWD}/../getSvnRevision.sh)"
    }
}
win32 {

    # Activate large adress aware, to access more the 2GB virtual RAM (for 32-bit version)
    !contains(QMAKE_HOST.arch, x86_64){
        QMAKE_LFLAGS += -Wl,--large-address-aware
    }

    # Get the svn revision in here if script succeed, we dont care about the external file generated,
    system($${PWD}/../getSvnRevision.bat) {
        DEFINES *= "HOPSANCLISVNREVISION=$$system($${PWD}/../getSvnRevision.bat)"
    }
}

#Debug output
#message(CLI Includepath is $${INCLUDEPATH})
#message(CLI Libs2 is $${LIBS})

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += main.cpp \
    CliUtilities.cpp \
    ModelValidation.cpp \
    core_cli.cpp \
    ModelUtilities.cpp \
    BuildUtilities.cpp

HEADERS += \
    version_cli.h \
    CliUtilities.h \
    ModelValidation.h \
    core_cli.h \
    ModelUtilities.h \
    BuildUtilities.h
