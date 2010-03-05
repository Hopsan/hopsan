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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    this->resize(800,600);
    this->setWindowTitle("HOPSAN NG");

    //Create a centralwidget for the main window
    centralwidget = new QWidget(this);
    centralwidget->setObjectName("centralwidget");

    //Create a grid on the centralwidget
    centralgrid = new QGridLayout(centralwidget);
    centralgrid->setSpacing(10);

    //Create a dock for the MessageWidget
    QDockWidget *messagedock = new QDockWidget(tr("Messages"), this);
    messagedock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageWidget->setReadOnly(true);
    messagedock->setWidget(mpMessageWidget);
    addDockWidget(Qt::BottomDockWidgetArea, messagedock);

    //Create the main tab container, need at least one tab
    projectTabs = new ProjectTabWidget(this);
    projectTabs->setObjectName("projectTabs");

    //Create a dock for the componentslibrary
    QDockWidget *libdock = new QDockWidget(tr("Components"), this);
    libdock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    library = new LibraryWidget(this);
    libdock->setWidget(library);
    addDockWidget(Qt::LeftDockWidgetArea, libdock);

    centralgrid->addWidget(projectTabs,0,0);

    centralwidget->setLayout(centralgrid);

    //Set the centralwidget
    this->setCentralWidget(centralwidget);

    projectTabs->addNewProjectTab();

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

    actionClose = new QAction(this);
    actionClose->setText("Close");

    actionSave = new QAction(this);
    actionSave->setText("Save");

    actionProject = new QAction(this);
    actionProject->setText("Project");

    actionLoadLibs = new QAction(this);
    actionLoadLibs->setText("Load Libraries");

    actionSimulate = new QAction(this);
    actionSimulate->setText("Simulate");

    actionPlot = new QAction(this);
    actionPlot->setText("Plot");

    //Add the actionbuttons to the menues
    menuNew->addAction(actionProject);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(actionOpen);
    menuFile->addAction(actionSave);
    menuFile->addSeparator();
    menuFile->addAction(actionClose);

    menuLibs->addAction(actionLoadLibs);
    menuLibs->addAction(actionOpen);

    menuSimulation->addAction(actionSimulate);

    menuView->addAction(libdock->toggleViewAction());

    menuPlot->addAction(actionPlot);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuLibs->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuPlot->menuAction());

    //Load default libraries
    library->addEmptyLibrary("User defined libraries");

    library->addEmptyLibrary("Hydraulic");
    library->addLibrary("../../HopsanGUI/componentData/hydraulic/sources","Hydraulic");
    library->addLibrary("../../HopsanGUI/componentData/hydraulic/restrictors","Hydraulic");
    library->addLibrary("../../HopsanGUI/componentData/hydraulic/volumes","Hydraulic");
    library->addLibrary("../../HopsanGUI/componentData/hydraulic/actuators","Hydraulic");

    library->addLibrary("../../HopsanGUI/componentData/signal");

    QMetaObject::connectSlotsByName(this);


    //Establish connections
    this->connect(this->actionSave,SIGNAL(triggered()),projectTabs,SLOT(saveProjectTab()));
    this->connect(this->actionClose,SIGNAL(triggered()),SLOT(close()));
    this->connect(this->actionProject,SIGNAL(triggered()),projectTabs,SLOT(addNewProjectTab()));
    this->connect(this->actionLoadLibs,SIGNAL(triggered()),library,SLOT(addLibrary()));
    this->connect(this->actionOpen,SIGNAL(triggered()),projectTabs,SLOT(loadModel()));

    this->connect(this->actionPlot,SIGNAL(triggered()),SLOT(plot()));

    this->connect(this->actionSimulate,SIGNAL(triggered()),projectTabs,SLOT(simulateCurrent()));

}

MainWindow::~MainWindow()
{
    delete projectTabs;
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
    if (projectTabs->closeAllProjectTabs())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

