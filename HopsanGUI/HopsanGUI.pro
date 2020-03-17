# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( ../Common.prf )

TARGET = hopsangui
TEMPLATE = app
DESTDIR = $${PWD}/../bin

QT += svg xml
QT += core gui network
CONFIG -= app_bundle

isEqual(QT_MAJOR_VERSION, 5){
    QT += widgets printsupport
    qtHaveModule(webkitwidgets) {
        QT += webkitwidgets
        DEFINES *= USEWEBKIT
        message(Using WebKit)
    } else {
        message(WebKit is not available)
    }
} else {
    QT += webkit
}

TARGET = $${TARGET}$${DEBUG_EXT}

#--------------------------------------------------------
# Set the QWT paths
include($${PWD}/../Dependencies/qwt.pri)
!have_qwt(){
    !build_pass:error("Could not find QWT libs, have you compiled them in the expected location")
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set the PythonQt paths
include($${PWD}/../Dependencies/pythonqt.pri)
have_pythonqt(){
    DEFINES *= USEPYTHONQT       #If PythonQt was found then lets build GUI with PythonQt and Python support
    !build_pass:message(Compiling HopsanGUI with PythonQt support)
} else {
    !build_pass:warning(Compiling HopsanGUI WITHOUT PythonQt and Python support)
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set the ZeroMQ paths
include($${PWD}/../Dependencies/zeromq.pri)
have_zeromq() {
    DEFINES *= USEZMQ       #If ZMQ was found then lets build GUI with ZMQ / msgpack support
    !build_pass:message(Compiling HopsanGUI with ZeroMQ and msgpack support)
    include($${PWD}/../Dependencies/msgpack.pri)

    # Also require msgpack.c, setup msgpack path
    !have_msgpack() {
        !build_pass:error("Could not find msgpack-c, which is required for serialization")
    }

    #--------------------------------------------------------
    # Depend on the remoteclient lib
    INCLUDEPATH += $${PWD}/../hopsanremote/libhopsanremoteclient/include
    LIBS += -L$${PWD}/../lib -lhopsanremoteclient
    #--------------------------------------------------------

    #--------------------------------------------------------
    # Depend on the remote common lib
    INCLUDEPATH += $${PWD}/../hopsanremote/libhopsanremotecommon/include
    LIBS += -L$${PWD}/../lib -lhopsanremotecommon
    #--------------------------------------------------------

} else {
    !build_pass:warning("Could not find ZeroMQ, compiling HopsanGUI WITHOUT ZeroMQ support")
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set hopsangeneratorgui Paths
INCLUDEPATH *= $${PWD}/../hopsangeneratorgui/include
LIBS *= -L$${PWD}/../bin -lhopsangeneratorgui$${DEBUG_EXT}
#--------------------------------------------------------

#--------------------------------------------------------
# Set SymHop Paths
INCLUDEPATH *= $${PWD}/../SymHop/include/
LIBS *= -L$${PWD}/../bin -lsymhop$${DEBUG_EXT}
DEFINES *= SYMHOP_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set Ops Paths
INCLUDEPATH *= $${PWD}/../Ops/include/
LIBS *= -L$${PWD}/../bin -lops$${DEBUG_EXT}
DEFINES *= OPS_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set Discount (libmarkdown) paths
include($${PWD}/../Dependencies/discount.pri)
have_libmarkdown(){
  DEFINES *= USEDISCOUNT
  !build_pass:message(Compiling with Discount (libmarkdown) support)
} else {
  !build_pass:warning(Compiling WITHOUT Discount (libmarkdown) support)
}
#--------------------------------------------------------

# Set hdf5 paths
include($${PWD}/../Dependencies/hdf5.pri)
have_hdf5(){
  DEFINES *= USEHDF5
  !build_pass:message("Compiling with HDF5 support")
} else {
  !build_pass:message("Compiling without HDF5 support")
}



#--------------------------------------------------------
# Set our own HopsanGUI Include Path
INCLUDEPATH *= $${PWD}/
#--------------------------------------------------------

# -------------------------------------------------
# Platform independent additional project options
# -------------------------------------------------
# Development flag, will Gui be development version
DEFINES *= DEVELOPMENT

# Enable C++11
lessThan(QT_MAJOR_VERSION, 5){
  QMAKE_CXXFLAGS += -std=c++11
} else {
  CONFIG += c++11
}
# Allow non-strict ansi code
QMAKE_CXXFLAGS *= -U__STRICT_ANSI__ -Wno-c++0x-compat

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
unix {
    # Set Python paths
    contains(DEFINES, USEPYTHONQT) {
        !build_pass:message("Looking for Python include and lib paths since USEPYTHONQT is defined")
        !build_pass:message("PythonQt appears to require Python $${PYTHONQT_PYVERSION}")
        QMAKE_CXXFLAGS *= $$system(python$${PYTHONQT_PYVERSION}-config --includes)
        LIBS *= $$system(python$${PYTHONQT_PYVERSION}-config --libs)
    } else {
        !build_pass:message("Not looking for Python since we are not using PythonQT")
    }

    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be relative the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to handle the $$ORIGIN stuff, adding manually to LFLAGS
    !macx:QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
     macx:QMAKE_RPATHDIR *= $${PWD}/../bin

    # Get the git commit timestamp and define it (if the command succeeds)
    system("$${PWD}/../getGitInfo.sh date.time $${PWD}") {
        timestamp=$$system($${PWD}/../getGitInfo.sh date.time $${PWD})
        DEFINES *= "HOPSANGUI_COMMIT_TIMESTAMP=$${timestamp}"
    }
}
win32 {
    # Set Python paths
    contains(DEFINES, USEPYTHONQT) {
        !build_pass:message("Looking for Python include and lib paths since USEPYTHONQT is defined")
        PYTHON_DEFAULT_PATHS *= c:/Python27
        PYTHON_PATH = $$selectPath($$(PYTHON_PATH), $$PYTHON_DEFAULT_PATHS, "python")
        INCLUDEPATH += $${PYTHON_PATH}/include
        LIBS += -L$${PYTHON_PATH}/libs
    } else {
       !build_pass: message("Not looking for Python since we are not using PythonQT")
    }

    # Enable auto-import
    QMAKE_LFLAGS += -Wl,--enable-auto-import

    # Activate large address aware, to access more the 2GB virtual RAM (for 32-bit version)
    !contains(QMAKE_HOST.arch, x86_64){
        QMAKE_LFLAGS += -Wl,--large-address-aware
    }

    # Activate console output of cout for debug builds (you also need to run in console but hopsan seems slow)
#    CONFIG(debug, debug|release) {
        CONFIG += console
#    }

    # Get the git commit timestamp and define it (if the command succeeds)
    system("$${PWD}/../getGitInfo.bat date.time $${PWD}") {
        timestamp=$$system($${PWD}/../getGitInfo.bat date.time $${PWD})
        DEFINES *= "HOPSANGUI_COMMIT_TIMESTAMP=$${timestamp}"
    }
}
macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

#Debug output
#message(GUI Includepath is $$INCLUDEPATH)
#message(GUI Libs is $${LIBS})
#message(GUI QMAKE_POST_LINK $$QMAKE_POST_LINK)

RESOURCES += \
    Resources.qrc

# Release compile only, will add the application icon
RC_FILE = HOPSANGUI.rc

# -------------------------------------------------
# Project files
# -------------------------------------------------
SOURCES += main.cpp \
    BuiltinTests.cpp \
    MainWindow.cpp \
    Widgets/ProjectTabWidget.cpp \
    GUIConnector.cpp \
    GUIPort.cpp \
    Widgets/MessageWidget.cpp \
    InitializationThread.cpp \
    Dialogs/OptionsDialog.cpp \
    UndoStack.cpp \
    GraphicsView.cpp \
    ProgressBarThread.cpp \
    GUIPortAppearance.cpp \
    GUIConnectorAppearance.cpp \
    Widgets/SystemParametersWidget.cpp \
    PlotWindow.cpp \
    PyWrapperClasses.cpp \
    GUIObjects/GUIWidgets.cpp \
    GUIObjects/GUISystem.cpp \
    GUIObjects/GUIObject.cpp \
    GUIObjects/GUIModelObjectAppearance.cpp \
    GUIObjects/GUIModelObject.cpp \
    GUIObjects/GUIContainerObject.cpp \
    GUIObjects/GUIComponent.cpp \
    Utilities/XMLUtilities.cpp \
    Utilities/GUIUtilities.cpp \
    Configuration.cpp \
    CopyStack.cpp \
    Dialogs/AboutDialog.cpp \
    CoreAccess.cpp \
    Widgets/UndoWidget.cpp \
    Widgets/QuickNavigationWidget.cpp \
    GUIObjects/GUIContainerPort.cpp \
    Dialogs/ContainerPortPropertiesDialog.cpp \
    Dialogs/HelpDialog.cpp \
    Dependencies/BarChartPlotter/plotterbase.cpp \
    Dependencies/BarChartPlotter/barchartplotter.cpp \
    Dependencies/BarChartPlotter/axisbase.cpp \
    Dialogs/OptimizationDialog.cpp \
    Dialogs/SensitivityAnalysisDialog.cpp \
    Dialogs/MovePortsDialog.cpp \
    loadFunctions.cpp \
    Widgets/WelcomeWidget.cpp \
    Widgets/AnimationWidget.cpp \
    GUIObjects/AnimatedComponent.cpp \
    AnimatedConnector.cpp \
    Dialogs/AnimatedIconPropertiesDialog.cpp \
    SimulationThreadHandler.cpp \
    Widgets/HcomWidget.cpp \
    LogDataHandler2.cpp \
    PlotTab.cpp \
    PlotCurve.cpp \
    PlotHandler.cpp \
    LogVariable.cpp \
    CachableDataVector.cpp \
    DesktopHandler.cpp \
    Dialogs/ComponentPropertiesDialog3.cpp \
    Dialogs/EditComponentDialog.cpp \
    Widgets/DebuggerWidget.cpp \
    HcomHandler.cpp \
    Widgets/HVCWidget.cpp \
    ModelHandler.cpp \
    Widgets/ModelWidget.cpp \
    OptimizationHandler.cpp \
    Utilities/HighlightingUtilities.cpp \
    Widgets/DataExplorer.cpp \
    Widgets/LibraryWidget.cpp \
    LibraryHandler.cpp \
    UnitScale.cpp \
    PlotArea.cpp \
    Utilities/HelpPopUpWidget.cpp \
    PlotCurveControlBox.cpp \
    MessageHandler.cpp \
    Widgets/FindWidget.cpp \
    ModelicaLibrary.cpp \
    Widgets/ModelicaEditor.cpp \
    Widgets/PlotWidget2.cpp \
    Utilities/IndexIntervalCollection.cpp \
    LogDataGeneration.cpp \
    RemoteCoreAccess.cpp \
    RemoteSimulationUtils.cpp \
    Dialogs/LicenseDialog.cpp \
    Widgets/TimeOffsetWidget.cpp \
    Dialogs/NumHopScriptDialog.cpp \
    PlotCurveStyle.cpp \
    Utilities/WebviewWrapper.cpp \
    GeneratorUtils.cpp \
    Dialogs/OptimizationScriptWizard.cpp \
    Widgets/TextEditorWidget.cpp

HEADERS += MainWindow.h \
    BuiltinTests.h \
    Widgets/ProjectTabWidget.h \
    GUIConnector.h \
    GUIPort.h \
    Widgets/MessageWidget.h \
    InitializationThread.h \
    version_gui.h \
    Dialogs/OptionsDialog.h \
    UndoStack.h \
    CoreAccess.h \
    GraphicsView.h \
    ProgressBarThread.h \
    common.h \
    CoreAccess.h \
    GUIPortAppearance.h \
    GUIConnectorAppearance.h \
    Widgets/SystemParametersWidget.h \
    PlotWindow.h \
    PyWrapperClasses.h \
    GUIObjects/GUIWidgets.h \
    GUIObjects/GUISystem.h \
    GUIObjects/GUIObject.h \
    GUIObjects/GUIModelObjectAppearance.h \
    GUIObjects/GUIModelObject.h \
    GUIObjects/GUIContainerObject.h \
    GUIObjects/GUIComponent.h \
    Utilities/XMLUtilities.h \
    Utilities/GUIUtilities.h \
    Configuration.h \
    CopyStack.h \
    Dialogs/AboutDialog.h \
    Widgets/UndoWidget.h \
    Widgets/QuickNavigationWidget.h \
    GUIObjects/GUIContainerPort.h \
    Dialogs/ContainerPortPropertiesDialog.h \
    Dialogs/HelpDialog.h \
    Dependencies/BarChartPlotter/plotterbase.h \
    Dependencies/BarChartPlotter/barchartplotter.h \
    Dependencies/BarChartPlotter/axisbase.h \
    Dialogs/OptimizationDialog.h \
    Dialogs/SensitivityAnalysisDialog.h \
    Dialogs/MovePortsDialog.h \
    loadFunctions.h \
    Widgets/WelcomeWidget.h \
    Widgets/AnimationWidget.h \
    GUIObjects/AnimatedComponent.h \
    AnimatedConnector.h \
    Dialogs/AnimatedIconPropertiesDialog.h \
    SimulationThreadHandler.h \
    Widgets/HcomWidget.h \
    LogDataHandler2.h \
    PlotTab.h \
    PlotCurve.h \
    PlotHandler.h \
    LogVariable.h \
    CachableDataVector.h \
    DesktopHandler.h \
    Dialogs/ComponentPropertiesDialog3.h \
    Dialogs/EditComponentDialog.h \
    Widgets/DebuggerWidget.h \
    HcomHandler.h \
    Widgets/HVCWidget.h \
    ModelHandler.h \
    Widgets/ModelWidget.h \
    OptimizationHandler.h \
    Utilities/HighlightingUtilities.h \
    Widgets/DataExplorer.h \
    Widgets/LibraryWidget.h \
    LibraryHandler.h \
    global.h \
    UnitScale.h \
    PlotArea.h \
    Utilities/HelpPopUpWidget.h \
    PlotCurveControlBox.h \
    MessageHandler.h \
    Widgets/FindWidget.h \
    ModelicaLibrary.h \
    Widgets/ModelicaEditor.h \
    GraphicsViewPort.h \
    Widgets/PlotWidget2.h \
    Utilities/IndexIntervalCollection.h \
    LogDataGeneration.h \
    RemoteCoreAccess.h \
    RemoteSimulationUtils.h \
    Dialogs/LicenseDialog.h \
    Utilities/EventFilters.h \
    Widgets/TimeOffsetWidget.h \
    Dialogs/NumHopScriptDialog.h \
    PlotCurveStyle.h \
    Utilities/WebviewWrapper.h \
    GeneratorUtils.h \
    Dialogs/OptimizationScriptWizard.h \
    Widgets/TextEditorWidget.h

    contains(DEFINES, USEPYTHONQT) {
        SOURCES += Widgets/PyDockWidget.cpp
        HEADERS += Widgets/PyDockWidget.h
    }

OTHER_FILES += \
    ../hopsan-default-configuration.xml

DISTFILES +=
