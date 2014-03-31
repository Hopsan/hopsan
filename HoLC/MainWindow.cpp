#include <QDockWidget>
#include <QColor>
#include <QAction>
#include <QToolBar>
#include <QGridLayout>
#include <QMenu>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeySequence>

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
    //Create and load configuration
    mpConfiguration = new Configuration(this);
    mpConfiguration->loadFromXml();

    this->setStyleSheet(mpConfiguration->getStyleSheet());
    this->setPalette(mpConfiguration->getPalette());

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
    mpEditorWidget = new EditorWidget(this);
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
    pNewAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-New.png"));
    QAction *pOpenAction = new QAction("Open Library", this);
    pOpenAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Open.png"));
    QAction *pSaveAction = new QAction("Save Current File", this);
    pSaveAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Save.png"));
    pSaveAction->setShortcut(QKeySequence("Ctrl+S"));
    QAction *pAddComponentAction = new QAction("Add New Component", this);
    pAddComponentAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Add.png"));
    QAction *pAddComponentFromFileAction = new QAction("Add Existing Component Source File", this);
    pAddComponentFromFileAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-AddFromFile.png"));
    QAction *pAddCafFromFileAction = new QAction("Add Existing Appearance File", this);
    pAddCafFromFileAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-AddCafFile.png"));
    QAction *pOptionsAction = new QAction("Options", this);
    pOptionsAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Options.png"));
    pOptionsAction->setCheckable(true);
    QAction *pCompileAction = new QAction("Compile Library", this);
    pCompileAction->setIcon(QIcon(":/gfx/graphics/uiicons/Hopsan-Compile.png"));

    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pNewAction);
    pToolBar->addAction(pOpenAction);
    pToolBar->addAction(pSaveAction);
    pToolBar->addAction(pAddComponentAction);
    pToolBar->addAction(pAddComponentFromFileAction);
    pToolBar->addAction(pAddCafFromFileAction);
    pToolBar->addAction(pOptionsAction);
    pToolBar->addAction(pCompileAction);
    this->addToolBar(pToolBar);

    //Create handlers
    mpMessageHandler = new MessageHandler(mpMessageWidget);
    mpFileHandler = new FileHandler(mpConfiguration, mpProjectFilesWidget, mpEditorWidget, mpMessageHandler);

    //Create dialogs
    mpNewProjectDialog = new NewProjectDialog(mpFileHandler, this);
    mpNewProjectDialog->hide();
    mpCreateComponentWizard = new CreateComponentWizard(mpFileHandler, mpMessageHandler, this);
    mpCreateComponentWizard->hide();

    //Setup connetions
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpFileHandler,              SLOT(updateText()));
    connect(pOpenAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(loadFromXml()));
    connect(pSaveAction,                    SIGNAL(triggered()),        mpFileHandler,              SLOT(saveToXml()));
    connect(mpEditorWidget,                 SIGNAL(textChanged()),      mpProjectFilesWidget,       SLOT(addAsterisk()));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpOptionsWidget,            SLOT(setVisible(bool)));
    connect(pOptionsAction,                 SIGNAL(toggled(bool)),      mpEditorWidget,             SLOT(setHidden(bool)));
    connect(pCompileAction,                 SIGNAL(triggered()),        mpFileHandler,              SLOT(compileLibrary()));
    connect(mpFileHandler,                  SIGNAL(fileOpened(bool)),   pOptionsAction,             SLOT(setChecked(bool)));
    connect(pNewAction,                     SIGNAL(triggered()),        mpNewProjectDialog,         SLOT(show()));
    connect(pAddComponentAction,            SIGNAL(triggered()),        mpCreateComponentWizard,    SLOT(open()));
    connect(pAddComponentFromFileAction,    SIGNAL(triggered()),        mpFileHandler,              SLOT(addComponent()));
    connect(pAddCafFromFileAction,          SIGNAL(triggered()),        mpFileHandler,              SLOT(addAppearanceFile()));

    //Load last session project (if exists)
    if(!mpConfiguration->getProjectPath().isEmpty())
    {
        mpFileHandler->loadFromXml(mpConfiguration->getProjectPath());
    }
}

MainWindow::~MainWindow()
{
    mpConfiguration->saveToXml();
}
