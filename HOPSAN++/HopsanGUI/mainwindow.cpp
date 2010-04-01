//$Id$

//#include <QtGui/QFileDialog>
//#include <QtCore/QTextStream>
//#include <QDebug>
//
//#include <QtGui/QMainWindow>
//#include <QtGui/QWidget>
//#include <QtGui/QGridLayout>
//#include <QtGui/QTabWidget>
//#include <QtGui/QTreeWidget>
//#include <QtGui/QTreeWidgetItem>
//#include <QtGui/QMenuBar>
//#include <QtGui/QMenu>
//#include <QtGui/QStatusBar>
//#include <QtGui/QAction>
//#include <QtCore/QMetaObject>
//#include <QtCore/QString>
//#include <QtCore/QDir>
//#include <QtCore/QStringList>
//#include <QtCore/QIODevice>
//#include <QListWidgetItem>
//#include <QStringList>
//#include <QDockWidget>

#include <iostream>
#include <QtGui>

#include "mainwindow.h"
#include "treewidget.h"
#include "treewidgetitem.h"
#include "listwidget.h"
#include "listwidgetitem.h"
#include "ProjectTabWidget.h"
#include "LibraryWidget.h"
#include "SimulationSetupWidget.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    this->resize(1024,768);
    this->setWindowTitle("HOPSAN NG");
    this->setWindowIcon(QIcon("../../HopsanGUI/icons/hopsan.ico"));

    mpPreferenceWidget = new PreferenceWidget(this);

    //Create a centralwidget for the main window
    mpCentralwidget = new QWidget(this);
    mpCentralwidget->setObjectName("centralwidget");

    //Create a grid on the centralwidget
    mpCentralgrid = new QGridLayout(mpCentralwidget);
    mpCentralgrid->setSpacing(10);

    //Create a dock for the MessageWidget
    QDockWidget *messagedock = new QDockWidget(tr("Messages"), this);
    messagedock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageWidget->setReadOnly(true);
    messagedock->setWidget(mpMessageWidget);
    addDockWidget(Qt::BottomDockWidgetArea, messagedock);
    mpMessageWidget->printGUIMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    //Create a dock for the componentslibrary
    QDockWidget *libdock = new QDockWidget(tr("Components"), this);
    libdock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    libdock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, libdock);

    //Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    //setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    //Create a SimulationSetupWidget
    mpSimulationSetupWidget = new SimulationSetupWidget(tr("Simulation setup"), this);

    //Create the main tab container, need at least one tab
    mpProjectTabs = new ProjectTabWidget(this);
    mpProjectTabs->setObjectName("projectTabs");

    //mpCentralgrid->addWidget(mpSimulationSetupWidget,0,0);
    mpCentralgrid->addWidget(mpProjectTabs,0,0);

    mpCentralwidget->setLayout(mpCentralgrid);

    //Set the centralwidget
    this->setCentralWidget(mpCentralwidget);

    mpProjectTabs->addNewProjectTab();

    //Create the menubar
    menubar = new QMenuBar();
    menubar->setGeometry(QRect(0,0,800,25));
    menubar->setObjectName("menubar");

    //Create the menues
    menuFile = new QMenu(menubar);
    menuFile->setObjectName("menuFile");
    menuFile->setTitle("&File");

    menuNew = new QMenu(menubar);
    menuNew->setObjectName("menuNew");
    menuNew->setTitle("New");

    menuLibs = new QMenu(menubar);
    menuLibs->setObjectName("menuLibs");
    menuLibs->setTitle("&Libraries");

    menuSimulation = new QMenu(menubar);
    menuSimulation->setObjectName("menuSimulation");
    menuSimulation->setTitle("&Simulation");

    menuView = new QMenu(menubar);
    menuView->setTitle("&View");

    menuPlot = new QMenu(menubar);
    menuPlot->setTitle("&Plot");

    this->setMenuBar(menubar);

    //Create the Statusbar
    statusBar = new QStatusBar();
    statusBar->setObjectName("statusBar");
    this->setStatusBar(statusBar);

    //Create the actionsbuttons
    actionOpen = new QAction(this);
    actionOpen->setText("Open");
    actionOpen->setShortcut(QKeySequence("Ctrl+o"));

    actionClose = new QAction(this);
    actionClose->setText("Close");
    actionClose->setShortcut(QKeySequence("Ctrl+q"));

    actionSave = new QAction(this);
    actionSave->setText("Save");
    actionSave->setShortcut(QKeySequence("Ctrl+s"));

    actionSaveAs = new QAction(this);
    actionSaveAs->setText("Save As");
    actionSaveAs->setShortcut(QKeySequence("Ctrl+Alt+s"));

    actionPreferences = new QAction(this);
    actionPreferences->setText("Model Preferences");
    actionPreferences->setShortcut(QKeySequence("Ctrl+Alt+p"));

    actionProject = new QAction(this);
    actionProject->setText("Project");

    actionLoadLibs = new QAction(this);
    actionLoadLibs->setText("Load Libraries");

    actionSimulate = new QAction(this);
    actionSimulate->setText("Simulate");
    actionSimulate->setShortcut(Qt::Key_F9);

    actionPlot = new QAction(this);
    actionPlot->setText("Plot");


    this->createActions();
    this->createToolbars();

    //Add the actionbuttons to the menues
    menuNew->addAction(actionProject);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(actionOpen);
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    menuFile->addSeparator();
    menuFile->addAction(actionPreferences);
    menuFile->addSeparator();
    menuFile->addAction(actionClose);

    menuLibs->addAction(actionLoadLibs);
    //menuLibs->addAction(actionOpen);

    menuSimulation->addAction(actionSimulate);

    menuView->addAction(libdock->toggleViewAction());
    menuView->addAction(messagedock->toggleViewAction());
    menuView->addAction(fileToolBar->toggleViewAction());
    menuView->addAction(clipboardToolBar->toggleViewAction());
    menuView->addAction(simToolBar->toggleViewAction());

    menuPlot->addAction(actionPlot);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuLibs->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuPlot->menuAction());

    //Load default libraries
    mpLibrary->addEmptyLibrary("User defined libraries");

    mpLibrary->addEmptyLibrary("Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/sources","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/sensors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/restrictors","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/volumes","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/actuators","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/valves","Hydraulic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/hydraulic/pumps","Hydraulic");

    mpLibrary->addEmptyLibrary("Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Sources","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Sinks","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Arithmetics","Signal");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/signal/Non-Linearities","Signal");

    mpLibrary->addEmptyLibrary("Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Transformers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Mass Loads","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Springs & Dampers","Mechanic");
    mpLibrary->addLibrary("../../HopsanGUI/componentData/mechanic/Sensors","Mechanic");

    mpLibrary->addEmptyLibrary("SubSystem");
    QIcon icon;
    LibraryContentItem *pLibsubcomp = new LibraryContentItem(icon,"SubSystem");
    QStringList appearanceData;
    appearanceData << "SubSystem";
    mpLibrary->addComponent("", "SubSystem", pLibsubcomp, appearanceData);


    QMetaObject::connectSlotsByName(this);


    //Establish connections
    this->connect(this->actionSave,SIGNAL(triggered()),mpProjectTabs,SLOT(saveProjectTab()));
    this->connect(this->actionSaveAs,SIGNAL(triggered()),mpProjectTabs,SLOT(saveProjectTabAs()));
    this->connect(this->actionClose,SIGNAL(triggered()),SLOT(close()));
    this->connect(this->actionProject,SIGNAL(triggered()),mpProjectTabs,SLOT(addNewProjectTab()));
    this->connect(this->actionLoadLibs,SIGNAL(triggered()),mpLibrary,SLOT(addLibrary()));
    this->connect(this->actionOpen,SIGNAL(triggered()),mpProjectTabs,SLOT(loadModel()));
    this->connect(this->actionPreferences,SIGNAL(triggered()),this,SLOT(openPreferences()));


    this->connect(this->actionPlot,SIGNAL(triggered()),SLOT(plot()));

    this->connect(this->actionSimulate,SIGNAL(triggered()),mpProjectTabs,SLOT(simulateCurrent()));
    connect(mpSimulationSetupWidget->mpSimulateButton, SIGNAL(released()), mpProjectTabs, SLOT(simulateCurrent()));
}

