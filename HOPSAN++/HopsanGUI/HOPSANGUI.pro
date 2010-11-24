# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
QT += svg xml
TARGET = HOPSANGUI
TEMPLATE = app
SOURCES += main.cpp \
    MainWindow.cpp \
    ProjectTabWidget.cpp \
    LibraryWidget.cpp \
    GUIConnector.cpp \
    GUIPort.cpp \
    PlotWidget.cpp \
    MessageWidget.cpp \
    SimulationThread.cpp \
    InitializationThread.cpp \
    OptionsDialog.cpp \
    UndoStack.cpp \
    GraphicsView.cpp \
    GraphicsScene.cpp \
    loadObjects.cpp \
    ProgressBarThread.cpp \
    CoreSystemAccess.cpp \
    GUIPortAppearance.cpp \
    GUIConnectorAppearance.cpp \
    GlobalParametersWidget.cpp \
    PlotWindow.cpp \
    PyWrapperClasses.cpp \
    PyDock.cpp \
    GUIObjects/GUIWidgets.cpp \
    GUIObjects/GUISystemPort.cpp \
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
    ComponentPropertiesDialog.cpp \
    ContainerPropertiesDialog.cpp

HEADERS += MainWindow.h \
    ProjectTabWidget.h \
    LibraryWidget.h \
    ../HopsanCore/HopsanCore.h \
    GUIConnector.h \
    GUIPort.h \
    PlotWidget.h \
    MessageWidget.h \
    SimulationThread.h \
    InitializationThread.h \
    version.h \
    OptionsDialog.h \
    UndoStack.h \
    CoreSystemAccess.h \
    GraphicsView.h \
    GraphicsScene.h \
    loadObjects.h \
    ProgressBarThread.h \
    common.h \
    CoreSystemAccess.h \
    GUIPortAppearance.h \
    GUIConnectorAppearance.h \
    GlobalParametersWidget.h \
    PlotWindow.h \
    PyWrapperClasses.h \
    PyDock.h \
    GUIObjects/GUIWidgets.h \
    GUIObjects/GUISystemPort.h \
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
    ComponentPropertiesDialog.h \
    ContainerPropertiesDialog.h

OTHER_FILES += 

# win32:DEFINES += STATICCORE
CONFIG(debug, debug|release) {
    DESTDIR = ../bin/debug
    LIBS += -L../bin/debug \
        -lHopsanCore
}
CONFIG(release, debug|release) {
    DESTDIR = ../bin/release
    LIBS += -L../bin/release \
        -lHopsanCore
}

#Define a parameter PYTHONQT_PATH e.g. '/home/apako69/pythonqt' in the project settings, also add '/home/apako69/pythonqt/lib' to LD_LIBRARY_PATH on *nix.
CONFIG(release, debug|release) {
    LIBS += -L$(PYTHONQT_PATH)/lib -lPythonQt \
                                   -lPythonQt_QtAll
}
CONFIG(debug, debug|release) {
    LIBS += -L$(PYTHONQT_PATH)/lib -lPythonQt_d \
                                   -lPythonQt_QtAll_d
}

INCLUDEPATH += $(PYTHONQT_PATH)/src \
               $(PYTHONQT_PATH)/extensions/PythonQt_QtAll

INCLUDEPATH += ../HopsanCore
unix {
    LIBS += -Wl,-rpath,./
    LIBS += -lqwt-qt4
    INCLUDEPATH += /usr/include/qwt-qt4/
    INCLUDEPATH += /usr/include/python2.6

    LIBS += $$system(python$${PYTHON_VERSION}-config --libs)

    QMAKE_CXXFLAGS += $$system(python$${PYTHON_VERSION}-config --includes)
}
win32 {
    INCLUDEPATH += c:/temp_qwt/src
    INCLUDEPATH += c:/temp_qwt/include
    LIBS += -Lc:/temp_qwt/lib

    INCLUDEPATH += c:/Qwt-5.2.2-svn/include
    INCLUDEPATH += c:/Qwt-5.2.1-svn/include
    LIBS += -Lc:/Qwt-5.2.2-svn/lib
    LIBS += -Lc:/Qwt-5.2.1-svn/lib

    CONFIG(debug, debug|release) {
        LIBS += -lqwtd5
    }
    CONFIG(release, debug|release) {
        LIBS += -lqwt5
    }
    INCLUDEPATH += $(PYTHON_PATH)/include
    LIBS += -L$(PYTHON_PATH)/libs
}
RESOURCES += 
