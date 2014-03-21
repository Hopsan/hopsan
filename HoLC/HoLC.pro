#-------------------------------------------------
#
# Project created by QtCreator 2014-03-20T08:36:37
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HoLC
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    Widgets/ProjectFilesWidget.cpp \
    Widgets/MessageWidget.cpp \
    Widgets/EditorWidget.cpp \
    Handlers/MessageHandler.cpp \
    Handlers/FileHandler.cpp \
    Utilities/HighlightingUtilities.cpp \
    Widgets/OptionsWidget.cpp \
    Handlers/OptionsHandler.cpp \
    Utilities/CompilingUtilities.cpp

HEADERS  += MainWindow.h \
    Widgets/ProjectFilesWidget.h \
    Widgets/MessageWidget.h \
    Widgets/EditorWidget.h \
    Handlers/MessageHandler.h \
    Handlers/FileHandler.h \
    Utilities/HighlightingUtilities.h \
    Widgets/OptionsWidget.h \
    Handlers/OptionsHandler.h \
    Utilities/CompilingUtilities.h

FORMS    +=
