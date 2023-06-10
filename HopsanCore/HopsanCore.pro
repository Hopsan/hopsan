# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsancore
TEMPLATE = lib
CONFIG += shared
DESTDIR = $${PWD}/../bin

QT -= core \
    gui

TARGET = $${TARGET}$${DEBUG_EXT}


#--------------------------------------------------
# Add the include path to our self, (HopsanCore)
INCLUDEPATH *= $${PWD}/include/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to default components (while they are compiled in)
INCLUDEPATH *= $${PWD}/../componentLibraries/defaultLibrary
#--------------------------------------------------

#--------------------------------------------------------
# Set the rappidxml and csv_parser include paths
INCLUDEPATH *= $${PWD}/dependencies/rapidxml
INCLUDEPATH *= $${PWD}/dependencies/indexingcsvparser/include
#--------------------------------------------------------

#--------------------------------------------------------
# Set numHop paths, (compile in)
INCLUDEPATH *= $${PWD}/dependencies/libnumhop/include
SOURCES += $${PWD}/dependencies/libnumhop/src/Expression.cpp
SOURCES += $${PWD}/dependencies/libnumhop/src/Helpfunctions.cpp
SOURCES += $${PWD}/dependencies/libnumhop/src/VariableStorage.cpp
HEADERS += $${PWD}/dependencies/libnumhop/include/numhop.h
#--------------------------------------------------------

#-------------------------------------------------
# Set sundials paths (compile in)
INCLUDEPATH *= $${PWD}/dependencies/sundials-extra/include
INCLUDEPATH *= $${PWD}/dependencies/sundials/include
SOURCES += $${PWD}/dependencies/sundials/src/kinsol/kinsol_spils.c \
           $${PWD}/dependencies/sundials/src/kinsol/kinsol_ls.c \
           $${PWD}/dependencies/sundials/src/kinsol/kinsol_io.c \
           $${PWD}/dependencies/sundials/src/kinsol/kinsol_direct.c \
           $${PWD}/dependencies/sundials/src/kinsol/kinsol_bbdpre.c \
           $${PWD}/dependencies/sundials/src/kinsol/kinsol.c \
           $${PWD}/dependencies/sundials/src/sunmatrix/dense/fsunmatrix_dense.c \
           $${PWD}/dependencies/sundials/src/sunmatrix/dense/sunmatrix_dense.c \
           $${PWD}/dependencies/sundials/src/nvector/serial/fnvector_serial.c \
           $${PWD}/dependencies/sundials/src/nvector/serial/nvector_serial.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_futils.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_nvector_senswrapper.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_nonlinearsolver.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_version.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_iterative.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_band.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_direct.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_dense.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_nvector.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_linearsolver.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_matrix.c \
           $${PWD}/dependencies/sundials/src/sundials/sundials_math.c \
           $${PWD}/dependencies/sundials/src/sunmatrix/band/sunmatrix_band.c \
           $${PWD}/dependencies/sundials/src/sunlinsol/band/sunlinsol_band.c \
           $${PWD}/dependencies/sundials/src/sunlinsol/dense/sunlinsol_dense.c \
#-------------------------------------------------

# -------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCore options
# -------------------------------------------------
QMAKE_CXXFLAGS += -pedantic -Wno-long-long

# Enable the use of M_PI and such
DEFINES *= _USE_MATH_DEFINES

# Turn on the Hopsan Core runtime log file
DEFINES *= HOPSANCORE_WRITELOG

# Turn off multi-threading
# DEFINES *= HOPSANCORE_NOMULTITHREADING


# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    # Turn on symbol export when building a  DLL
    DEFINES += HOPSANCORE_DLLEXPORT
    # Disable unicode strings (use ascii)
    DEFINES -= UNICODE

    # Enable auto-import
    QMAKE_LFLAGS += -Wl,--enable-auto-import

    # Retreive the HopsanCore source code version info and regenerate version header (if getGitInfo succeeds)
    system("$${PWD}/../getGitInfo.bat shorthash $${PWD}") {
        commitdatetime=$$system($${PWD}/../getGitInfo.bat date.time $${PWD})
        commithash=$$system($${PWD}/../getGitInfo.bat shorthash $${PWD})
        message(Core revision: $${commitdatetime})
        message(Core hash: $${commithash})
        system($${PWD}/../writeGitVersionHeader.bat $${PWD}/include/HopsanCoreGitVersion.h HOPSANCORE $${commithash} $${commitdatetime})
    }
}
unix {
    LIBS += -ldl

    # Add runtime search path so that dynamically loaded libraries in the same directory can be found.
    # Note! QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem hande $$ORIGIN, adding manually to LFLAGS
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

    # Retreive the HopsanCore source code version info and regenerate version header (if getGitInfo succeeds)
    system("$${PWD}/../getGitInfo.sh shorthash $${PWD}") {
        commitdatetime=$$system($${PWD}/../getGitInfo.sh date.time $${PWD})
        commithash=$$system($${PWD}/../getGitInfo.sh shorthash $${PWD})
        message(Core revision: $${commitdatetime})
        message(Core hash: $${commithash})
        system($${PWD}/../writeGitVersionHeader.sh $${PWD}/include/HopsanCoreGitVersion.h HOPSANCORE $${commithash} $${commitdatetime})
    }
}
macx {
    INCLUDEPATH += /opt/local/include/
    QMAKE_LIBDIR += /opt/local/lib/
}

