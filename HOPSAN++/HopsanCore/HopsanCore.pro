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
INCLUDEPATH *= $${PWD}/
#--------------------------------------------------

#--------------------------------------------------
# Add the include path to default components (while they are compiled in)
INCLUDEPATH *= $${PWD}/../componentLibraries/defaultLibrary/code
#--------------------------------------------------

#--------------------------------------------------------
# Set the rappidxml and csv_parser include paths
INCLUDEPATH *= $${PWD}/Dependencies/rapidxml-1.13
INCLUDEPATH *= $${PWD}/Dependencies/libcsv_parser++-1.0.0/include/csv_parser
#--------------------------------------------------------

# -------------------------------------------------
# Non platform specific HopsanCore options
# -------------------------------------------------
CONFIG(debug, debug|release) {
  DEFINES *= DEBUGCOMPILING
}
CONFIG(release, debug|release) {
  DEFINES *= RELEASECOMPILING
}

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
win32 {
    #DEFINES += STATICCORE      #Use this if you are compiling the core into a program directly or building a static lib
    DEFINES += DOCOREDLLEXPORT  #Use this if you are compiling the core as a DLL or SO
    DEFINES -= UNICODE

    #--------------------------------------------------------
    # Set the TBB LIBS and INCLUDEPATH (helpfunction for Windows)
    d = $$setTBBWindowsPathInfo($$(TBB_PATH), $$DESTDIR)
    !isEmpty(d){
        DEFINES *= USETBB       #If TBB was found then lets build core with TBB support
        message(Compiling HopsanCore with TBB support)
        LIBS *= $$magic_hopsan_libpath
        INCLUDEPATH *= $$magic_hopsan_includepath
        QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
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
    system($${PWD}/../getSvnRevision.sh)
}

#Debug output
#message(CORE QMAKE_POST_LINK $${QMAKE_POST_LINK})
#message(CORE Includepath is $$INCLUDEPATH)
#message(CORE Libs is $${LIBS})
#message(CORE Defines is $${DEFINES})

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += Port.cc \
    Node.cc \
    Component.cc \
    Nodes/Nodes.cc \
    CoreUtilities/LoadExternal.cc \
    HopsanEssentials.cc \
    ComponentUtilities/ValveHysteresis.cc \
    ComponentUtilities/TurbulentFlowFunction.cc \
    ComponentUtilities/IntegratorLimited.cc \
    ComponentUtilities/Integrator.cc \
    CoreUtilities/HopsanCoreMessageHandler.cc \
    ComponentUtilities/AuxiliarySimulationFunctions.cpp \
    ComponentUtilities/DoubleIntegratorWithDamping.cpp \
    ComponentUtilities/matrix.cc \
    ComponentUtilities/ludcmp.cc \
    CoreUtilities/HmfLoader.cc \
    ComponentSystem.cc \
    Dependencies/libcsv_parser++-1.0.0/csv_parser.cpp \
    ComponentUtilities/WhiteGaussianNoise.cc \
    ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.cpp \
    ComponentUtilities/SecondOrderTransferFunction.cc \
    ComponentUtilities/FirstOrderTransferFunction.cc \
    ComponentUtilities/CSVParser.cc \
    ../componentLibraries/defaultLibrary/code/Components.cc
HEADERS += win32dll.h \
    Port.h \
    Node.h \
    HopsanCore.h \
    Component.h \
    Nodes/Nodes.h \
    CoreUtilities/LoadExternal.h \
    CoreUtilities/ClassFactory.hpp \
    ComponentEssentials.h \
    ComponentUtilities.h \
    HopsanEssentials.h \
    ComponentUtilities/ValveHysteresis.h \
    ComponentUtilities/TurbulentFlowFunction.h \
    ComponentUtilities/IntegratorLimited.h \
    ComponentUtilities/Integrator.h \
    ComponentUtilities/Delay.hpp \
    CoreUtilities/HopsanCoreMessageHandler.h \
    version.h \
    ComponentUtilities/AuxiliarySimulationFunctions.h \
    ComponentUtilities/DoubleIntegratorWithDamping.h \
    ComponentUtilities/matrix.h \
    ComponentUtilities/ludcmp.h \
    CoreUtilities/HmfLoader.h \
    ComponentSystem.h \
    CoreUtilities/FindUniqueName.h \
    ComponentUtilities/CSVParser.h \
    CoreUtilities/ClassFactoryStatusCheck.hpp \
    ComponentUtilities/WhiteGaussianNoise.h \
    ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h \
    ComponentUtilities/SecondOrderTransferFunction.h \
    ComponentUtilities/FirstOrderTransferFunction.h \
    Dependencies/libcsv_parser++-1.0.0/include/csv_parser/csv_parser.hpp \
    ComponentUtilities/num2string.hpp \
    ComponentUtilities/AuxiliaryMathematicaWrapperFunctions.h

OTHER_FILES += \
    HopsanCoreBuild.prf






