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
INCLUDEPATH *= $${PWD}/Dependencies/rapidxml-1.13
INCLUDEPATH *= $${PWD}/Dependencies/libcsv_parser++-1.0.0/include/csv_parser
INCLUDEPATH *= $${PWD}/Dependencies/boost
#--------------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCore options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  DEFINES *= DEBUGCOMPILING
  QMAKE_CXXFLAGS += -pedantic -Wno-long-long
}
CONFIG(release, debug|release) {
  DEFINES *= RELEASECOMPILING
}
DEFINES *= MAINCORE

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE
 #   LIBS += -L"C:\Program Files\Microsoft SDKs\Windows\v7.1\Lib"
    LIBS += -lole32
    LIBS += -loleaut32

    #--------------------------------------------------------
    # Set the TBB LIBS and INCLUDEPATH (helpfunction for Windows)
    d = $$setTBBWindowsPathInfo($$(TBB_PATH), $$DESTDIR)
    !isEmpty(d){
        DEFINES *= USETBB       #If TBB was found then lets build core with TBB support
        message(Compiling HopsanCore with TBB support)
        LIBS *= $$magic_hopsan_libpath
        INCLUDEPATH *= $$magic_hopsan_includepath
        QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
    } else {
        message(Compiling HopsanCore WITHOUT TBB support)
    }
    #--------------------------------------------------------

    #Generate the svnrevnum.h file
    system($${PWD}/../getSvnRevision.bat)
}
unix { 
    DEFINES *= USETBB
    LIBS += -ltbb -ldl
    INCLUDEPATH += /usr/include/tbb/

    #Generate the svnrevnum.h file
    system($${PWD}/../getSvnRevision.sh include HOPSANCORESVNREVISION)
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
    #DO NOT remove the commented line bellow, it will be autoreplaced by script
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
    src/ComponentUtilities/ValveHysteresis.cc \
    src/ComponentUtilities/TurbulentFlowFunction.cc \
    src/ComponentUtilities/SecondOrderTransferFunction.cc \
    src/ComponentUtilities/matrix.cc \
    src/ComponentUtilities/ludcmp.cc \
    src/ComponentUtilities/IntegratorLimited.cc \
    src/ComponentUtilities/Integrator.cc \
    src/ComponentUtilities/FirstOrderTransferFunction.cc \
    src/ComponentUtilities/CSVParser.cc \
    Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp \
    src/Parameters.cc \
    src/ComponentUtilities/AuxiliarySimulationFunctions.cc \
    src/ComponentUtilities/DoubleIntegratorWithDamping.cc \
    src/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cc \
    src/ComponentUtilities/EquationSystemSolver.cpp \
    src/CoreUtilities/GeneratorHandler.cpp \
    src/CoreUtilities/MultiThreadingUtilities.cpp \
    src/CoreUtilities/CoSimulationUtilities.cpp \
    src/CoreUtilities/StringUtilities.cpp \
    src/HopsanTypes.cc
HEADERS += \
    include/win32dll.h \
    include/version.h \
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
    Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp \
    include/Parameters.h \
    include/Components/DummyComponent.hpp \
    include/ComponentUtilities/EquationSystemSolver.h \
    Dependencies/rapidxml-1.13/hopsan_rapidxml.hpp \
    include/CoreUtilities/GeneratorHandler.h \
    include/CoreUtilities/MultiThreadingUtilities.h \
    include/CoreUtilities/CoSimulationUtilities.h \
    include/CoreUtilities/StringUtilities.h \
    include/HopsanTypes.h

OTHER_FILES += \
    HopsanCoreBuild.prf








