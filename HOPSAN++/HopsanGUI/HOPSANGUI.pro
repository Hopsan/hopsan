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
    ParameterDialog.cpp \
    MessageWidget.cpp \
    SimulationThread.cpp \
    InitializationThread.cpp \
    PreferenceWidget.cpp \
    OptionsWidget.cpp \
    GUIObject.cpp \
    UndoStack.cpp \
    AppearanceData.cpp \
    GUIUtilities.cpp \
    GraphicsView.cpp \
    GraphicsScene.cpp \
    loadObjects.cpp \
    ProgressBarThread.cpp \
    GUISystem.cpp \
    CoreSystemAccess.cpp \
    GUIPortAppearance.cpp \
    GUIConnectorAppearance.cpp \
    GlobalParametersWidget.cpp
HEADERS += MainWindow.h \
    ProjectTabWidget.h \
    LibraryWidget.h \
    ../HopsanCore/HopsanCore.h \
    GUIConnector.h \
    GUIPort.h \
    PlotWidget.h \
    ParameterDialog.h \
    MessageWidget.h \
    SimulationThread.h \
    InitializationThread.h \
    version.h \
    PreferenceWidget.h \
    OptionsWidget.h \
    GUIObject.h \
    AppearanceData.h \
    UndoStack.h \
    GUIUtilities.h \
    CoreSystemAccess.h \
    GraphicsView.h \
    GraphicsScene.h \
    loadObjects.h \
    ProgressBarThread.h \
    common.h \
    GUISystem.h \
    CoreSystemAccess.h \
    GUIPortAppearance.h \
    GUIConnectorAppearance.h \
    GlobalParametersWidget.h
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

INCLUDEPATH += ../HopsanCore
unix {
    LIBS += -Wl,-rpath,./
    LIBS += -lqwt-qt4
    INCLUDEPATH += /usr/include/qwt-qt4/
}
win32 {
    # Ingopath:
    INCLUDEPATH += c:/temp_qwt/src
    LIBS += -Lc:/temp_qwt/lib

    INCLUDEPATH += c:/Qwt-5.2.1-svn/include
    LIBS += -Lc:/Qwt-5.2.1-svn/lib

    INCLUDEPATH += c:/Qwt-5.2.1/include
    LIBS += -Lc:/Qwt-5.2.1/lib

    CONFIG(debug, debug|release) {
        LIBS += -lqwtd5
    }
    CONFIG(release, debug|release) {
        LIBS += -lqwt5
    }
}
RESOURCES += 
