# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( HopsanGuiBuild.prf )
include( $${PWD}/../HopsanRemote/HopsanRemoteBuild.pri )

TARGET = HopsanGUI
TEMPLATE = app
DESTDIR = $${PWD}/../bin

macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9


QT += svg xml
QT += core gui network

isEqual(QT_MAJOR_VERSION, 5){
    QT += widgets webkitwidgets multimedia printsupport
} else {
    QT += webkit
}


TARGET = $${TARGET}$${DEBUG_EXT}

# Make c++11 mandatory but allow non-strict ANSI
QMAKE_CXXFLAGS *= -std=c++11 -U__STRICT_ANSI__ -Wno-c++0x-compat

#--------------------------------------------------------
# Set the QWT paths and dll/so/dylib/framework post linking copy command
d = $$setQWTPathInfo($$(QWT_PATH), $$DESTDIR)
isEmpty(d):warning("ERROR: Failed to locate QWT libs, have you compiled them and put them in the expected location")
#!macx:LIBS *= $$magic_hopsan_libpath
#!macx:INCLUDEPATH *= $$magic_hopsan_includepath

LIBS *= $$magic_hopsan_libpath
INCLUDEPATH *= $$magic_hopsan_includepath
macx:QMAKE_LFLAGS *= -lqwt

macx:message(LIBS=$$LIBS)
macx:message(INCLUDEPATH=$$INCLUDEPATH)
macx:message(QMAKE_LFLAGS=$$QMAKE_LFLAGS)

QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
#--------------------------------------------------------

