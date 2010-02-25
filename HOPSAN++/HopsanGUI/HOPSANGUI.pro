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
    plotwidget.cpp
HEADERS += mainwindow.h \
    listwidgetitem.h \
    ProjectTabWidget.h \
    LibraryWidget.h \
    ../HopsanCore/HopsanCore.h \
    GUIConnector.h \
    GUIPort.h \
    GUIComponent.h \
    GUIConnectorLine.h \
    plotwidget.h
OTHER_FILES +=

# win32:DEFINES += STATICCORE
DESTDIR = ../bin/debug

unix:LIBS += -L../HopsanCore/bin/debug -lHopsanCore -Wl,-rpath,../../../HopsanCore/bin/debug

unix:LIBS += -lqwt-qt4
#-L/usr/share/doc/libqwt5-qt4 \

unix:INCLUDEPATH += /usr/include/qwt-qt4/

LIBS += -L../bin/debug \
    -lHopsanCore

INCLUDEPATH += ../HopsanCore


#Ingopath:
win32:INCLUDEPATH += c:/temp_qwt/src
win32:LIBS += c:/temp_qwt/lib/qwtd5.dll