MainWindow::~MainWindow()
{
    delete mpProjectTabs;
    delete menubar;
    delete statusBar;
}


void MainWindow::plot()
{
    QDockWidget *varPlotDock = new QDockWidget(tr("Plot Variables"), this);
    varPlotDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    VariableListDialog *variableList = new VariableListDialog(varPlotDock);
    varPlotDock->setWidget(variableList);
    //variableList->show();
    addDockWidget(Qt::RightDockWidgetArea, varPlotDock);

}


//! Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllProjectTabs then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mpProjectTabs->closeAllProjectTabs())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}


//! Defines the actions used by the toolbars
void MainWindow::createActions()
{

    newAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_39.png"), tr("&New"), this);
    newAction->setShortcut(tr("New"));
    newAction->setStatusTip(tr("Create New Project"));
    connect(newAction, SIGNAL(triggered()), mpProjectTabs,SLOT(addNewProjectTab()));

    openAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_13.png"), tr("&Open"), this);
    openAction->setShortcut(tr("Open"));
    openAction->setStatusTip(tr("Load Model File"));
    connect(openAction, SIGNAL(triggered()), mpProjectTabs,SLOT(loadModel()));

    saveAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_11.png"), tr("&Save"), this);
    saveAction->setShortcut(tr("Save"));
    saveAction->setStatusTip(tr("Save Model File"));
    connect(saveAction, SIGNAL(triggered()), mpProjectTabs,SLOT(saveProjectTab()));

    saveAsAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_12.png"), tr("&Save As"), this);
    saveAsAction->setShortcut(tr("Save As"));
    saveAsAction->setStatusTip(tr("Save Model File As"));
    connect(saveAsAction, SIGNAL(triggered()), mpProjectTabs,SLOT(saveProjectTabAs()));

    cutAction = new QAction(QIcon("../../HopsanGUI/icons/cut.png"), tr("&Cut"), this);
    cutAction->setShortcut(tr("Cut"));
    cutAction->setStatusTip(tr("Cut Selection"));
    connect(cutAction, SIGNAL(triggered()), this->mpProjectTabs->getCurrentTab()->mpGraphicsView,SLOT(cutSelected()));

    copyAction = new QAction(QIcon("../../HopsanGUI/icons/copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Copy"));
    copyAction->setStatusTip(tr("Copy Selection"));
    connect(copyAction, SIGNAL(triggered()), this->mpProjectTabs->getCurrentTab()->mpGraphicsView,SLOT(copySelected()));

    pasteAction = new QAction(QIcon("../../HopsanGUI/icons/paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Paste"));
    pasteAction->setStatusTip(tr("Paste Selection"));
    connect(pasteAction, SIGNAL(triggered()), this->mpProjectTabs->getCurrentTab()->mpGraphicsView,SLOT(paste()));

    simulateAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_27.png"), tr("&Simulate"), this);
    simulateAction->setShortcut(tr("Simulate"));
    simulateAction->setStatusTip(tr("Simulate Current Project"));
    connect(simulateAction, SIGNAL(triggered()), mpProjectTabs,SLOT(simulateCurrent()));

    plotAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_20.png"), tr("&Plot"), this);
    plotAction->setShortcut(tr("Plot"));
    plotAction->setStatusTip(tr("Plot Something"));
    connect(plotAction, SIGNAL(triggered()), this,SLOT(plot()));
}



//! Creates the toolbars
void MainWindow::createToolbars()
{
    fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->setAllowedAreas(Qt::TopToolBarArea);
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);

    clipboardToolBar = addToolBar(tr("Clipboard Toolbar"));
    clipboardToolBar->setAllowedAreas(Qt::TopToolBarArea);
    clipboardToolBar->addAction(cutAction);
    clipboardToolBar->addAction(copyAction);
    clipboardToolBar->addAction(pasteAction);

    simToolBar = addToolBar(tr("Simulation Toolbar"));
    simToolBar->setAllowedAreas(Qt::TopToolBarArea);
    simToolBar->addAction(simulateAction);
    simToolBar->addAction(plotAction);

    mpSimulationToolBar = addToolBar(tr("Simulation"));
    mpSimulationToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimulationToolBar->addWidget(mpSimulationSetupWidget);

}


void MainWindow::openPreferences()
{
    this->mpPreferenceWidget->show();
}