#--------------------------------------------------------
# Set the PythonQt paths and dll/so post linking copy command
d = $$setPythonQtPathInfo($$(PYTHONQT_PATH), $$DESTDIR)
!isEmpty(d){
    DEFINES *= USEPYTHONQT       #If PythonQt was found then lets build GUI with PythonQt and Python support
    message(Compiling HopsanGUI with PythonQt support)
    LIBS *= $$magic_hopsan_libpath
    INCLUDEPATH *= $$magic_hopsan_includepath
    QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
} else {
    message(Compiling HopsanGUI WITHOUT PythonQt and Python support)
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set the ZMQ paths and dll/so post linking copy command
d = $$setZMQPathInfo($$(ZMQ_PATH), $$DESTDIR)
!isEmpty(d){
    DEFINES *= USEZMQ       #If ZMQ was found then lets build GUI with ZMQ / msgpack support
    message(Compiling HopsanGUI with ZeroMQ and msgpack support)
    LIBS *= $$magic_hopsan_libpath
    INCLUDEPATH *= $$magic_hopsan_includepath
    QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link

    INCLUDEPATH *= $${PWD}/../HopsanRemote/HopsanServer
    INCLUDEPATH *= $${PWD}/../HopsanRemote/HopsanServerClient
    SOURCES += $${PWD}/../HopsanRemote/HopsanServerClient/RemoteHopsanClient.cpp
    HEADERS += $${PWD}/../HopsanRemote/HopsanServerClient/RemoteHopsanClient.h

} else {
    message(Compiling HopsanGUI WITHOUT ZeroMQ and msgpack support)
}
#--------------------------------------------------------

#--------------------------------------------------------
# Set HopsanCore Paths
INCLUDEPATH *= $${PWD}/../HopsanCore/include/
LIBS *= -L$${PWD}/../bin -lHopsanCore$${DEBUG_EXT}
#--------------------------------------------------------

#--------------------------------------------------------
# Set SymHop Paths
INCLUDEPATH *= $${PWD}/../SymHop/include/
LIBS *= -L$${PWD}/../bin -lSymHop$${DEBUG_EXT}
#--------------------------------------------------------

#--------------------------------------------------------
# Set HopsanGenerator Paths
#INCLUDEPATH *= $${PWD}/../HopsanGenerator/include/
#LIBS *= -L$${PWD}/../bin -lHopsanGenerator$${DEBUG_EXT}
#--------------------------------------------------------

#--------------------------------------------------------
# Set Discount Paths
d = $$setDiscountPathInfo($$(DISCOUNT_PATH), $$DESTDIR)
!isEmpty(d){
    DEFINES *= USEDISCOUNT
    LIBS *= $$magic_hopsan_libpath
    INCLUDEPATH *= $$magic_hopsan_includepath
    QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
    message(Compiling with Discount (libmarkdown) support)
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

# -------------------------------------------------
# Platform specific additional project options
# -------------------------------------------------
unix {
    # Set Python paths
    contains(DEFINES, USEPYTHONQT) {
        message(Trying to find Python include and lib paths since USEPYTHONQT is defined)
        QMAKE_CXXFLAGS *= $$system(python$${PYTHON_VERSION}-config --includes) #TODO: Why does not include path work here
        LIBS *= $$system(python$${PYTHON_VERSION}-config --libs)
        INCLUDEPATH *= $$system(python$${PYTHON_VERSION}-config --includes)
    } else {
        message(Not looking for python since we are not using PYTHONQT)
    }


    # This will add runtime .so search paths to the executable, by using $ORIGIN these paths will be relative the executable (regardless of working dir, VERY useful)
    # The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to handle the $$ORIGIN stuff, adding manually to LFLAGS
    !macx:QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'
     macx:QMAKE_RPATHDIR *= $${PWD}/../bin

    # Get the svn revision in here if script succeed, we don't care about the external file generated,
    system($${PWD}/../getSvnRevision.sh) {
        DEFINES *= "HOPSANGUISVNREVISION=$$system($${PWD}/../getSvnRevision.sh)"
    }

}
win32 {
    #DEFINES += STATICCORE

    # Set Python paths
    contains(DEFINES, USEPYTHONQT) {
        message(Trying to find Python include and lib paths since USEPYTHONQT is defined)
        PYTHON_DEFAULT_PATHS *= c:/Python27
        PYTHON_PATH = $$selectPath($$(PYTHON_PATH), $$PYTHON_DEFAULT_PATHS, "python")
        INCLUDEPATH += $${PYTHON_PATH}/include
        LIBS += -L$${PYTHON_PATH}/libs
    } else {
        message(Not looking for python since we are not using PYTHONQT)
    }

    # Enable auto-import
    QMAKE_LFLAGS += -Wl,--enable-auto-import

    # Activate large address aware, to access more the 2GB virtual RAM (for 32-bit version)
    !contains(QMAKE_HOST.arch, x86_64){
        QMAKE_LFLAGS += -Wl,--large-address-aware
    }

    # Activate console output of cout for debug builds (you aslo need to run in console but hopsan seems slow)
    CONFIG(debug, debug|release) {
        CONFIG += console
    }

    # Get the svn revision in here if script succeed, we don't care about the external file generated,
    system($${PWD}/../getSvnRevision.bat){
        DEFINES *= "HOPSANGUISVNREVISION=$$system($${PWD}/../getSvnRevision.bat)"
    }
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
    Dialogs/ComponentPropertiesDialog.cpp \
    Dialogs/ContainerPropertiesDialog.cpp \
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
    Dialogs/ParameterSettingsLayout.cpp \
    Dialogs/ModelObjectPropertiesDialog.cpp \
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
    Optimization/OptimizationWorker.cpp \
    Optimization/OptimizationWorkerComplex.cpp \
    Optimization/OptimizationWorkerComplexRF.cpp \
    Optimization/OptimizationWorkerComplexRFM.cpp \
    Optimization/OptimizationWorkerComplexRFP.cpp \
    Optimization/OptimizationWorkerParticleSwarm.cpp \
    Optimization/OptimizationWorkerParameterSweep.cpp \
    Widgets/FindWidget.cpp \
    ModelicaLibrary.cpp \
    Widgets/ModelicaEditor.cpp \
    Widgets/PlotWidget2.cpp \
    Utilities/IndexIntervalCollection.cpp \
    LogDataGeneration.cpp



HEADERS += MainWindow.h \
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
    Dialogs/ComponentPropertiesDialog.h \
    Dialogs/ContainerPropertiesDialog.h \
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
    Dialogs/ParameterSettingsLayout.h \
    Dialogs/ModelObjectPropertiesDialog.h \
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
    Optimization/OptimizationWorker.h \
    Optimization/OptimizationWorkerComplex.h \
    Optimization/OptimizationWorkerComplexRF.h \
    Optimization/OptimizationWorkerComplexRFM.h \
    Optimization/OptimizationWorkerComplexRFP.h \
    Optimization/OptimizationWorkerParticleSwarm.h \
    Optimization/OptimizationWorkerParameterSweep.h \
    Widgets/FindWidget.h \
    ModelicaLibrary.h \
    Widgets/ModelicaEditor.h \
    GraphicsViewPort.h \
    Widgets/PlotWidget2.h \
    Utilities/IndexIntervalCollection.h \
    LogDataGeneration.h

    contains(DEFINES, USEPYTHONQT) {
        SOURCES += Widgets/PyDockWidget.cpp
        HEADERS += Widgets/PyDockWidget.h
    }

OTHER_FILES += \
    ../hopsandefaults \
    HopsanGuiBuild.prf
