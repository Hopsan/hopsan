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

QT += core gui network svg xml testlib
CONFIG -= app_bundle

isEqual(QT_MAJOR_VERSION, 5){
    QT += widgets printsupport
    unix {
        QT += webenginewidgets
        DEFINES *= USEWEBENGINE
    }
    win32 {
        qtHaveModule(webkitwidgets) {
            QT += webkitwidgets
            DEFINES *= USEWEBKIT
            message(Using WebKit)
        } else {
            message(WebKit is not available)
        }
    }
} else {
    QT += webkit
}

TARGET = $${TARGET}$${DEBUG_EXT}

#--------------------------------------------------------
# Set the QWT paths
include($${PWD}/../dependencies/qwt.pri)
!have_qwt(){
    !build_pass:error("Could not find QWT libs, have you compiled them in the expected location")
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set the ZeroMQ paths
include($${PWD}/../dependencies/zeromq.pri)
have_zeromq() {
    DEFINES *= USEZMQ
    !build_pass:message(Compiling HopsanGUI with ZeroMQ and msgpack support)
    include($${PWD}/../dependencies/msgpack.pri)

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
# Set hdf5exporter and hdf5 paths
LIBS += -L$${PWD}/../lib -lhopsanhdf5exporter$${DEBUG_EXT}
include($${PWD}/../dependencies/hdf5.pri)
have_hdf5(){
  INCLUDEPATH *= $${PWD}/../hopsanhdf5exporter
  DEFINES *= USEHDF5
  !build_pass:message("Compiling HopsanGUI with HDF5 support")
} else {
  LIBS -= -lhopsanhdf5exporter$${DEBUG_EXT}
  !build_pass:message("Compiling HopsanGUI without HDF5 support")
}
#--------------------------------------------------------

include($${PWD}/../dependencies/libzip.pri)
include($${PWD}/../dependencies/xerces.pri)

#--------------------------------------------------------
# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lhopsancore$${DEBUG_EXT}
DEFINES *= HOPSANCORE_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set hopsangeneratorgui Paths
INCLUDEPATH *= $${PWD}/../hopsangeneratorgui/include
LIBS *= -L$${PWD}/../lib -lhopsangeneratorgui$${DEBUG_EXT}
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
# Set hopsandcp Paths
INCLUDEPATH *= $${PWD}/../hopsandcp/include/
LIBS *= -L$${PWD}/../bin -lhopsandcp$${DEBUG_EXT}
DEFINES *= HOPSANDCP_DLLIMPORT
#--------------------------------------------------------

#--------------------------------------------------------
# Set Discount (libmarkdown) paths
include($${PWD}/../dependencies/discount.pri)
have_libmarkdown(){
  DEFINES *= USEDISCOUNT
  !build_pass:message(Compiling with Discount (libmarkdown) support)
} else {
  !build_pass:warning(Compiling WITHOUT Discount (libmarkdown) support)
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set our own HopsanGUI Include Path
INCLUDEPATH *= $${PWD}/
#--------------------------------------------------------

# -------------------------------------------------
# Platform independent additional project options
# -------------------------------------------------
# Development flag, will Gui be development version
DEFINES *= DEVELOPMENT

# Allow non-strict ansi code
QMAKE_CXXFLAGS *= -U__STRICT_ANSI__ -Wno-c++0x-compat

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
unix {
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
    GUIObjects/GUIWidgets.cpp \
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
    dependencies/BarChartPlotter/plotterbase.cpp \
    dependencies/BarChartPlotter/barchartplotter.cpp \
    dependencies/BarChartPlotter/axisbase.cpp \
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
    GUIObjects/GUIWidgets.h \
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
    dependencies/BarChartPlotter/plotterbase.h \
    dependencies/BarChartPlotter/barchartplotter.h \
    dependencies/BarChartPlotter/axisbase.h \
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
    Widgets/TextEditorWidget.h \
    HcomTest.hpp

OTHER_FILES += \
    ../hopsan-default-configuration.xml

DISTFILES +=

message($$LIBS)
