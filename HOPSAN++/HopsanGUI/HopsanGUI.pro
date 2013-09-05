# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
# -------------------------------------------------
# Global project options
# -------------------------------------------------
include( HopsanGuiBuild.prf )

TARGET = HopsanGUI
TEMPLATE = app
DESTDIR = $${PWD}/../bin

QT += svg xml
QT += core gui webkit network


TARGET = $${TARGET}$${DEBUG_EXT}

#--------------------------------------------------------
# Set the QWT paths and dll/so post linking copy command
d = $$setQWTPathInfo($$(QWT_PATH), $$DESTDIR)
isEmpty(d):error('Failed to find QWT libs, have you compiled them and put them in the expected location')
LIBS *= $$magic_hopsan_libpath
INCLUDEPATH *= $$magic_hopsan_includepath
QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
#--------------------------------------------------------

#--------------------------------------------------------
# Set the PythonQt paths and dll/so post linking copy command
d = $$setPythonQtPathInfo($$(PYTHONQT_PATH), $$DESTDIR)
isEmpty(d):error('Failed to find PythonQt libs, have you compiled them and put them in the expected location')
LIBS *= $$magic_hopsan_libpath
INCLUDEPATH *= $$magic_hopsan_includepath
QMAKE_POST_LINK *= $$magic_hopsan_qmake_post_link
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
    QMAKE_CXXFLAGS *= $$system(python$${PYTHON_VERSION}-config --includes) #TODO: Why does not include path work here
    LIBS *= $$system(python$${PYTHON_VERSION}-config --libs)
    INCLUDEPATH *= $$system(python$${PYTHON_VERSION}-config --includes)
    #This will add runtime so search paths to the executable, by using $ORIGIN these paths will be realtive the executable (regardless of working dir, VERY useful)
    #The QMAKE_LFLAGS_RPATH and QMAKE_RPATHDIR does not seem to be able to hande the $$ORIGIN stuff, adding manually to LFLAGS
    # TODO: We need to add teh relative paths automatically from the path variables created above
    #QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/../lib\'
    #QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/Dependencies/qwt-6.0.0/lib\'
    #QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/Dependencies/PythonQt2.0.1/lib\'
    QMAKE_LFLAGS *= -Wl,-rpath,\'\$$ORIGIN/./\'

    #Get the svn revision in here if script succeed, we dont care about the external file generated,
    system($${PWD}/../getSvnRevision.sh) {
        DEFINES *= "HOPSANGUISVNREVISION=\"\\\"$$system($${PWD}/../getSvnRevision.sh)\\\"\""
    }

}
win32 {
    #DEFINES += STATICCORE

    #Set Python paths
    PYTHON_DEFAULT_PATHS *= c:/Python27
    PYTHON_DEFAULT_PATHS *= c:/Python26
    PYTHON_PATH = $$selectPath($$(PYTHON_PATH), $$PYTHON_DEFAULT_PATHS, "python")
    INCLUDEPATH += $${PYTHON_PATH}/include
    LIBS += -L$${PYTHON_PATH}/libs

    #Activate large adress aware, to access more the 2GB virtual RAM (for 32-bit version)
    #Also enable auto-import
    QMAKE_LFLAGS += -Wl,--large-address-aware,--enable-auto-import

    CONFIG(debug, debug|release) {
        CONFIG += console #Use this for consol app support (cout output, you aslo need to run in consol but hopsan seems slow)
    }

    #Get the svn revision in here if script succeed, we dont care about the external file generated,
    system($${PWD}/../getSvnRevision.bat){
        DEFINES *= "HOPSANGUISVNREVISION=\"\\\"$$system($${PWD}/../getSvnRevision.bat)\\\"\""
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
    Widgets/LibraryWidget.cpp \
    GUIConnector.cpp \
    GUIPort.cpp \
    Widgets/PlotWidget.cpp \
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
    Widgets/PyDockWidget.cpp \
    GUIObjects/GUIWidgets.cpp \
    GUIObjects/GUISystem.cpp \
    GUIObjects/GUIObject.cpp \
    GUIObjects/GUIModelObjectAppearance.cpp \
    GUIObjects/GUIModelObject.cpp \
    GUIObjects/GUIGroup.cpp \
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
    Dialogs/WelcomeDialog.cpp \
    Dialogs/HelpDialog.cpp \
    Dependencies/BarChartPlotter/plotterbase.cpp \
    Dependencies/BarChartPlotter/barchartplotter.cpp \
    Dependencies/BarChartPlotter/axisbase.cpp \
    Dialogs/OptimizationDialog.cpp \
    Dialogs/SensitivityAnalysisDialog.cpp \
    Dialogs/ComponentGeneratorDialog.cpp \
    Dialogs/MovePortsDialog.cpp \
    loadFunctions.cpp \
    Utilities/ComponentGeneratorUtilities.cpp \
    Widgets/WelcomeWidget.cpp \
    Widgets/AnimationWidget.cpp \
    GUIObjects/AnimatedComponent.cpp \
    AnimatedConnector.cpp \
    Dialogs/AnimatedIconPropertiesDialog.cpp \
    Dialogs/ParameterSettingsLayout.cpp \
    Dialogs/ModelObjectPropertiesDialog.cpp \
    SimulationThreadHandler.cpp \
    Widgets/HcomWidget.cpp \
    LogDataHandler.cpp \
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
    Utilities/HighlightingUtilities.cpp



HEADERS += MainWindow.h \
    Widgets/ProjectTabWidget.h \
    Widgets/LibraryWidget.h \
    GUIConnector.h \
    GUIPort.h \
    Widgets/PlotWidget.h \
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
    Widgets/PyDockWidget.h \
    GUIObjects/GUIWidgets.h \
    GUIObjects/GUISystem.h \
    GUIObjects/GUIObject.h \
    GUIObjects/GUIModelObjectAppearance.h \
    GUIObjects/GUIModelObject.h \
    GUIObjects/GUIGroup.h \
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
    Dialogs/WelcomeDialog.h \
    Dialogs/HelpDialog.h \
    Dependencies/BarChartPlotter/plotterbase.h \
    Dependencies/BarChartPlotter/barchartplotter.h \
    Dependencies/BarChartPlotter/axisbase.h \
    Dialogs/OptimizationDialog.h \
    Dialogs/SensitivityAnalysisDialog.h \
    Dialogs/ComponentGeneratorDialog.h \
    Dialogs/MovePortsDialog.h \
    loadFunctions.h \
    Utilities/ComponentGeneratorUtilities.h \
    Widgets/WelcomeWidget.h \
    Widgets/AnimationWidget.h \
    GUIObjects/AnimatedComponent.h \
    AnimatedConnector.h \
    Dialogs/AnimatedIconPropertiesDialog.h \
    Dialogs/ParameterSettingsLayout.h \
    Dialogs/ModelObjectPropertiesDialog.h \
    SimulationThreadHandler.h \
    Widgets/HcomWidget.h \
    LogDataHandler.h \
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
    Utilities/HighlightingUtilities.h

OTHER_FILES += \
    ../hopsandefaults \
    HopsanGuiBuild.prf
