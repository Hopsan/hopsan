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
    this->setWindowIcon(QIcon("../../HopsanGUI/icons/hopsan.png"));

    mpPreferenceWidget = new PreferenceWidget(this);
    mpOptionsWidget = new OptionsWidget(this);

    //Create a centralwidget for the main window
    mpCentralwidget = new QWidget(this);
    mpCentralwidget->setObjectName("centralwidget");

    //Create a grid on the centralwidget
    mpCentralgrid = new QGridLayout(mpCentralwidget);
    mpCentralgrid->setSpacing(10);

    //Create a dock for the MessageWidget
    messagedock = new QDockWidget(tr("Messages"), this);
    messagedock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageWidget->setReadOnly(true);
    messagedock->setWidget(mpMessageWidget);
    addDockWidget(Qt::BottomDockWidgetArea, messagedock);
    mpMessageWidget->printGUIMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    //Create a dock for the componentslibrary
    libdock = new QDockWidget(tr("Components"), this);
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



    //Create the Statusbar
    statusBar = new QStatusBar();
    statusBar->setObjectName("statusBar");
    this->setStatusBar(statusBar);

    this->createActions();
    this->createToolbars();
    this->createMenus();

    mpProjectTabs->addNewProjectTab();

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

    //-------------------------------------------------------------------------------------
    //! @todo This is QDcode, needs to be rewritten in some better way
    mpLibrary->addEmptyLibrary("Subsystem");
    QStringList appearanceData;
    appearanceData << "Subsystem";
    appearanceData << QString("../../HopsanGUI/subsystemtmp.svg"); //Icon path
    QIcon icon;
    icon.addFile("../../HopsanGUI/subsystemtmp.svg");
    LibraryContentItem *pLibsubcomp = new LibraryContentItem(icon,"Subsystem");
    mpLibrary->addComponent("", "Subsystem", pLibsubcomp, appearanceData);
    //------------------------------------------------------------------------------------

    QMetaObject::connectSlotsByName(this);

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
    openAction->setShortcut(QKeySequence("Ctrl+o"));
    openAction->setStatusTip(tr("Load Model File"));
    connect(openAction, SIGNAL(triggered()), mpProjectTabs,SLOT(loadModel()));

    saveAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_11.png"), tr("&Save"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+s"));
    saveAction->setStatusTip(tr("Save Model File"));
    connect(saveAction, SIGNAL(triggered()), mpProjectTabs,SLOT(saveProjectTab()));

    saveAsAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_12.png"), tr("&Save As"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    saveAsAction->setStatusTip(tr("Save Model File As"));
    connect(saveAsAction, SIGNAL(triggered()), mpProjectTabs,SLOT(saveProjectTabAs()));

    closeAction = new QAction(this);
    closeAction->setText("Close");
    closeAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(this->closeAction,SIGNAL(triggered()),SLOT(close()));

    cutAction = new QAction(QIcon("../../HopsanGUI/icons/cut.png"), tr("&Cut"), this);
    cutAction->setShortcut(tr("Cut"));
    cutAction->setStatusTip(tr("Cut Selection"));

    copyAction = new QAction(QIcon("../../HopsanGUI/icons/copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Copy"));
    copyAction->setStatusTip(tr("Copy Selection"));

    pasteAction = new QAction(QIcon("../../HopsanGUI/icons/paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Paste"));
    pasteAction->setStatusTip(tr("Paste Selection"));

    simulateAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_27.png"), tr("&Simulate"), this);
    simulateAction->setShortcut(tr("Simulate"));
    simulateAction->setStatusTip(tr("Simulate Current Project"));
    connect(simulateAction, SIGNAL(triggered()), mpProjectTabs,SLOT(simulateCurrent()));

    plotAction = new QAction(QIcon("../../HopsanGUI/icons/onebit_20.png"), tr("&Plot"), this);
    plotAction->setShortcut(tr("Plot"));
    plotAction->setStatusTip(tr("Plot Something"));
    connect(plotAction, SIGNAL(triggered()), this,SLOT(plot()));

    loadLibsAction = new QAction(this);
    loadLibsAction->setText("Load Libraries");
    connect(loadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addLibrary()));

    preferencesAction = new QAction(QIcon("../../HopsanGUI/icons/preferences.png"), tr("&Model Preferences"), this);
    preferencesAction->setText("Model Preferences");
    preferencesAction->setShortcut(QKeySequence("Ctrl+Alt+p"));
    connect(preferencesAction,SIGNAL(triggered()),this,SLOT(openPreferences()));

    optionsAction = new QAction(QIcon("../../HopsanGUI/icons/options.png"), tr("&Options"), this);
    optionsAction->setText("Options");
    connect(optionsAction,SIGNAL(triggered()),this,SLOT(openOptions()));

    resetZoomAction = new QAction(QIcon("../../HopsanGUI/icons/zoom100.png"), tr("&Reset Zoom"), this);
    resetZoomAction->setText("Reset Zoom");
}


//! Creates the menus
void MainWindow::createMenus()
{
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

    menuSimulation = new QMenu(menubar);
    menuSimulation->setObjectName("menuSimulation");
    menuSimulation->setTitle("&Simulation");

    menuEdit = new QMenu(menubar);
    menuEdit->setTitle("&Edit");

    menuView = new QMenu(menubar);
    menuView->setTitle("&View");

    menuTools = new QMenu(menubar);
    menuTools->setTitle("&Tools");

    menuPlot = new QMenu(menubar);
    menuPlot->setTitle("&Plot");

    this->setMenuBar(menubar);

    //Add the actionbuttons to the menues
    newAction->setText("Project");
    menuNew->addAction(newAction);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(openAction);
    menuFile->addAction(saveAction);
    menuFile->addAction(saveAsAction);
    menuFile->addSeparator();
    menuFile->addAction(loadLibsAction);
    menuFile->addSeparator();
    menuFile->addAction(preferencesAction);
    menuFile->addSeparator();
    menuFile->addAction(closeAction);

    menuSimulation->addAction(simulateAction);

    menuEdit->addAction(copyAction);
    menuEdit->addAction(cutAction);
    menuEdit->addAction(pasteAction);

    menuView->addAction(libdock->toggleViewAction());
    menuView->addAction(messagedock->toggleViewAction());
    menuView->addAction(fileToolBar->toggleViewAction());
    menuView->addAction(clipboardToolBar->toggleViewAction());
    menuView->addAction(simToolBar->toggleViewAction());

    menuTools->addAction(optionsAction);

    menuPlot->addAction(plotAction);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuTools->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuPlot->menuAction());
}

//! Creates the toolbars
void MainWindow::createToolbars()
{
    //viewScaleCombo = new QComboBox;
    //QStringList scales;
    //scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    //viewScaleCombo->addItems(scales);
    //viewScaleCombo->setCurrentIndex(2);

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
    clipboardToolBar->addAction(optionsAction);

    simToolBar = addToolBar(tr("Simulation Toolbar"));
    simToolBar->setAllowedAreas(Qt::TopToolBarArea);
    //simToolBar->addWidget(viewScaleCombo);
    simToolBar->addAction(resetZoomAction);
    simToolBar->addAction(preferencesAction);
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

void MainWindow::openOptions()
{
    this->mpOptionsWidget->show();
}
