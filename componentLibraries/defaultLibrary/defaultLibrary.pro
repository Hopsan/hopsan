include( ../../Common.prf )

# QT -= core gui, means that we should not link the default qt libs into the component
# Template = lib, means that we want to build a library (.dll or .so)
QT -= core gui
TEMPLATE = lib

# TARGET is the name of the compiled lib, (.dll or .so will be added automatically)
# Change this to the name of YOUR lib
TARGET = defaultcomponentlibrary
TARGET = $${TARGET}$${DEBUG_EXT}

# Destination for the compiled dll. $${PWD}/ means the same directory as this .pro file, even if you use shadow build
DESTDIR = $${PWD}/

# The location to search for the Hopsan include files, by specifying the path here, you dont need to do this everywhere in all of your component .hpp files
# You can also add additional paths for eg. your own Utility functions, just add additional INCLUDEPATH *= ... lines.
# *= Means append unique
INCLUDEPATH *= $${PWD}/code/
INCLUDEPATH *= $${PWD}/../../HopsanCore/include/
INCLUDEPATH *= $${PWD}/../../HopsanCore/dependencies/sundials/include/
INCLUDEPATH *= $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/
INCLUDEPATH *= $${PWD}/../../HopsanCore/dependencies/sundials-extra/include/
SOURCES += $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol_spils.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol_ls.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol_io.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol_direct.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol_bbdpre.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/kinsol/kinsol.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/cvode/cvode.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/cvode/cvode_io.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/cvode/cvode_ls.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/cvode/cvode_nls.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/cvode/cvode_proj.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunnonlinsol/newton/sunnonlinsol_newton.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunnonlinsol/fixedpoint/sunnonlinsol_fixedpoint.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunmatrix/dense/fsunmatrix_dense.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunmatrix/dense/sunmatrix_dense.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/nvector/serial/fnvector_serial.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/nvector/serial/nvector_serial.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_futils.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_nvector_senswrapper.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_nonlinearsolver.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_version.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_iterative.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_band.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_direct.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_dense.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_nvector.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_linearsolver.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_matrix.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sundials/sundials_math.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunmatrix/band/sunmatrix_band.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunlinsol/band/sunlinsol_band.c \
           $${PWD}/../../HopsanCore/dependencies/sundials/src/sunlinsol/dense/sunlinsol_dense.c \

# The location of the HopsanCore .dll or .so file, needed to link against when compiling your library
LIBS *= -L$${PWD}/../../bin

# In debug mode HopsanCore has the debug extension _d
LIBS *= -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT

# Reduce compile output clutter, but show warnings
CONFIG += silent warn_on plugin

# The compiler should be pedantic to catch all errors (optional)
QMAKE_CXXFLAGS += -pedantic -Wno-long-long -Wconversion

# Enable C++14
CONFIG += c++14

# Enable the use of M_PI and such
DEFINES *= _USE_MATH_DEFINES

# -------------------------------------------------
# Project files
# -------------------------------------------------
HEADERS += \

SOURCES += \
    defaultComponentLibraryInternal.cpp \
    defaultComponentLibrary.cpp

OTHER_FILES += \

include($${PWD}/../../dependencies/fmi4c.pri)
include(Components.pri)
