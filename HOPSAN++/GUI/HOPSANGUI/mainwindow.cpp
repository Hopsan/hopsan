#include "mainwindow.h"
#include <iostream>
#include <QtGui/QFileDialog>
#include <QtCore/QTextStream>

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
    projectTabs = new ProjectTabWidget();
    projectTabs->setObjectName("projectTabs");
    projectTabs->addProjectTab();

    //Create the library for components representation
    library = new LibraryWidget(this);

    //Add the tree and tabcontainer to the centralgrid
    centralgrid->addWidget(library,0,0);
    centralgrid->addWidget(projectTabs,0,1,5,1);
    centralgrid->setColumnMinimumWidth(0,120);
    centralgrid->setColumnStretch(0,0);
    centralgrid->setColumnMinimumWidth(1,100);
    centralgrid->setColumnStretch(1,10);

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

    //Add the actionbuttons to the menues
    menuNew->addAction(actionProject);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(actionOpen);
    menuFile->addAction(actionSave);
    menuFile->addSeparator();
    menuFile->addAction(actionClose);

    menuLibs->addAction(actionLoadLibs);

    menuSimulation->addAction(actionSimulate);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuLibs->menuAction());
    menubar->addAction(menuSimulation->menuAction());

    QMetaObject::connectSlotsByName(this);


    //Establish connections
    this->connect(this->actionSave,SIGNAL(triggered()),projectTabs,SLOT(saveProjectTab()));
    this->connect(this->actionClose,SIGNAL(triggered()),SLOT(close()));
    this->connect(this->actionProject,SIGNAL(triggered()),projectTabs,SLOT(addProjectTab()));
    this->connect(this->actionLoadLibs,SIGNAL(triggered()),SLOT(addLibs()));

}

MainWindow::~MainWindow()
{
    delete projectTabs;
    //delete tab;
    delete menubar;
    delete statusBar;
    delete scene;
    delete view;
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

    //If no directory is set, i.e. cancel is presses, do no more
    if (libDir.isEmpty() == true)
        return;

    QDir libDirObject(libDir);  //Create a QDir object that contains the info about the library direction

    //Get the name for the library to be set in the tree
    QString libName = libDirObject.dirName();

    //Add the library to the tree
    library->addLibrary(libName);

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

        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
            return;

        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file
        while (!inFile.atEnd()) {
            QString line = inFile.readLine();   //line contains each row in the file

            if (line.startsWith("NAME")){
                componentName = line.mid(5);
            }

            if (line.startsWith("ICON")){
                iconPath = libDirObject.absolutePath() + "/" + line.mid(5);
                icon.addFile(iconPath);
            }
        }
        file.close();
        //Add data to the paremeterData list
        parameterData << componentName << iconPath;
        //Add the component to the library
        library->addComponent(libName,componentName,icon,parameterData);
    }
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

