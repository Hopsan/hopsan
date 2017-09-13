# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = HopsanCore
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
INCLUDEPATH *= $${PWD}/dependencies/IndexingCSVParser
#--------------------------------------------------------

#--------------------------------------------------------
# Set numHop paths, (compile in)
INCLUDEPATH *= $${PWD}/dependencies/libNumHop/libNumHop
SOURCES += $${PWD}/dependencies/libNumHop/libNumHop/Expression.cc
SOURCES += $${PWD}/dependencies/libNumHop/libNumHop/Helpfunctions.cc
SOURCES += $${PWD}/dependencies/libNumHop/libNumHop/VariableStorage.cc
HEADERS += $${PWD}/dependencies/libNumHop/libNumHop/numhop.h
#--------------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCore options
# -------------------------------------------------
QMAKE_CXXFLAGS += -pedantic -Wno-long-long

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}
# Enable the use of M_PI and such
DEFINES *= _USE_MATH_DEFINES

# Turn on the Hopsan Core runtime log file
DEFINES *= WRITEHOPSANCORELOG

# Turn on multi-threading
DEFINES *= MULTITHREADING


# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE

    # Enable auto-import
    QMAKE_LFLAGS += -Wl,--enable-auto-import

    # Retreive the HopsanCore source code version info and regenerate version header
    commitdatetime=$$system($${PWD}/../getGitInfo.bat date.time $${PWD})
    commithash=$$system($${PWD}/../getGitInfo.bat shorthash $${PWD})
    message(Core revision: $${commitdatetime})
    message(Core hash: $${commithash})
    system($${PWD}/../writeGitVersionHeader.bat $${PWD}/include/HopsanCoreGitVersion.h HOPSANCORE $${commithash} $${commitdatetime})
}
unix { 
    LIBS += -ldl

    # Retreive the HopsanCore source code version info and regenerate version header
    commitdatetime=$$system($${PWD}/../getGitInfo.sh date.time $${PWD})
    commithash=$$system($${PWD}/../getGitInfo.sh shorthash $${PWD})
    message(Core revision: $${commitdatetime})
    message(Core hash: $${commithash})
    system($${PWD}/../writeGitVersionHeader.sh $${PWD}/include/HopsanCoreGitVersion.h HOPSANCORE $${commithash} $${commitdatetime})
 
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
    #INTERNALCOMPLIB.CC#
    src/Port.cc \
    src/Nodes.cc \
    src/Node.cc \
    src/HopsanEssentials.cc \
    src/ComponentSystem.cc \
    src/Component.cc \
    src/CoreUtilities/LoadExternal.cc \
    src/CoreUtilities/HopsanCoreMessageHandler.cc \
    src/CoreUtilities/HmfLoader.cc \
    src/ComponentUtilities/WhiteGaussianNoise.cc \
    src/ComponentUtilities/SecondOrderTransferFunction.cc \
    src/ComponentUtilities/matrix.cc \
    src/ComponentUtilities/ludcmp.cc \
    src/ComponentUtilities/IntegratorLimited.cc \
    src/ComponentUtilities/FirstOrderTransferFunction.cc \
    src/ComponentUtilities/CSVParser.cc \
    src/Parameters.cc \
    src/ComponentUtilities/AuxiliarySimulationFunctions.cc \
    src/ComponentUtilities/DoubleIntegratorWithDamping.cc \
    src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc \
    src/ComponentUtilities/EquationSystemSolver.cc \
    src/HopsanTypes.cc \
    src/ComponentUtilities/HopsanPowerUser.cc \
    src/ComponentUtilities/LookupTable.cc \
    src/ComponentUtilities/PLOParser.cc \
    $${PWD}/dependencies/IndexingCSVParser/IndexingCSVParser.cpp \
    src/Quantities.cc \
    src/CoreUtilities/NumHopHelper.cc \
    src/CoreUtilities/AliasHandler.cc \
    src/CoreUtilities/ConnectionAssistant.cc \
    src/CoreUtilities/SimulationHandler.cc \
    src/CoreUtilities/CoSimulationUtilities.cc \
    src/CoreUtilities/GeneratorHandler.cc \
    src/CoreUtilities/MultiThreadingUtilities.cc \
    src/CoreUtilities/StringUtilities.cc \
    src/CoreUtilities/SaveRestoreSimulationPoint.cc
HEADERS += \
    include/win32dll.h \
    include/Port.h \
    include/Nodes.h \
    include/Node.h \
    include/HopsanEssentials.h \
    include/HopsanCore.h \
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
    include/Parameters.h \
    include/Components/DummyComponent.hpp \
    include/ComponentUtilities/EquationSystemSolver.h \
    $${PWD}/dependencies/rapidxml/hopsan_rapidxml.hpp \
    include/CoreUtilities/GeneratorHandler.h \
    include/CoreUtilities/MultiThreadingUtilities.h \
    include/CoreUtilities/CoSimulationUtilities.h \
    include/CoreUtilities/StringUtilities.h \
    include/HopsanTypes.h \
    include/ComponentUtilities/HopsanPowerUser.h \
    include/HopsanCoreMacros.h \
    include/compiler_info.h \
    include/Components/ModelicaComponent.hpp \
    include/ComponentUtilities/LookupTable.h \
    include/ComponentUtilities/PLOParser.h \
    $${PWD}/dependencies/IndexingCSVParser/IndexingCSVParser.h \
    $${PWD}/dependencies/IndexingCSVParser/IndexingCSVParserImpl.hpp \
    include/Quantities.h \
    include/NodeRWHelpfuncs.hpp \
    include/HopsanCoreVersion.h \
    include/HopsanCoreGitVersion.h \
    include/CoreUtilities/NumHopHelper.h \
    include/CoreUtilities/ConnectionAssistant.h \
    include/CoreUtilities/AliasHandler.h \
    include/CoreUtilities/SimulationHandler.h \
    include/CoreUtilities/SaveRestoreSimulationPoint.h









