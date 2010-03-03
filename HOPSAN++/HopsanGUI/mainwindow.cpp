//$Id$

#include "mainwindow.h"
#include <iostream>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>
#include <QDebug>

#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QStatusBar>
#include <QtGui/QAction>
#include <QtCore/QMetaObject>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QIODevice>
#include <QListWidgetItem>
#include <QStringList>
#include <QDockWidget>

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

    //Create the main tab container, need at least one tab
    projectTabs = new ProjectTabWidget(this);
    projectTabs->setObjectName("projectTabs");
    projectTabs->addProjectTab();

    //Create the library for components representation
    /*library = new LibraryWidget(this);

    //Add the tree and tabcontainer to the centralgrid
    centralgrid->addWidget(library,0,0);
    centralgrid->addWidget(projectTabs,0,1,5,1);
    centralgrid->setColumnMinimumWidth(0,120);
    centralgrid->setColumnStretch(0,0);
    centralgrid->setColumnMinimumWidth(1,100);
    centralgrid->setColumnStretch(1,10);*/

    //Create a dock for the componentslibrary
    QDockWidget *libdock = new QDockWidget(tr("Components"), this);
    libdock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    library = new LibraryWidget(libdock);
    libdock->setWidget(library);
    addDockWidget(Qt::LeftDockWidgetArea, libdock);

    centralgrid->addWidget(projectTabs,0,0);

    centralwidget->setLayout(centralgrid);

    //Set the centralwidget
    this->setCentralWidget(centralwidget);

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
    library->addLibrary("User defined libraries");
    library->addLibrary("Hydraulic");
    addLibs("../../HopsanGUI/componentData/hydraulic/sources","Hydraulic");//This method should be in LibraryWidget and addLibs() may be here
    addLibs("../../HopsanGUI/componentData/hydraulic/restrictors","Hydraulic");//This method should be in LibraryWidget and addLibs() may be here
    addLibs("../../HopsanGUI/componentData/hydraulic/volumes","Hydraulic");//This method should be in LibraryWidget and addLibs() may be here
    addLibs("../../HopsanGUI/componentData/hydraulic/actuators","Hydraulic");//This method should be in LibraryWidget and addLibs() may be here

    addLibs("../../HopsanGUI/componentData/signal");//This method should be in LibraryWidget and addLibs() may be here

    QMetaObject::connectSlotsByName(this);


    //Establish connections
    this->connect(this->actionSave,SIGNAL(triggered()),projectTabs,SLOT(saveProjectTab()));
    this->connect(this->actionClose,SIGNAL(triggered()),SLOT(close()));
    this->connect(this->actionProject,SIGNAL(triggered()),projectTabs,SLOT(addProjectTab()));
    this->connect(this->actionLoadLibs,SIGNAL(triggered()),SLOT(addLibs()));
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


void MainWindow::addLibs(QString libDir, QString parentLib)
{
    //If no directory is set, i.e. cancel is presses, do no more
    if (libDir.isEmpty() == true)
        return;

    QDir libDirObject(libDir);  //Create a QDir object that contains the info about the library direction

    //Get the name for the library to be set in the tree
    QString libName = libDirObject.dirName();

    //Add the library to the tree
    library->addLibrary(libName,parentLib);

    QStringList filters;        //Create a QStringList object that contains name filters
    filters << "*.txt";         //Create the name filter
    libDirObject.setNameFilters(filters);       //Set the name filter

    QStringList libList = libDirObject.entryList(); //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)    //Iterate over the file names
    {
        //Set up needed variables
        QStringList parameterData;
        QString componentName;
        QIcon icon;
        QString iconPath;
        QString nPorts;
        QString portPosX;
        QString portPosY;

        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
            return;

        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file
        while (!inFile.atEnd()) {
            QString line = inFile.readLine();   //line contains each row in the file

            if (line.startsWith("NAME"))
            {
                componentName = line.mid(5);
                parameterData << componentName;
            }

            if (line.startsWith("ICON"))
            {
                iconPath = libDirObject.absolutePath() + "/" + line.mid(5);
                icon.addFile(iconPath);
                parameterData << iconPath;
            }
            if (line.startsWith("PORTS"))
            {
                nPorts = line.mid(6);
                parameterData << nPorts;
                for (size_t i = 0; i < nPorts.toInt(); ++i)
                {
                    line = inFile.readLine();
                    portPosX = line.mid(0);
                    line = inFile.readLine();
                    portPosY = line.mid(0);
                    std::cout << qPrintable(componentName) << " x: " << qPrintable(portPosX) << " y: " << qPrintable(portPosY) << std::endl;
                    parameterData << portPosX << portPosY;
                }
            }
        }
        file.close();
        //Add data to the paremeterData list
  //      parameterData << componentName << iconPath;

        ListWidgetItem *libcomp= new ListWidgetItem(icon,componentName);
      //  std::cout << parameterData.size() << std::endl;
        libcomp->setParameterData(parameterData);

        //Add the component to the library
        //library->addComponent(libName,componentName,icon,parameterData);
        library->addComponent(libName, libcomp, parameterData);
    }
}



void MainWindow::addLibs()
{
    /*QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    fileName = QFileDialog::getExistingDirectory();*/

    /*Alt. way
    fileName = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));*/

    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir

    libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                 fileDialogOpenDir.currentPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    addLibs(libDir,QString("User defined libraries"));
    //std::cout << qPrintable(libDir) << std::endl;
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

