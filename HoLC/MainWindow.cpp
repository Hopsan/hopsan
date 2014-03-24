#include <QDockWidget>
#include <QColor>
#include <QAction>
#include <QToolBar>
#include <QGridLayout>
#include <QMenu>

#include "MainWindow.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/EditorWidget.h"
#include "Widgets/OptionsWidget.h"
#include "Handlers/MessageHandler.h"
#include "Handlers/FileHandler.h"
#include "Handlers/OptionsHandler.h"

MainWindow::MainWindow(QWidget *pParent)
    : QMainWindow(pParent)
{
    this->resize(800,600);

    //Create dock widgets
    QDockWidget *pFilesDockWidget = new QDockWidget("Project Files", this);
    QDockWidget *pMessageDockWidget = new QDockWidget("Messages", this);

    //Add dock widgets
    this->addDockWidget(Qt::LeftDockWidgetArea, pFilesDockWidget);
    this->addDockWidget(Qt::BottomDockWidgetArea, pMessageDockWidget);

    mpOptionsHandler = new OptionsHandler();

    //Create widgets
    mpEditorWidget = new EditorWidget(this);
    mpProjectFilesWidget = new ProjectFilesWidget(this);
    mpMessageWidget = new MessageWidget(this);
    mpOptionsWidget = new OptionsWidget(mpOptionsHandler, this);

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
    pNewAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-New.png"));
    QAction *pOpenAction = new QAction("Open Library", this);
    pOpenAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-Open.png"));
    QAction *pSaveAction = new QAction("Save Current File", this);
    pSaveAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-Save.png"));
    QAction *pAddComponentAction = new QAction("Add New Component", this);
    pAddComponentAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-Add.png"));
    QAction *pOptionsAction = new QAction("Options", this);
    pOptionsAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-Options.png"));
    pOptionsAction->setCheckable(true);
    QAction *pCompileAction = new QAction("Compile Library", this);
    pCompileAction->setIcon(QIcon("../HopsanGUI/graphics/uiicons/Hopsan-Compile.png"));

    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pNewAction);
    pToolBar->addAction(pOpenAction);
    pToolBar->addAction(pSaveAction);
    pToolBar->addAction(pAddComponentAction);
    pToolBar->addAction(pOptionsAction);
    pToolBar->addAction(pCompileAction);
    this->addToolBar(pToolBar);

    //Create handlers
    mpMessageHandler = new MessageHandler(mpMessageWidget);
    mpFileHandler = new FileHandler(mpProjectFilesWidget, mpEditorWidget, mpMessageHandler, mpOptionsHandler);

    //Setup connetions
    connect(mpEditorWidget, SIGNAL(textChanged()),    mpFileHandler,          SLOT(updateText()));
    connect(pOpenAction,    SIGNAL(triggered()),      mpFileHandler,          SLOT(loadFromXml()));
    connect(pSaveAction,    SIGNAL(triggered()),      mpFileHandler,          SLOT(saveToXml()));
    connect(mpEditorWidget, SIGNAL(textChanged()),    mpProjectFilesWidget,   SLOT(addAsterisk()));
    connect(pOptionsAction, SIGNAL(toggled(bool)),    mpOptionsWidget,        SLOT(setVisible(bool)));
    connect(pOptionsAction, SIGNAL(toggled(bool)),    mpEditorWidget,         SLOT(setHidden(bool)));
    connect(pCompileAction, SIGNAL(triggered()),      mpFileHandler,          SLOT(compileLibrary()));
    connect(mpFileHandler,  SIGNAL(fileOpened(bool)), pOptionsAction,       SLOT(setChecked(bool)));

    //Debug
    QString msg = "Test message!";
    mpMessageHandler->addInfoMessage(msg);
    mpMessageHandler->addWarningMessage(msg);
    mpMessageHandler->addErrorMessage(msg);
}

MainWindow::~MainWindow()
{
}

