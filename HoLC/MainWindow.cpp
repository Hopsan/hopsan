/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#include <QDockWidget>
#include <QColor>
#include <QAction>
#include <QToolBar>
#include <QGridLayout>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QDebug>

#include "MainWindow.h"
#include "Configuration.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/EditorWidget.h"
#include "Widgets/OptionsWidget.h"
#include "Handlers/MessageHandler.h"
#include "Handlers/FileHandler.h"
#include "Dialogs/NewProjectDialog.h"
#include "Dialogs/CreateComponentWizard.h"

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
{
    qDebug() << "Testing debug!";

    //Create and load configuration
    mpConfiguration = new Configuration(this);
    mpConfiguration->loadFromXml();

    this->setStyleSheet(mpConfiguration->getStyleSheet());
    this->setPalette(mpConfiguration->getPalette());
    this->setWindowIcon(QIcon(":graphics/uiicons/Holc-Icon.png"));

    QFont font = qApp->font();
    font.setPixelSize(12);
    qApp->setFont(font);

    // Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width

    //Create dock widgets
    QDockWidget *pFilesDockWidget = new QDockWidget("Project Files", this);
    QDockWidget *pMessageDockWidget = new QDockWidget("Messages", this);

    //Add dock widgets
    this->addDockWidget(Qt::LeftDockWidgetArea, pFilesDockWidget);
    this->addDockWidget(Qt::BottomDockWidgetArea, pMessageDockWidget);

    //Create widgets
    mpEditorWidget = new EditorWidget(mpConfiguration, this);
    mpProjectFilesWidget = new ProjectFilesWidget(this);
    mpMessageWidget = new MessageWidget(this);
    mpOptionsWidget = new OptionsWidget(mpConfiguration, this);

    //Assign widgets to areas
    QWidget *pCentralWidget = new QWidget(this);
    QGridLayout *pCentralLayout = new QGridLayout(pCentralWidget);
    pCentralLayout->addWidget(mpEditorWidget,0,0);
    pCentralLayout->addWidget(mpOptionsWidget,0,0);
    mpOptionsWidget->setHidden(true);
    setCentralWidget(pCentralWidget);
    pFilesDockWidget->setWidget(mpProjectFilesWidget);
    pMessageDockWidget->setWidget(mpMessageWidget);

    //Create toolbars
    QAction *pNewAction = new QAction("New Library", this);
    pNewAction->setIcon(QIcon(":graphics/uiicons/Hopsan-New.png"));
    QAction *pOpenAction = new QAction("Open Library", this);
    pOpenAction->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));
    QAction *pSaveAction = new QAction("Save Current File", this);
    pSaveAction->setIcon(QIcon(":graphics/uiicons/Hopsan-Save.png"));
    pSaveAction->setShortcut(QKeySequence("Ctrl+S"));
    QAction *pAddComponentAction = new QAction("Add New Component", this);
    pAddComponentAction->setIcon(QIcon(":graphics/uiicons/Hopsan-Add.png"));
    QAction *pAddComponentFromFileAction = new QAction("Add Existing Component Source File", this);
    pAddComponentFromFileAction->setIcon(QIcon(":graphics/uiicons/Hopsan-AddFromFile.png"));
    QAction *pAddCafFromFileAction = new QAction("Add Existing Appearance File", this);
    pAddCafFromFileAction->setIcon(QIcon(":graphics/uiicons/Hopsan-AddCafFile.png"));
    QAction *pOptionsAction = new QAction("Options", this);
    pOptionsAction->setIcon(QIcon(":graphics/uiicons/Hopsan-Options.png"));
    pOptionsAction->setCheckable(true);
    QAction *pCompileAction = new QAction("Compile Library", this);
    pCompileAction->setIcon(QIcon(":graphics/uiicons/Hopsan-Compile.png"));
    QAction *pDebugAction = new QAction("Debug", this);
    pDebugAction->setShortcut(QKeySequence("Ctrl+D"));

    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pNewAction);
    pToolBar->addAction(pOpenAction);
    pToolBar->addAction(pSaveAction);
    pToolBar->addAction(pAddComponentAction);
    pToolBar->addAction(pAddComponentFromFileAction);
    pToolBar->addAction(pAddCafFromFileAction);
    pToolBar->addAction(pOptionsAction);
    pToolBar->addAction(pCompileAction);
    pToolBar->addAction(pDebugAction);
    this->addToolBar(pToolBar);

    //Create handlers
    mpMessageHandler = new MessageHandler(mpMessageWidget);
    mpFileHandler = new FileHandler(mpConfiguration, mpProjectFilesWidget, mpEditorWidget, mpMessageHandler);

    //Create dialogs
    mpNewProjectDialog = new NewProjectDialog(mpFileHandler, this);
    mpNewProjectDialog->hide();
    mpCreateComponentWizard = new CreateComponentWizard(mpFileHandler, mpMessageHandler, this);
    mpCreateComponentWizard->hide();

    //Setup connections
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpFileHandler,              SLOT(updateText()));
    connect(pOpenAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(loadFromXml()));
    connect(pSaveAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(saveToXml()));
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpFileHandler,              SLOT(setFileNotSaved()));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpOptionsWidget,            SLOT(setVisible(bool)));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpEditorWidget,             SLOT(setHidden(bool)));
    connect(pCompileAction,                 SIGNAL(triggered()),        mpFileHandler,              SLOT(compileLibrary()));
    connect(mpFileHandler,                  SIGNAL(fileOpened(bool)),   pOptionsAction,             SLOT(setChecked(bool)));
    connect(pNewAction,                     SIGNAL(triggered()),        mpNewProjectDialog,         SLOT(show()));
    connect(pAddComponentAction,            SIGNAL(triggered()),        mpCreateComponentWizard,    SLOT(open()));
    connect(pAddComponentFromFileAction,    SIGNAL(triggered()),        mpFileHandler,              SLOT(addComponent()));
    connect(pAddCafFromFileAction,          SIGNAL(triggered()),        mpFileHandler,              SLOT(addAppearanceFile()));
    connect(pDebugAction,                   SIGNAL(triggered()),        mpEditorWidget,             SLOT(generateAutoCompleteList()));

    //Load last session project (if exists)
    if(!mpConfiguration->getProjectPath().isEmpty())
    {
        mpFileHandler->loadFromXml(mpConfiguration->getProjectPath());
    }
}

MainWindow::~MainWindow()
{
    //! @todo Is it really needed to save config here? It is saved every time config is changed anyway...
    mpConfiguration->saveToXml();
}
