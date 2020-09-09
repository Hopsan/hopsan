#-------------------------------------------------
#
# Project created by QtCreator 2011-03-28T13:31:46
#
#-------------------------------------------------
# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsancli
TEMPLATE = app
DESTDIR = $${PWD}/../bin

QT       -= core gui

TARGET = $${TARGET}$${DEBUG_EXT}

CONFIG   += console
CONFIG   -= app_bundle

# Allow non-strict ansi code
QMAKE_CXXFLAGS *= -U__STRICT_ANSI__ -Wno-c++0x-compat

#--------------------------------------------------------
# Set the tclap and rapidxml include path
INCLUDEPATH *= $${PWD}/../dependencies/tclap/include
INCLUDEPATH *= $${PWD}/../HopsanCore/dependencies/rapidxml
#--------------------------------------------------------

#--------------------------------------------------------
# Set the TicToc include path
INCLUDEPATH *= $${PWD}/../Utilities
#--------------------------------------------------------

#--------------------------------------------------------
# Set hdf5exporter and hdf5 paths
LIBS += -L$${PWD}/../lib -lhopsanhdf5exporter$${DEBUG_EXT}
include($${PWD}/../dependencies/hdf5.pri)
have_hdf5(){
  INCLUDEPATH *= $${PWD}/../hopsanhdf5exporter
  DEFINES *= USEHDF5
  !build_pass:message("Compiling HopsanCLI with HDF5 support")
} else {
  LIBS -= -lhopsanhdf5exporter$${DEBUG_EXT}
  !build_pass:message("Compiling HopsanCLI without HDF5 support")
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set hopsan core paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set OPS paths
DEFINES *= USEOPS
contains(DEFINES, USEOPS) {
  INCLUDEPATH *= $${PWD}/../Ops/include
  LIBS *= -L$${PWD}/../bin -lops$${DEBUG_EXT}
  DEFINES *= OPS_DLLIMPORT
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set HopsanGenerator Paths
DEFINES *= HOPSANCLI_USEGENERATOR
contains(DEFINES, HOPSANCLI_USEGENERATOR) {
  message(Compiling HopsanCLI with HopsanGenerator)
  INCLUDEPATH *= $${PWD}/../HopsanGenerator/include
  LIBS *= -L$${PWD}/../bin -lhopsangenerator$${DEBUG_EXT}
  LIBS *= -lsymhop$${DEBUG_EXT}
  include($${PWD}/../dependencies/fmilibrary.pri)
  DEFINES *= HOPSANGENERATOR_DLLIMPORT
} else {
  message(Compiling HopsanCLI without HopsanGenerator)
}
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

    # Get git version info and define if return code success
    system("$${PWD}/../getGitInfo.sh date.time $${PWD}") {
        timestamp=$$system($${PWD}/../getGitInfo.sh date.time $${PWD})
        DEFINES *= "HOPSANCLI_COMMIT_TIMESTAMP=$${timestamp}"
    }
}
win32 {

    # Activate large adress aware, to access more the 2GB virtual RAM (for 32-bit version)
    !contains(QMAKE_HOST.arch, x86_64){
        QMAKE_LFLAGS += -Wl,--large-address-aware
    }

    # Get git version info and define if return code success
    system("$${PWD}/../getGitInfo.bat date.time $${PWD}") {
        timestamp=$$system($${PWD}/../getGitInfo.bat date.time $${PWD})
        DEFINES *= "HOPSANCLI_COMMIT_TIMESTAMP=$${timestamp}"
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
