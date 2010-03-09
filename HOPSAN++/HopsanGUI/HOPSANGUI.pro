# -------------------------------------------------
# Project created by QtCreator 2009-12-28T14:27:59
# -------------------------------------------------
QT += svg
TARGET = HOPSANGUI
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    listwidgetitem.cpp \
    ProjectTabWidget.cpp \
    LibraryWidget.cpp \
    GUIConnector.cpp \
    GUIPort.cpp \
    GUIComponent.cpp \
    GUIConnectorLine.cpp \
    plotwidget.cpp \
    GUIComponentSelectionBox.cpp \
    ParameterDialog.cpp \
    MessageWidget.cpp \
    SimulationSetupWidget.cpp \
    SimulationThread.cpp \
    InitializationThread.cpp
HEADERS += mainwindow.h \
    listwidgetitem.h \
    ProjectTabWidget.h \
    LibraryWidget.h \
    ../HopsanCore/HopsanCore.h \
    GUIConnector.h \
    GUIPort.h \
    GUIComponent.h \
    GUIConnectorLine.h \
    plotwidget.h \
    GUIComponentSelectionBox.h \
    ParameterDialog.h \
    MessageWidget.h \
    SimulationSetupWidget.h \
    SimulationThread.h \
    InitializationThread.h
OTHER_FILES += 

# win32:DEFINES += STATICCORE
DESTDIR = ../bin/debug
LIBS += -L../bin/debug \
    -lHopsanCore
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
    
    # make install path ("c:\Qwt-5.2.1-svn\lib" need to be added in windows PATH env variable)
    INCLUDEPATH += c:\Qwt-5.2.1-svn\include
    LIBS += -Lc:\Qwt-5.2.1-svn\lib
    LIBS += -lqwtd5
}
RESOURCES += 
