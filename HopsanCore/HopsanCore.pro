# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( HopsanCoreBuild.prf )

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
INCLUDEPATH *= $${PWD}/../Dependencies/rapidxml-1.13
INCLUDEPATH *= $${PWD}/../Dependencies/libcsv_parser++-1.0.0/include/csv_parser
INCLUDEPATH *= $${PWD}/../Dependencies/IndexingCSVParser
#INCLUDEPATH *= $${PWD}/Dependencies/boost
#--------------------------------------------------------

#--------------------------------------------------------
# Set numHop paths, (compile in)
INCLUDEPATH *= $${PWD}/../Dependencies/libNumHop/libNumHop
SOURCES += $${PWD}/../Dependencies/libNumHop/libNumHop/Expression.cc
SOURCES += $${PWD}/../Dependencies/libNumHop/libNumHop/Helpfunctions.cc
SOURCES += $${PWD}/../Dependencies/libNumHop/libNumHop/VariableStorage.cc
HEADERS += $${PWD}/../Dependencies/libNumHop/libNumHop/numhop.h
#--------------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCore options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}

# Turn on the Hopsan Core runtime log file
DEFINES *= WRITEHOPSANCORELOG

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE

    #--------------------------------------------------------
    # Set the TBB LIBS and INCLUDEPATH (helpfunction for Windows)
    foundTBB = $$setTBBWindowsPathInfo($$(TBB_PATH), $$DESTDIR)
    equals(foundTBB, true) {
        DEFINES *= USETBB       #If TBB was found then lets build core with TBB support
        !build_pass:message("Compiling HopsanCore with TBB support")
        LIBS *= $$magic_hopsan_libpath
        INCLUDEPATH *= $$magic_hopsan_includepath
        QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
    } else {
        !build_pass:message("Compiling HopsanCore WITHOUT TBB support")
    }
    #--------------------------------------------------------

    # Enable auto-import
    QMAKE_LFLAGS += -Wl,--enable-auto-import

    # Retreive the hopsan core source code revision number
    system($${PWD}/../getSvnRevision.bat include HopsanCoreSVNRevision.h HOPSANCORESVNREVISION)
}
unix { 
    DEFINES *= USETBB
    LIBS += -ltbb -ldl
    INCLUDEPATH += /usr/include/tbb/

    # Retreive the hopsan core source code revision number
    system($${PWD}/../getSvnRevision.sh include HopsanCoreSVNRevision.h HOPSANCORESVNREVISION)
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
    src/ComponentUtilities/EquationSystemSolver.cpp \
    src/HopsanTypes.cc \
    src/ComponentUtilities/HopsanPowerUser.cc \
    src/ComponentUtilities/LookupTable.cc \
    src/ComponentUtilities/PLOParser.cc \
    ../Dependencies/IndexingCSVParser/IndexingCSVParser.cpp \
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
    ../Dependencies/rapidxml-1.13/hopsan_rapidxml.hpp \
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
    ../Dependencies/IndexingCSVParser/IndexingCSVParser.h \
    ../Dependencies/IndexingCSVParser/IndexingCSVParserImpl.hpp \
    include/Quantities.h \
    include/NodeRWHelpfuncs.hpp \
    include/HopsanCoreVersion.h \
    include/HopsanCoreSVNRevision.h \
    include/CoreUtilities/NumHopHelper.h \
    include/CoreUtilities/ConnectionAssistant.h \
    include/CoreUtilities/AliasHandler.h \
    include/CoreUtilities/SimulationHandler.h \
    include/CoreUtilities/SaveRestoreSimulationPoint.h

OTHER_FILES += \
    HopsanCoreBuild.prf