#Debug output
#message(CORE QMAKE_POST_LINK $${QMAKE_POST_LINK})
#message(CORE Includepath is $$INCLUDEPATH)
#message(CORE Libs is $${LIBS})
#message(CORE Defines is $${DEFINES})

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += \
    #DO NOT remove the commented line below, it will be autoreplaced by script
    #INTERNALCOMPLIB.CPP#
    src/Port.cpp \
    src/Nodes.cpp \
    src/Node.cpp \
    src/HopsanEssentials.cpp \
    src/ComponentSystem.cpp \
    src/Component.cpp \
    src/CoreUtilities/LoadExternal.cpp \
    src/CoreUtilities/HopsanCoreMessageHandler.cpp \
    src/CoreUtilities/HmfLoader.cpp \
    src/ComponentUtilities/WhiteGaussianNoise.cpp \
    src/ComponentUtilities/SecondOrderTransferFunction.cpp \
    src/ComponentUtilities/matrix.cpp \
    src/ComponentUtilities/ludcmp.cpp \
    src/ComponentUtilities/IntegratorLimited.cpp \
    src/ComponentUtilities/FirstOrderTransferFunction.cpp \
    src/ComponentUtilities/CSVParser.cpp \
    src/Parameters.cpp \
    src/ComponentUtilities/AuxiliarySimulationFunctions.cpp \
    src/ComponentUtilities/DoubleIntegratorWithDamping.cpp \
    src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cpp \
    src/ComponentUtilities/EquationSystemSolver.cpp \
    src/HString.cpp \
    src/ComponentUtilities/HopsanPowerUser.cpp \
    src/ComponentUtilities/LookupTable.cpp \
    src/ComponentUtilities/PLOParser.cpp \
    src/ComponentUtilities/TempDirectoryHandle.cpp \
    $${PWD}/dependencies/indexingcsvparser/src/indexingcsvparser.cpp \
    src/Quantities.cpp \
    src/CoreUtilities/NumHopHelper.cpp \
    src/CoreUtilities/AliasHandler.cpp \
    src/CoreUtilities/ConnectionAssistant.cpp \
    src/CoreUtilities/SimulationHandler.cpp \
    src/CoreUtilities/MultiThreadingUtilities.cpp \
    src/CoreUtilities/StringUtilities.cpp \
    src/CoreUtilities/SaveRestoreSimulationPoint.cpp
HEADERS += \
    include/win32dll.h \
    include/Port.h \
    include/Nodes.h \
    include/Node.h \
    include/HopsanEssentials.h \
    include/HopsanCore.h \
    include/HString.h \
    include/HVector.hpp \
    include/ComponentUtilities.h \
    include/ComponentSystem.h \
    include/ComponentEssentials.h \
    include/Component.h \
    include/CoreUtilities/LoadExternal.h \
    include/CoreUtilities/HopsanCoreMessageHandler.h \
    include/CoreUtilities/HmfLoader.h \
    include/CoreUtilities/ClassFactoryStatusCheck.hpp \
    include/CoreUtilities/ClassFactory.hpp \
    include/ComponentUtilities/WhiteGaussianNoise.h \
    include/ComponentUtilities/ValveHysteresis.h \
    include/ComponentUtilities/TurbulentFlowFunction.h \
    include/ComponentUtilities/SecondOrderTransferFunction.h \
    include/ComponentUtilities/num2string.hpp \
    include/ComponentUtilities/matrix.h \
    include/ComponentUtilities/ludcmp.h \
    include/ComponentUtilities/IntegratorLimited.h \
    include/ComponentUtilities/Integrator.h \
    include/ComponentUtilities/FirstOrderTransferFunction.h \
    include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h \
    include/ComponentUtilities/DoubleIntegratorWithDamping.h \
    include/ComponentUtilities/Delay.hpp \
    include/ComponentUtilities/CSVParser.h \
    include/ComponentUtilities/AuxiliarySimulationFunctions.h \
    include/ComponentUtilities/AuxiliaryMathematicaWrapperFunctions.h \
    include/ComponentUtilities/TempDirectoryHandle.h \
    include/Parameters.h \
    include/Components/DummyComponent.hpp \
    include/ComponentUtilities/EquationSystemSolver.h \
    $${PWD}/dependencies/rapidxml/hopsan_rapidxml.hpp \
    include/CoreUtilities/MultiThreadingUtilities.h \
    include/CoreUtilities/StringUtilities.h \
    include/HopsanTypes.h \
    include/ComponentUtilities/HopsanPowerUser.h \
    include/HopsanCoreMacros.h \
    include/compiler_info.h \
    include/ComponentUtilities/LookupTable.h \
    include/ComponentUtilities/PLOParser.h \
    $${PWD}/dependencies/indexingcsvparser/include/indexingcsvparser/indexingcsvparser.h \
    include/Quantities.h \
    include/NodeRWHelpfuncs.hpp \
    include/HopsanCoreVersion.h \
    include/HopsanCoreGitVersion.h \
    include/CoreUtilities/NumHopHelper.h \
    include/CoreUtilities/ConnectionAssistant.h \
    include/CoreUtilities/AliasHandler.h \
    include/CoreUtilities/SimulationHandler.h \
    include/CoreUtilities/SaveRestoreSimulationPoint.h

#DO NOT remove the commented line below, it will be autoreplaced by script
#INTERNALCOMPLIB_FMI4C_DEPENDENCY#
