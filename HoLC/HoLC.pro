#-------------------------------------------------
#
# Project created by QtCreator 2014-03-20T08:36:37
#
#-------------------------------------------------
include( ../Common.prf )

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG -= app_bundle

TARGET = holc
TEMPLATE = app
DESTDIR = $${PWD}/../bin

INCLUDEPATH *= $${PWD}/
LIBS *= -L$${PWD}/../bin

SOURCES += main.cpp\
    MainWindow.cpp \
    Widgets/ProjectFilesWidget.cpp \
    Widgets/MessageWidget.cpp \
    Widgets/EditorWidget.cpp \
    Handlers/MessageHandler.cpp \
    Handlers/FileHandler.cpp \
    Utilities/HighlightingUtilities.cpp \
    Widgets/OptionsWidget.cpp \
    Utilities/CompilingUtilities.cpp \
    Dialogs/NewProjectDialog.cpp \
    Dialogs/CreateComponentWizard.cpp \
    Configuration.cpp \
    Utilities/XMLUtilities.cpp \
    Utilities/StringUtilities.cpp \
    Widgets/TextEditor.cpp \
    Widgets/FindWidget.cpp

HEADERS  += MainWindow.h \
    Widgets/ProjectFilesWidget.h \
    Widgets/MessageWidget.h \
    Widgets/EditorWidget.h \
    Handlers/MessageHandler.h \
    Handlers/FileHandler.h \
    Utilities/HighlightingUtilities.h \
    Widgets/OptionsWidget.h \
    Utilities/CompilingUtilities.h \
    Dialogs/NewProjectDialog.h \
    Dialogs/CreateComponentWizard.h \
    Configuration.h \
    Utilities/XMLUtilities.h \
    Utilities/StringUtilities.h \
    Widgets/TextEditor.h \
    Widgets/FindWidget.h

FORMS    +=

RESOURCES += \
    icons.qrc \
    templates.qrc

RC_FILE = HoLC.rc

OTHER_FILES += \
    holcdefaults
