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
    projectTabs = new QTabWidget();
    projectTabs->setObjectName("projectTabs");

    tab = new QWidget();
    tab->setObjectName("tab");
    projectTabs->addTab(tab,"");

    //Create a grid for the tabs
    tabgrid = new QGridLayout(tab);

    //Create the tree for components
    componentsTree = new TreeWidget();
    componentsTree->setHeaderLabel("Components");
    componentsTree->setColumnCount(2);

    //Add the toolbox and tabcontainer to the centralgrid
    centralgrid->addWidget(componentsTree,0,0);
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
    menuFile->addSeparator();
    menuFile->addAction(actionClose);

    menuLibs->addAction(actionLoadLibs);

    menuSimulation->addAction(actionSimulate);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuLibs->menuAction());
    menubar->addAction(menuSimulation->menuAction());

    QMetaObject::connectSlotsByName(this);


    //Establish connections
    this->connect(this->actionClose,SIGNAL(triggered()),SLOT(close()));
    this->connect(this->actionProject,SIGNAL(triggered()),SLOT(addProject()));
    this->connect(this->actionLoadLibs,SIGNAL(triggered()),SLOT(addLibs()));


}

MainWindow::~MainWindow()
{
}

void MainWindow::addProject()
{
    std::cout << "hej" << std::endl;
}

void MainWindow::addLibs()
{
    /*QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    fileName = QFileDialog::getExistingDirectory();*/

    /*Alt. way
    fileName = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));*/

    libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    QDir libDirObject(libDir);  //Create a QDir object

    QTreeWidgetItem *libName = new QTreeWidgetItem(this->componentsTree);   //Create an item for the treewidget
    libName->setText(0,libDirObject.dirName()); //set the name of the treeitem to component directorys name

    QStringList filters;        //Create a QStringList object that contains name filters
    filters << "*.txt";         //Create the name filter
    libDirObject.setNameFilters(filters);       //Set the name filter

    QStringList libList = libDirObject.entryList(); //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)    //Iterate over the file names
    {
        //std::cout << libList.at(i).toLocal8Bit().constData() << std::endl;

        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
            return;

        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file
        while (!inFile.atEnd()) {
            QString line = inFile.readLine();   //line contains each row in the file
            //std::cout << line.toLocal8Bit().constData() << std::endl;
            QTreeWidgetItem *componentName = new QTreeWidgetItem(libName); //Create a new tree item for each component
            componentName->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

            if (line.startsWith("NAME")){
                componentName->setText(0,line.mid(5));
               //std::cout << line.mid(5).toStdString() << std::endl;
            }

            if (line.startsWith("ICON")){
                QString iconPath = libDirObject.absolutePath() + "/" + line.mid(5);
                QIcon icon(iconPath);
                componentName->setIcon(1,icon);
            }
        }
        file.close();

    }


}
