/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   LibraryWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Library Widgets
//! Revised 2011-06-17 by Robert Braun
//!
//$Id$

#include <QtGui>

#include "Configuration.h"
#include "LibraryWidget.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/ProjectTabWidget.h"
#include "common.h"
#include "version_gui.h"

using namespace std;
using namespace hopsan;

//! @todo Make "External Libraries" a reserved word

// Forward declarations
class LibraryComponent;

//! @brief Constructor for the library widget
//! @param parent Pointer to the parent (main window)
LibraryWidget::LibraryWidget(MainWindow *parent)
        :   QWidget(parent)
{
    mUpConvertAllCAF = UNDECIDED_TO_ALL;
    mpCoreAccess = new CoreLibraryAccess();

    //! @todo Dont know if this is the right place to do this, but we need to do it early
    // We want to register certain GUI specific KeyValues in the core to prevent external libs from loading components with theses typenames
    mpCoreAccess->reserveComponentTypeName(HOPSANGUICONTAINERPORTTYPENAME);
    mpCoreAccess->reserveComponentTypeName(HOPSANGUISYSTEMTYPENAME);
    mpCoreAccess->reserveComponentTypeName(HOPSANGUIGROUPTYPENAME);

    mpContentsTree = new LibraryContentsTree();
    mpSecretHiddenContentsTree = new LibraryContentsTree();

    //mpTree = new LibraryTreeWidget(this);
    mpTree = new QTreeWidget(this);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);

    mpComponentNameField = new QLabel();
    mpComponentNameField->hide();

    mpList = new LibraryListWidget(this);
    mpList->setViewMode(QListView::IconMode);
    mpList->setResizeMode(QListView::Adjust);
    mpList->setIconSize(QSize(40,40));
    mpList->setGridSize(QSize(45,45));
    mpList->hide();

    QSize iconSize = QSize(24,24);  //Size of library icons

    mpTreeViewButton = new QToolButton();
    mpTreeViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryTreeView.png"));
    mpTreeViewButton->setIconSize(iconSize);
    mpTreeViewButton->setToolTip(tr("Single List View"));
    mpDualViewButton = new QToolButton();
    mpDualViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryDualView.png"));
    mpDualViewButton->setIconSize(iconSize);
    mpDualViewButton->setToolTip(tr("Dual List View"));
    mpGenerateComponentButton = new QToolButton();
    mpGenerateComponentButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-New.png"));
    mpGenerateComponentButton->setIconSize(iconSize);
    mpGenerateComponentButton->setToolTip(tr("Generate New Component"));
    mpLoadExternalButton = new QToolButton();
    mpLoadExternalButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LoadLibrary.png"));
    mpLoadExternalButton->setIconSize(iconSize);
    mpLoadExternalButton->setToolTip(tr("Load External Library"));
    mpLoadFmuButton = new QToolButton();
    mpLoadFmuButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));
    mpLoadFmuButton->setIconSize(iconSize);
    mpLoadFmuButton->setToolTip(tr("Import Functional Mockup Unit (FMU)"));

    connect(mpTreeViewButton, SIGNAL(clicked()), this, SLOT(setListView()));
    connect(mpDualViewButton, SIGNAL(clicked()), this, SLOT(setDualView()));
    connect(mpGenerateComponentButton, SIGNAL(clicked()), this, SLOT(generateComponent()));
    connect(mpLoadExternalButton, SIGNAL(clicked()), this, SLOT(addExternalLibrary()));
    connect(mpLoadFmuButton, SIGNAL(clicked()), this, SLOT(importFmu()));

    mpGrid = new QGridLayout(this);
    mpGrid->addWidget(mpTree,                       0,0,1,6);
    mpGrid->addWidget(mpComponentNameField,         1,0,1,6);
    mpGrid->addWidget(mpList,                       2,0,1,6);
    mpGrid->addWidget(mpTreeViewButton,             3,0,1,1);
    mpGrid->addWidget(mpDualViewButton,             3,1,1,1);
    mpGrid->addWidget(mpGenerateComponentButton,    3,2,1,1);
    mpGrid->addWidget(mpLoadExternalButton,         3,3,1,1);
#ifdef DEVELOPMENT
    mpGrid->addWidget(mpLoadFmuButton,              3,4,1,1);
#endif
    mpGrid->setContentsMargins(4,4,4,4);
    mpGrid->setHorizontalSpacing(0);

    setLayout(mpGrid);
    this->setMouseTracking(true);

    mViewMode = gConfig.getLibraryStyle();
    this->setGfxType(USERGRAPHICS);     //Also updates the widget
}


//! @brief Reimplementation of QWidget::sizeHint()
//! Used to reduce the size of the library widget when docked.
QSize LibraryWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    size.rwidth() = 210;            //Set very small width. A minimum apperantly stops at resonable size.
    return size;
}


//! @brief Refreshes the contents in the library widget
void LibraryWidget::update()
{
    mpTree->clear();
    mpList->clear();
    mListItemToContentsMap.clear();
    mTreeItemToContentsMap.clear();

    switch (mViewMode)
    {
      case 0:
        {
            //Do stuff 0
            loadTreeView(mpContentsTree);
        }
        break;

      case 1:
        {
            loadDualView(mpContentsTree);
        }
        break;
    }
}


//! @brief Recursive function that reads data from a contents node and draws it in single tree view mode
//! @param tree Library contents tree node to load data from
//! @param parentItem Tree widget item to append the data to
void LibraryWidget::loadTreeView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem)
{
    mpList->hide();

    mTreeItemToContentsTreeMap.insert(parentItem, tree);

    QTreeWidgetItem *tempItem;

        //Recursively call child nodes
    if(parentItem == 0)
    {
        //Load child libraries
        for(int i=0; i<tree->mChildNodesPtrs.size(); ++i)
        {
            if(!tree->mChildNodesPtrs.at(i)->isEmpty())
            {
                tempItem = new QTreeWidgetItem();
                QFont tempFont = tempItem->font(0);
                tempFont.setBold(true);
                tempItem->setFont(0,tempFont);
                tempItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                tempItem->setText(0, tree->mChildNodesPtrs.at(i)->mName);
                tempItem->setToolTip(0, tree->mChildNodesPtrs.at(i)->mName);
                mpTree->addTopLevelItem(tempItem);
                loadTreeView(tree->mChildNodesPtrs.at(i), tempItem);
            }
        }

        //Load components
        for(int i=0; i<tree->mComponentPtrs.size(); ++i)
        {
            tempItem = new QTreeWidgetItem();
            mTreeItemToContentsMap.insert(tempItem, tree->mComponentPtrs.at(i));
            tempItem->setText(0, tree->mComponentPtrs.at(i)->getName());
            tempItem->setToolTip(0, tree->mComponentPtrs.at(i)->getName());
            mpTree->addTopLevelItem(tempItem);
        }
    }
    else
    {
        //Load child libraries
        for(int i=0; i<tree->mChildNodesPtrs.size(); ++i)
        {
            if(!tree->mChildNodesPtrs.at(i)->isEmpty())
            {
                tempItem = new QTreeWidgetItem();
                QFont tempFont = tempItem->font(0);
                tempFont.setBold(true);
                tempItem->setFont(0,tempFont);
                tempItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                tempItem->setText(0, tree->mChildNodesPtrs.at(i)->mName);
                tempItem->setToolTip(0, tree->mChildNodesPtrs.at(i)->mName);
                parentItem->addChild(tempItem);
                loadTreeView(tree->mChildNodesPtrs.at(i), tempItem);
            }
        }

        //Load components
        for(int i=0; i<tree->mComponentPtrs.size(); ++i)
        {
            tempItem = new QTreeWidgetItem();
            mTreeItemToContentsMap.insert(tempItem, tree->mComponentPtrs.at(i));
            tempItem->setText(0, tree->mComponentPtrs.at(i)->getName());
            tempItem->setToolTip(0, tree->mComponentPtrs.at(i)->getName());
            tempItem->setIcon(0, tree->mComponentPtrs.at(i)->getIcon(mGfxType));
            parentItem->addChild(tempItem);
        }
    }

    connect(mpTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(initializeDrag(QTreeWidgetItem*, int)), Qt::UniqueConnection);
}


//! @brief Recursive function that reads data from a contents node and draws it with dual view mode
//! @param tree Library contents tree node to load data from
//! @param parentItem Tree widget item to append the data to
void LibraryWidget::loadDualView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem)
{
    QTreeWidgetItem *tempItem;

    mTreeItemToContentsTreeMap.insert(parentItem, tree);

    //Recursively call child nodes
    if(parentItem == 0)
    {
        //Load child libraries
        for(int i=0; i<tree->mChildNodesPtrs.size(); ++i)
        {
            if(!tree->mChildNodesPtrs.at(i)->isEmpty())
            {
                tempItem = new QTreeWidgetItem();
                QFont tempFont = tempItem->font(0);
                tempFont.setBold(true);
                tempItem->setFont(0,tempFont);
                tempItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                tempItem->setText(0, tree->mChildNodesPtrs.at(i)->mName);
                tempItem->setToolTip(0, tree->mChildNodesPtrs.at(i)->mName);
                mpTree->addTopLevelItem(tempItem);
                loadDualView(tree->mChildNodesPtrs.at(i), tempItem);
            }
        }
    }
    else
    {
        //Load child libraries
        for(int i=0; i<tree->mChildNodesPtrs.size(); ++i)
        {
            if(!tree->mChildNodesPtrs.at(i)->isEmpty())
            {
                tempItem = new QTreeWidgetItem();
                QFont tempFont = tempItem->font(0);
                tempFont.setBold(true);
                tempItem->setFont(0,tempFont);
                tempItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                tempItem->setText(0, tree->mChildNodesPtrs.at(i)->mName);
                tempItem->setToolTip(0, tree->mChildNodesPtrs.at(i)->mName);
                parentItem->addChild(tempItem);
                loadDualView(tree->mChildNodesPtrs.at(i), tempItem);
            }
        }
    }

    mpComponentNameField->show();
    mpList->show();
    connect(mpTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(showLib(QTreeWidgetItem*, int)), Qt::UniqueConnection);
}


//! @brief Slot that loads components from a tree widget item to the icon box in dual view mode
//! @param item Tree widget item to load from
void LibraryWidget::showLib(QTreeWidgetItem *item, int /*column*/)
{
    //Find the node in the contents tree
    QStringList treePath;
    treePath.prepend(item->text(0));
    QTreeWidgetItem *tempItem = item;
    while(tempItem->parent() != 0)
    {
        treePath.prepend(tempItem->parent()->text(0));
        tempItem = tempItem->parent();
    }

    LibraryContentsTree *tree = mpContentsTree;
    for(int i=0; i<treePath.size(); ++i)
    {
        for(int j=0; j<tree->mChildNodesPtrs.size(); ++j)
        {
            if(tree->mChildNodesPtrs.at(j)->mName == treePath.at(i))
            {
                tree = tree->mChildNodesPtrs.at(j);
                break;
            }
        }
    }

    mpList->clear();

    qDebug() << "1";

    //Add components
    for(int i=0; i<tree->mComponentPtrs.size(); ++i)        //Add own components
    {
        QListWidgetItem *tempItem = new QListWidgetItem();
        tempItem->setIcon(tree->mComponentPtrs.at(i)->getIcon(mGfxType));
        tempItem->setToolTip(tree->mComponentPtrs.at(i)->getName());
        mListItemToContentsMap.insert(tempItem, tree->mComponentPtrs.at(i));
        tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        mpList->addItem(tempItem);
    }
    qDebug() << "2";
    for(int j=0; j<tree->mChildNodesPtrs.size(); ++j)       //Add components from child libraries too
    {
        for(int i=0; i<tree->mChildNodesPtrs.at(j)->mComponentPtrs.size(); ++i)
        {
            QListWidgetItem *tempItem = new QListWidgetItem();
            tempItem->setToolTip(tree->mChildNodesPtrs.at(j)->mComponentPtrs.at(i)->getName());
            tempItem->setIcon(tree->mChildNodesPtrs.at(j)->mComponentPtrs.at(i)->getIcon(mGfxType));
            mListItemToContentsMap.insert(tempItem, tree->mChildNodesPtrs.at(j)->mComponentPtrs.at(i));
            tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpList->addItem(tempItem);
        }
    }
    qDebug() << "3";

    connect(mpList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(initializeDrag(QListWidgetItem*)), Qt::UniqueConnection);
}


//! @brief Initializes drag operation to workspace from a list widget item
//! @param item List widget item
void LibraryWidget::initializeDrag(QListWidgetItem *item)
{
    if(!mListItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString typeName = mListItemToContentsMap.find(item).value()->getTypeName();
    QIcon icon = mListItemToContentsMap.find(item).value()->getIcon(mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(typeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);

    mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();
}


//! @brief Initializes drag operation to workspace from a tree widget item
//! @param item Tree widget item
void LibraryWidget::initializeDrag(QTreeWidgetItem *item, int /*dummy*/)
{
    if(!mTreeItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString typeName = mTreeItemToContentsMap.find(item).value()->getTypeName();
    QIcon icon = mTreeItemToContentsMap.find(item).value()->getIcon(mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(typeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}


//! @brief Loads a component library from specified directory
//! First loads DLL files, then XML. DLL files are not necessary if component is already loaded.
//! @param libDir Directory to check
//! @param external Used to indicate that the library is external (places contents under "External Libraries")
void LibraryWidget::loadLibrary(QString libDir, bool external)
{
    if (libDir.isEmpty() == true)       // Don't add empty folders to the library
        return;

    //Create a QDir object that contains the info about the library directory
    QDir libDirObject(libDir);

    if(external)
    {
        LibraryContentsTree *pExternalTree;
        if(!mpContentsTree->findChild("External Libraries"))
            pExternalTree = mpContentsTree->addChild("External Libraries");
        else
            pExternalTree = mpContentsTree->findChild("External Libraries");
        loadLibraryFolder(libDir, pExternalTree);
    }
    else
    {
        libDirObject.setFilter(QDir::AllDirs);
        QStringList subDirList = libDirObject.entryList();
        subDirList.removeAll(".");
        subDirList.removeAll("..");
        subDirList.removeAll(".svn");
        for(int i=0; i<subDirList.size(); ++i)
        {
            loadLibraryFolder(libDir+"/"+subDirList.at(i), mpContentsTree);
        }
    }

    update();       //Redraw the library
}


//! @brief Slots that opens the component generator dialog
void LibraryWidget::generateComponent()
{
    gpMainWindow->getComponentGeneratorDialog()->open();
}


//! @brief Adds a new external library to Hopsan
//! @param libDir Directory to add (empty = user selection)
void LibraryWidget::addExternalLibrary(QString libDir)
{
    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir

    if(libDir.isEmpty())    //Let user select a directory if no directory is specified
    {
        libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                   fileDialogOpenDir.currentPath(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    }
    if(!gConfig.hasUserLib(libDir))     //Check so that path does not already exist
    {
        gConfig.addUserLib(libDir);     //Register new library in configuration
        loadExternalLibrary(libDir);    //Load the library
    }
    else
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Library " + libDir + " is already loaded!");
    }
}


void LibraryWidget::importFmu()
{
    //Load .fmu file and create paths
    QString filePath = QFileDialog::getOpenFileName(this, tr("Import Functional Mockup Unit (FMU)"),
                                          gExecPath + "/../",
                                          tr("Functional Mockup Unit (*.fmu)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;


    QProgressDialog progressBar(tr("Initializing"), QString(), 0, 0, gpMainWindow);
    progressBar.show();
    progressBar.setMaximum(10);
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Importing FMU"));
    progressBar.setValue(0);


    QFileInfo fmuFileInfo = QFile(filePath);
    fmuFileInfo.setFile(filePath);

    QDir zipDir;
    zipDir = QDir::cleanPath(gExecPath + "../ThirdParty/7z");

    QDir gccDir;
    gccDir = QDir::cleanPath(gExecPath + "../ThirdParty/mingw32/bin");

    QString fmuName = fmuFileInfo.fileName();
    fmuName.chop(4);

    if(!QDir(gExecPath + "../import").exists())
        QDir().mkdir(gExecPath + "../import");

    if(!QDir(gExecPath + "../import/FMU").exists())
        QDir().mkdir(gExecPath + "../import/FMU");

    if(!QDir(gExecPath + "../import/FMU/" + fmuName).exists())
        QDir().mkdir(gExecPath + "../import/FMU/" + fmuName);

    QString fmuPath = gExecPath + "../import/FMU/" + fmuName;
    QDir fmuDir = QDir::cleanPath(fmuPath);


    progressBar.setValue(1);
    progressBar.setLabelText("Unpacking files");


    //Unzip .fmu file
    QProcess zipProcess;
    zipProcess.setWorkingDirectory(zipDir.path());
    QStringList arguments;
    arguments << "x" << fmuFileInfo.filePath() << "-o" + fmuDir.path() << "-aoa";
    zipProcess.start(zipDir.path() + "/7z.exe", arguments);
    zipProcess.waitForFinished();
    QByteArray zipResult = zipProcess.readAll();
    QList<QByteArray> zipResultList = zipResult.split('\n');
    for(int i=0; i<zipResultList.size(); ++i)
    {
        QString msg = zipResultList.at(i);
        msg = msg.remove(msg.size()-1, 1);
        if(!msg.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage(msg);
        }
    }


    progressBar.setValue(2);


    //Move all binary files to FMU directory
    QDir win32Dir = QDir::cleanPath(fmuDir.path() + "/binaries/win32");
    if(!win32Dir.exists())
    {
        removeDir(fmuDir.path());
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed.");
        return;
    }
    QFileInfoList binaryFiles = win32Dir.entryInfoList(QDir::Files);
    for(int i=0; i<binaryFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(binaryFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + binaryFiles.at(i).fileName());
        tempFile.remove();
    }


    //Move all resource files to FMU directory
    QDir resDir = QDir::cleanPath(fmuDir.path() + "/resources");
    QFileInfoList resFiles = resDir.entryInfoList(QDir::Files);
    for(int i=0; i<resFiles.size(); ++i)
    {
        QFile tempFile;
        tempFile.setFileName(resFiles.at(i).filePath());
        tempFile.copy(fmuDir.path() + "/" + resFiles.at(i).fileName());
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + tempFile.fileName() + " to " + fmuDir.path() + "/" + resFiles.at(i).fileName());
        tempFile.remove();
    }


    QStringList filters;
    filters << "*.hmf";
    fmuDir.setNameFilters(filters);
    QStringList hmfList = fmuDir.entryList();
    for (int i = 0; i < hmfList.size(); ++i)
    {
        QFile hmfFile;
        hmfFile.setFileName(fmuDir.path() + "/" + hmfList.at(i));
        if(hmfFile.exists())
        {
            hmfFile.copy(gExecPath + hmfList.at(i));
            hmfFile.remove();
            hmfFile.setFileName(gExecPath + hmfList.at(i));
            gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + hmfFile.fileName() + " to " + gExecPath + hmfList.at(i));
        }
    }
    fmuDir.setFilter(QDir::NoFilter);



    progressBar.setValue(2.5);
    progressBar.setLabelText("Parsing XML file");


    //Load XML data from ModelDescription.xml
    //Copy xml-file to this directory
    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(fmuDir.path() + "/ModelDescription.xml");
    if(!win32Dir.exists())
    {
        removeDir(fmuDir.path());
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: ModelDescription.xml not found.");
        return;
    }
    QDomDocument fmuDomDocument;
    QDomElement fmuRoot = loadXMLDomDocument(modelDescriptionFile, fmuDomDocument, "fmiModelDescription");
    modelDescriptionFile.close();

    if(fmuRoot == QDomElement())
    {
        removeDir(fmuDir.path());
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Could not parse ModelDescription.xml.");
        return;
    }


    progressBar.setValue(3);
    progressBar.setLabelText("Writing fmuLib.cc");


    //Create fmuLib.cc
    QFile fmuLibFile;
    fmuLibFile.setFileName(fmuDir.path() + "/fmuLib.cc");
    if(!fmuLibFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Could not open fmuLib.cc for writing.");
        removeDir(fmuDir.path());
        return;
    }

    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Writing fmuLib.cc");

    QTextStream fmuLibStream(&fmuLibFile);
    fmuLibStream << "#include \"component_code/"+fmuName+".hpp\"\n";
    fmuLibStream << "#include \""+gExecPath+INCLUDEPATH+"ComponentEssentials.h\"\n";
    fmuLibStream << "using namespace hopsan;\n\n";
    fmuLibStream << "extern \"C\" DLLEXPORT void register_contents(ComponentFactory* cfact_ptr, NodeFactory* nfact_ptr)\n";
    fmuLibStream << "{\n";
    fmuLibStream << "    cfact_ptr->registerCreatorFunction(\"" + fmuName + "\", " + fmuName + "::Creator);\n";
    fmuLibStream << "}\n";
    fmuLibFile.close();


    progressBar.setValue(3.5);
    progressBar.setLabelText("Writing " + fmuName + ".hpp");


    //Create <fmuname>.hpp
    QDir().mkdir(fmuDir.path() + "/component_code");
    QFile fmuComponentHppFile;
    fmuComponentHppFile.setFileName(fmuDir.path() + "/component_code/" + fmuName + ".hpp");
    if(!fmuComponentHppFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Could not open "+fmuName+".hpp for writing.");
        removeDir(fmuDir.path());
        return;
    }

    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Writing " + fmuName + ".hpp");

    QTextStream fmuComponentHppStream(&fmuComponentHppFile);
    fmuComponentHppStream << "#ifndef "+fmuName+"_H\n";
    fmuComponentHppStream << "#define "+fmuName+"_H\n\n";
    fmuComponentHppStream << "#define BUFSIZE 4096\n\n";
    fmuComponentHppStream << "#define _WIN32_WINNT 0x0502\n";
    fmuComponentHppStream << "#include \"../fmi_me.h\"\n";
    fmuComponentHppStream << "#include \"../xml_parser.h\"\n";
    fmuComponentHppStream << "#include \""+gExecPath+INCLUDEPATH+"ComponentEssentials.h\"\n\n";
    fmuComponentHppStream << "#include <sstream>\n";
    fmuComponentHppStream << "#include <string>\n";
    fmuComponentHppStream << "#include <stdio.h>\n";
    fmuComponentHppStream << "#include <stdlib.h>\n";
    fmuComponentHppStream << "#include <string.h>\n";
    fmuComponentHppStream << "#include <assert.h>\n";
    fmuComponentHppStream << "#ifdef WIN32\n";
    fmuComponentHppStream << "#include <windows.h>\n";
    fmuComponentHppStream << "#endif\n\n";
    fmuComponentHppStream << "void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status,\n";
    fmuComponentHppStream << "               fmiString category, fmiString message, ...){}\n\n";
    fmuComponentHppStream << "namespace hopsan {\n\n";
    fmuComponentHppStream << "    class "+fmuName+" : public ComponentQ\n";
    fmuComponentHppStream << "    {\n";
    fmuComponentHppStream << "    private:\n";
    fmuComponentHppStream << "        FMU mFMU;\n";
    fmuComponentHppStream << "        FILE* mFile;\n";
    fmuComponentHppStream << "        fmiComponent c;                  // instance of the fmu\n";
    fmuComponentHppStream << "        fmiEventInfo eventInfo;          // updated by calls to initialize and eventUpdate\n";
    fmuComponentHppStream << "        int nx;                          // number of state variables\n";
    fmuComponentHppStream << "        int nz;                          // number of state event indicators\n";
    fmuComponentHppStream << "        double *x;                       // continuous states\n";
    fmuComponentHppStream << "        double *xdot;                    // the crresponding derivatives in same order\n";
    fmuComponentHppStream << "        double *z;                // state event indicators\n";
    fmuComponentHppStream << "        double *prez;             // previous values of state event indicators\n\n";
    QDomElement variablesElement = fmuRoot.firstChildElement("ModelVariables");
    QDomElement varElement = variablesElement.firstChildElement("ScalarVariable");
    int i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            fmuComponentHppStream << "        Port *mpIn"+numStr+";\n";
            fmuComponentHppStream << "        Port *mpOut"+numStr+";\n";
        }
        else if(varElement.attribute("causality") == "input")
            fmuComponentHppStream << "        Port *mpIn"+numStr+";\n";
        else if(varElement.attribute("causality") == "output")
            fmuComponentHppStream << "        Port *mpOut"+numStr+";\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    //fmuComponentHppStream << "        Port *mpP1;\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
            fmuComponentHppStream << "        double *mpND_in" + numStr + ";\n";
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
            fmuComponentHppStream << "        double *mpND_out" + numStr + ";\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "\n";
    fmuComponentHppStream << "    public:\n";
    fmuComponentHppStream << "        static Component *Creator()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            return new "+fmuName+"();\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        "+fmuName+"() : ComponentQ()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            mFMU.modelDescription = parse(\""+fmuDir.path()+"/ModelDescription.xml\");\n";
    fmuComponentHppStream << "            assert(mFMU.modelDescription);\n";
    fmuComponentHppStream << "            assert(loadDll(\""+fmuDir.path()+"/"+fmuName+".dll\"));\n";
    fmuComponentHppStream << "            addInfoMessage(getString(mFMU.modelDescription, att_modelIdentifier));\n\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            fmuComponentHppStream << "            mpIn"+numStr+" = addReadPort(\""+varElement.attribute("name")+"In\", \"NodeSignal\", Port::NOTREQUIRED);\n";
            fmuComponentHppStream << "            mpOut"+numStr+" = addWritePort(\""+varElement.attribute("name")+"Out\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        }
        else if(varElement.attribute("causality") == "input")
            fmuComponentHppStream << "            mpIn"+numStr+" = addReadPort(\""+varElement.attribute("name")+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        else if(varElement.attribute("causality") == "output")
            fmuComponentHppStream << "            mpOut"+numStr+" = addWritePort(\""+varElement.attribute("name")+"\", \"NodeSignal\", Port::NOTREQUIRED);\n";
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    //fmuComponentHppStream << "            mpP1 = addWritePort(\"out\", \"NodeSignal\");\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "       void initialize()\n";
    fmuComponentHppStream << "       {\n";
    fmuComponentHppStream << "           if (!mFMU.modelDescription)\n";
    fmuComponentHppStream << "           {\n";
    fmuComponentHppStream << "               addErrorMessage(\"Missing FMU model description\");\n";
    fmuComponentHppStream << "               stopSimulation();\n";
    fmuComponentHppStream << "           }\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    int nInputs=0;
    int nOutputs=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
        {
            fmuComponentHppStream << "          mpND_in"+numStr+" = getSafeNodeDataPtr(mpIn"+numStr+", NodeSignal::VALUE);\n\n";
            ++nInputs;
        }
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
        {
            fmuComponentHppStream << "          mpND_out"+numStr+" = getSafeNodeDataPtr(mpOut"+numStr+", NodeSignal::VALUE);\n\n";
            ++nOutputs;
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "            //Initialize FMU\n";
    fmuComponentHppStream << "            ModelDescription* md;            // handle to the parsed XML file\n";
    fmuComponentHppStream << "            const char* guid;                // global unique id of the fmu\n";
    fmuComponentHppStream << "            fmiCallbackFunctions callbacks;  // called by the model during simulation\n";
    fmuComponentHppStream << "            fmiStatus fmiFlag;               // return code of the fmu functions\n";
    fmuComponentHppStream << "            fmiReal t0 = 0;                  // start time\n";
    fmuComponentHppStream << "            fmiBoolean toleranceControlled = fmiFalse;\n";

    progressBar.setValue(4.5);

    fmuComponentHppStream << "            int loggingOn = 0;\n\n";
    fmuComponentHppStream << "            // instantiate the fmu\n";
    fmuComponentHppStream << "            md = mFMU.modelDescription;\n";
    fmuComponentHppStream << "            guid = getString(md, att_guid);\n";
    fmuComponentHppStream << "            callbacks.logger = fmuLogger;\n";
    fmuComponentHppStream << "            callbacks.allocateMemory = calloc;\n";
    fmuComponentHppStream << "            callbacks.freeMemory = free;\n";
    fmuComponentHppStream << "            c = mFMU.instantiateModel(getModelIdentifier(md), guid, callbacks, loggingOn);\n\n";
    fmuComponentHppStream << "            // allocate memory\n";
    fmuComponentHppStream << "            nx = getNumberOfStates(md);\n";
    fmuComponentHppStream << "            nz = getNumberOfEventIndicators(md);\n";
    fmuComponentHppStream << "            x    = (double *) calloc(nx, sizeof(double));\n";
    fmuComponentHppStream << "            xdot = (double *) calloc(nx, sizeof(double));\n";
    fmuComponentHppStream << "            if (nz>0)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                z    =  (double *) calloc(nz, sizeof(double));\n";
    fmuComponentHppStream << "                prez =  (double *) calloc(nz, sizeof(double));\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            // set the start time and initialize\n";
    fmuComponentHppStream << "            fmiFlag =  mFMU.setTime(c, t0);\n";
    fmuComponentHppStream << "            fmiFlag =  mFMU.initialize(c, toleranceControlled, t0, &eventInfo);\n\n";
    fmuComponentHppStream << "            z = new double;\n";
    fmuComponentHppStream << "            prez = new double;\n";
    fmuComponentHppStream << "            *z = NULL;\n";
    fmuComponentHppStream << "            *prez = NULL;\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void simulateOneTimestep()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            ScalarVariable** vars = mFMU.modelDescription->modelVariables;\n";
    fmuComponentHppStream << "            double value;\n";
    fmuComponentHppStream << "            ScalarVariable* sv;\n";
    fmuComponentHppStream << "            fmiValueReference vr;\n\n";
    fmuComponentHppStream << "            //write input values\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "input")
        {
            fmuComponentHppStream << "            if(mpIn"+numStr+"->isConnected())\n";
            fmuComponentHppStream << "            {\n";
            fmuComponentHppStream << "                sv = vars["+numStr+"];\n";
            fmuComponentHppStream << "                vr = getValueReference(sv);\n";
            fmuComponentHppStream << "                value = (*mpND_in"+numStr+");\n";
            fmuComponentHppStream << "                mFMU.setReal(c, &vr, 1, &value);\n\n";
            fmuComponentHppStream << "            }\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "            //run simulation\n";
    fmuComponentHppStream << "            simulateFMU();\n\n";
    fmuComponentHppStream << "            //write back output values\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    while (!varElement.isNull())
    {
        QString numStr;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality") || varElement.attribute("causality") == "output")
        {

            fmuComponentHppStream << "            sv = vars["+numStr+"];\n";
            fmuComponentHppStream << "            vr = getValueReference(sv);\n";
            fmuComponentHppStream << "            mFMU.getReal(c, &vr, 1, &value);\n";
            fmuComponentHppStream << "            (*mpND_out"+numStr+") = value;\n\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuComponentHppStream << "        }\n";
    fmuComponentHppStream << "        void finalize()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            //cleanup\n";
    fmuComponentHppStream << "            mFMU.terminate(c);\n";
    fmuComponentHppStream << "            mFMU.freeModelInstance(c);\n";
    fmuComponentHppStream << "            if (x!=NULL) free(x);\n";
    fmuComponentHppStream << "            if (xdot!= NULL) free(xdot);\n";
    fmuComponentHppStream << "            if (z!= NULL) free(z);\n";
    fmuComponentHppStream << "            if (prez!= NULL) free(prez);\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        bool loadDll(std::string dllPath)\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            bool success = true;\n";
    fmuComponentHppStream << "            HANDLE h;\n";
    fmuComponentHppStream << "            std::string libdir = dllPath;\n";
    fmuComponentHppStream << "            while(libdir.at(libdir.size()-1) != '/')\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "            libdir.erase(libdir.size()-1, 1);\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            SetDllDirectoryA(libdir.c_str());       //Set search path for dependencies\n";
    fmuComponentHppStream << "            h = LoadLibraryA(dllPath.c_str());\n";
    fmuComponentHppStream << "            if (!h)\n";
    fmuComponentHppStream << "            {\n";
    //fmuComponentHppStream << "                qDebug() << QString(\"error: Could not load dll\\n\");\n";
    fmuComponentHppStream << "                success = false; // failure\n";
    fmuComponentHppStream << "                return success;\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            mFMU.dllHandle = h;\n\n";
    fmuComponentHppStream << "            mFMU.getModelTypesPlatform   = (fGetModelTypesPlatform) getAdr(&success, \"fmiGetModelTypesPlatform\");\n";
    fmuComponentHppStream << "            mFMU.instantiateModel        = (fInstantiateModel)   getAdr(&success, \"fmiInstantiateModel\");\n";
    fmuComponentHppStream << "            mFMU.freeModelInstance       = (fFreeModelInstance)  getAdr(&success, \"fmiFreeModelInstance\");\n";
    fmuComponentHppStream << "            mFMU.setTime                 = (fSetTime)            getAdr(&success, \"fmiSetTime\");\n";
    fmuComponentHppStream << "            mFMU.setContinuousStates     = (fSetContinuousStates)getAdr(&success, \"fmiSetContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.completedIntegratorStep = (fCompletedIntegratorStep)getAdr(&success, \"fmiCompletedIntegratorStep\");\n";
    fmuComponentHppStream << "            mFMU.initialize              = (fInitialize)         getAdr(&success, \"fmiInitialize\");\n";
    fmuComponentHppStream << "            mFMU.getDerivatives          = (fGetDerivatives)     getAdr(&success, \"fmiGetDerivatives\");\n";
    fmuComponentHppStream << "            mFMU.getEventIndicators      = (fGetEventIndicators) getAdr(&success, \"fmiGetEventIndicators\");\n";
    fmuComponentHppStream << "            mFMU.eventUpdate             = (fEventUpdate)        getAdr(&success, \"fmiEventUpdate\");\n";
    fmuComponentHppStream << "            mFMU.getContinuousStates     = (fGetContinuousStates)getAdr(&success, \"fmiGetContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.getNominalContinuousStates = (fGetNominalContinuousStates)getAdr(&success, \"fmiGetNominalContinuousStates\");\n";
    fmuComponentHppStream << "            mFMU.getStateValueReferences = (fGetStateValueReferences)getAdr(&success, \"fmiGetStateValueReferences\");\n";
    fmuComponentHppStream << "            mFMU.terminate               = (fTerminate)          getAdr(&success, \"fmiTerminate\");\n\n";
    fmuComponentHppStream << "            mFMU.getVersion              = (fGetVersion)         getAdr(&success, \"fmiGetVersion\");\n";
    fmuComponentHppStream << "            mFMU.setDebugLogging         = (fSetDebugLogging)    getAdr(&success, \"fmiSetDebugLogging\");\n";
    fmuComponentHppStream << "            mFMU.setReal                 = (fSetReal)            getAdr(&success, \"fmiSetReal\");\n";
    fmuComponentHppStream << "            mFMU.setInteger              = (fSetInteger)         getAdr(&success, \"fmiSetInteger\");\n";
    fmuComponentHppStream << "            mFMU.setBoolean              = (fSetBoolean)         getAdr(&success, \"fmiSetBoolean\");\n";
    fmuComponentHppStream << "            mFMU.setString               = (fSetString)          getAdr(&success, \"fmiSetString\");\n";
    fmuComponentHppStream << "            mFMU.getReal                 = (fGetReal)            getAdr(&success, \"fmiGetReal\");\n";
    fmuComponentHppStream << "            mFMU.getInteger              = (fGetInteger)         getAdr(&success, \"fmiGetInteger\");\n";
    fmuComponentHppStream << "            mFMU.getBoolean              = (fGetBoolean)         getAdr(&success, \"fmiGetBoolean\");\n";
    fmuComponentHppStream << "            mFMU.getString               = (fGetString)          getAdr(&success, \"fmiGetString\");\n";
    fmuComponentHppStream << "            return success;\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void* getAdr(bool* success, const char* functionName)\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            char name[BUFSIZE];\n";
    fmuComponentHppStream << "            void* fp;\n";
    fmuComponentHppStream << "            ModelDescription *me = mFMU.modelDescription;\n";
    fmuComponentHppStream << "            sprintf(name, \"%s_%s\", getModelIdentifier(me), functionName);\n";
    fmuComponentHppStream << "            fp = (void*)GetProcAddress((HINSTANCE__*)mFMU.dllHandle, name);\n";
    fmuComponentHppStream << "            //fp = (void*)GetProcAddress((HINSTANCE)fmu->dllHandle, name);        //CASTINGS MAY NOT WORK!!!\n";
    fmuComponentHppStream << "            if (!fp) {\n";
    fmuComponentHppStream << "                *success = false; // mark dll load as 'failed'\n";
    fmuComponentHppStream << "            }\n";
    fmuComponentHppStream << "            return fp;\n";
    fmuComponentHppStream << "        }\n\n";
    fmuComponentHppStream << "        void simulateFMU()\n";
    fmuComponentHppStream << "        {\n";
    fmuComponentHppStream << "            int i;                          // For loop index\n";
    fmuComponentHppStream << "            fmiBoolean timeEvent, stateEvent, stepEvent;\n";
    fmuComponentHppStream << "            fmiStatus fmiFlag;               // return code of the fmu functions\n\n";
    fmuComponentHppStream << "            if (eventInfo.terminateSimulation)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                stopSimulation();\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            //Simulate one step\n\n";
    fmuComponentHppStream << "            // get current state and derivatives\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getContinuousStates(c, x, nx);\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getDerivatives(c, xdot, nx);\n\n";
    fmuComponentHppStream << "            // advance time\n";
    fmuComponentHppStream << "            timeEvent = eventInfo.upcomingTimeEvent && eventInfo.nextEventTime < mTime;\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.setTime(c, mTime);\n\n";
    fmuComponentHppStream << "            // perform one step\n";
    fmuComponentHppStream << "            for (i=0; i<nx; i++) x[i] += mTimestep*xdot[i]; // forward Euler method\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.setContinuousStates(c, x, nx);\n\n";
    fmuComponentHppStream << "            // Check for step event, e.g. dynamic state selection\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.completedIntegratorStep(c, &stepEvent);\n\n";
    fmuComponentHppStream << "            // Check for state event\n";
    fmuComponentHppStream << "            for (i=0; i<nz; i++) prez[i] = z[i];\n";
    fmuComponentHppStream << "            fmiFlag = mFMU.getEventIndicators(c, z, nz);\n";
    fmuComponentHppStream << "            stateEvent = FALSE;\n";
    fmuComponentHppStream << "            for (i=0; i<nz; i++)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                stateEvent = stateEvent || (prez[i] * z[i] < 0);\n";
    fmuComponentHppStream << "            }\n\n";
    fmuComponentHppStream << "            //! @todo Event criteria are disabled for now, so there will be a time event every time step no matter what.\n\n";
    fmuComponentHppStream << "            // handle events\n";
    fmuComponentHppStream << "            if (timeEvent || stateEvent || stepEvent)\n";
    fmuComponentHppStream << "            {\n";
    fmuComponentHppStream << "                // event iteration in one step, ignoring intermediate results\n";
    fmuComponentHppStream << "                fmiFlag = mFMU.eventUpdate(c, fmiFalse, &eventInfo);\n";
    fmuComponentHppStream << "                // terminate simulation, if requested by the model\n";
    fmuComponentHppStream << "                if (eventInfo.terminateSimulation)\n";
    fmuComponentHppStream << "                {\n";
    fmuComponentHppStream << "                    stopSimulation();\n";
    fmuComponentHppStream << "                }\n";
    fmuComponentHppStream << "            } // if event\n";
    fmuComponentHppStream << "        }\n";
    fmuComponentHppStream << "    };\n";
    fmuComponentHppStream << "}\n\n";
    fmuComponentHppStream << "#endif // "+fmuName+"_H\n";
    fmuComponentHppFile.close();


    progressBar.setValue(5);
    progressBar.setLabelText("Writing " + fmuName + ".xml");
    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Writing "+fmuName+".xml");

    //Create <fmuname>.xml
    //! @todo Use dom elements for generating xml (this is just stupid)
    QFile fmuXmlFile;
    fmuXmlFile.setFileName(fmuDir.path() + "/" + fmuName + ".xml");
    if(!fmuXmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Could not open "+fmuName+".xml for writing.");
        removeDir(fmuDir.path());
        return;
    }

    QTextStream fmuXmlStream(&fmuXmlFile);
    fmuXmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    fmuXmlStream << "<hopsanobjectappearance version=\"0.2\">\n";
    fmuXmlStream << "    <modelobject typename=\""+fmuName+"\" displayname=\""+fmuName+"\">\n";
    fmuXmlStream << "        <icons>\n";
    fmuXmlStream << "            <icon type=\"user\" path=\"fmu.svg\" iconrotation=\"ON\" scale=\"1.0\"/>\n";
    fmuXmlStream << "        </icons>\n";
    fmuXmlStream << "        <portpositions>\n";
    varElement = variablesElement.firstChildElement("ScalarVariable");
    i=0;
    double inputPosStep=1.0/(nInputs+1.0);      //These 4 variables are used for port positioning
    double outputPosStep=1.0/(nOutputs+1.0);
    double inputPos=0;
    double outputPos=0;
    while (!varElement.isNull())
    {
        QString numStr, numStr2;
        numStr.setNum(i);
        if(!varElement.hasAttribute("causality"))
        {
            inputPos += inputPosStep;
            numStr2.setNum(inputPos);
            fmuXmlStream << "            <portpose name=\""+varElement.attribute("name")+"In\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
            outputPos += outputPosStep;
            numStr2.setNum(outputPos);
            fmuXmlStream << "            <portpose name=\""+varElement.attribute("name")+"Out\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
        }
        else if(varElement.attribute("causality") == "input")
        {
            inputPos += inputPosStep;
            numStr2.setNum(inputPos);
            fmuXmlStream << "            <portpose name=\""+varElement.attribute("name")+"\" x=\"0.0\" y=\""+numStr2+"\" a=\"180\"/>\n";
        }
        else if(varElement.attribute("causality") == "output")
        {
            outputPos += outputPosStep;
            numStr2.setNum(outputPos);
            fmuXmlStream << "            <portpose name=\""+varElement.attribute("name")+"\" x=\"1.0\" y=\""+numStr2+"\" a=\"0\"/>\n";
        }
        ++i;
        varElement = varElement.nextSiblingElement("ScalarVariable");
    }
    fmuXmlStream << "        </portpositions>\n";
    fmuXmlStream << "    </modelobject>\n";
    fmuXmlStream << "</hopsanobjectappearance>\n";
    fmuXmlFile.close();


    progressBar.setValue(5.5);


    //Move FMI source files to compile directory
    QFile simSupportSourceFile;
    simSupportSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/sim_support.c");
    if(simSupportSourceFile.copy(fmuDir.path() + "/sim_support.c"))
    {
        progressBar.setLabelText("Copying sim_support.c");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + simSupportSourceFile.fileName() + " to " + fmuDir.path() + "/sim_support.c");
    }

    QFile stackSourceFile;
    stackSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/stack.cc");
    if(stackSourceFile.copy(fmuDir.path() + "/stack.cc"))
    {
        progressBar.setLabelText("Copying stack.cc");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + stackSourceFile.fileName() + " to " + fmuDir.path() + "/stack.cc");
    }

    QFile xmlParserSourceFile;
    xmlParserSourceFile.setFileName(gExecPath + "../ThirdParty/fmi/xml_parser.h");
    if(xmlParserSourceFile.copy(fmuDir.path() + "/xml_parser.h"))
    {
        progressBar.setLabelText("Copying xml_parser.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + xmlParserSourceFile.fileName() + " to " + fmuDir.path() + "/xml_parser.h");
    }

    QFile simSupportHeaderFile;
    simSupportHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/sim_support.h");
    if(simSupportHeaderFile.copy(fmuDir.path() + "/sim_support.h"))
    {
        progressBar.setLabelText("Copying sim_support.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + simSupportHeaderFile.fileName() + " to " + fmuDir.path() + "/sim_support.h");
    }

    QFile stackHeaderFile;
    stackHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/stack.h");
    if(stackHeaderFile.copy(fmuDir.path() + "/stack.h"))
    {
        progressBar.setLabelText("Copying stack.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + stackHeaderFile.fileName() + " to " + fmuDir.path() + "/stack.h");
    }

    QFile xmlParserHeaderFile;
    xmlParserHeaderFile.setFileName(gExecPath + "../ThirdParty/fmi/xml_parser.cc");
    if(xmlParserHeaderFile.copy(fmuDir.path() + "/xml_parser.cc"))
    {
        progressBar.setLabelText("Copying xml_parser.cc");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + xmlParserHeaderFile.fileName() + " to " + fmuDir.path() + "/xml_parser.cc");
    }

    QFile expatFile;
    expatFile.setFileName(gExecPath + "../ThirdParty/fmi/expat.h");
    if(expatFile.copy(fmuDir.path() + "/expat.h"))
    {
        progressBar.setLabelText("Copying expat.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + expatFile.fileName() + " to " + fmuDir.path() + "/expat.h");
    }

    QFile expatExternalFile;
    expatExternalFile.setFileName(gExecPath + "../ThirdParty/fmi/expat_external.h");
    if(expatExternalFile.copy(fmuDir.path() + "/expat_external.h"))
    {
        progressBar.setLabelText("Copying expat_external.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + expatExternalFile.fileName() + " to " + fmuDir.path() + "/expat_external.h");
    }

    QFile libExpatAFile;
    libExpatAFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpat.a");
    if(libExpatAFile.copy(fmuDir.path() + "/libexpat.a"))
    {
        progressBar.setLabelText("Copying libexpat.a");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + libExpatAFile.fileName() + " to " + fmuDir.path() + "/libexpat.a");
    }

    QFile libExpatDllFile;
    libExpatDllFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpat.dll");
    if(libExpatDllFile.copy(fmuDir.path() + "/libexpat.dll"))
    {
        progressBar.setLabelText("Copying libexpat.dll");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + libExpatDllFile.fileName() + " to " + fmuDir.path() + "/libexpat.dll");
    }

    QFile libExpatwAFile;
    libExpatwAFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpatw.a");
    if(libExpatwAFile.copy(fmuDir.path() + "/libexpatw.a"))
    {
        progressBar.setLabelText("Copying libexpatw.a");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + libExpatwAFile.fileName() + " to " + fmuDir.path() + "/libexpatw.a");
    }

    QFile libExpatwDllFile;
    libExpatwDllFile.setFileName(gExecPath + "../ThirdParty/fmi/libexpatw.dll");
    if(libExpatwDllFile.copy(fmuDir.path() + "/libexpatw.dll"))
    {
        progressBar.setLabelText("Copying libexpatw.dll");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + libExpatwDllFile.fileName() + " to " + fmuDir.path() + "/libexpatw.dll");
    }

    QFile fmiMeFile;
    fmiMeFile.setFileName(gExecPath + "../ThirdParty/fmi/fmi_me.h");
    if(fmiMeFile.copy(fmuDir.path() + "/fmi_me.h"))
    {
        progressBar.setLabelText("Copying fmi_me.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + fmiMeFile.fileName() + " to " + fmuDir.path() + "/fmi_me.h");
    }

    QFile fmiModelFunctionsFile;
    fmiModelFunctionsFile.setFileName(gExecPath + "../ThirdParty/fmi/fmiModelFunctions.h");
    if(fmiModelFunctionsFile.copy(fmuDir.path() + "/fmiModelFunctions.h"))
    {
        progressBar.setLabelText("Copying fmiModelFunctions.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + fmiModelFunctionsFile.fileName() + " to " + fmuDir.path() + "/fmiModelFunctions.h");
    }

    QFile fmiModelTypesFile;
    fmiModelTypesFile.setFileName(gExecPath + "../ThirdParty/fmi/fmiModelTypes.h");
    if(fmiModelTypesFile.copy(fmuDir.path() + "/fmiModelTypes.h"))
    {
        progressBar.setLabelText("Copying fmiModelTypes.h");
        gpMainWindow->mpMessageWidget->printGUIInfoMessage("Copying " + fmiModelTypesFile.fileName() + " to " + fmuDir.path() + "/fmiModelTypes.h");
    }


    progressBar.setValue(6);
    progressBar.setLabelText("Writing compilation script");


    //Create compilation script file
    QFile clBatchFile;
    clBatchFile.setFileName(fmuDir.path() + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Could not open compile.bat for writing.");
        removeDir(fmuDir.path());
        return;
    }
    QTextStream clBatchStream(&clBatchFile);
    clBatchStream << "g++.exe -shared fmuLib.cc stack.cc xml_parser.cc -o fmuLib.dll -L../../../bin/ -lHopsanCore -L./ -llibexpat\n";
    clBatchFile.close();


    progressBar.setLabelText("Compiling " + fmuName + ".dll");
    progressBar.setValue(6.5);


    //Call compilation script file
    QProcess gccProcess;
    gccProcess.start("cmd.exe", QStringList() << "/c" << "cd " + fmuDir.path() + " & compile.bat");
    gccProcess.waitForFinished();
    progressBar.setValue(8);
    QByteArray gccResult = gccProcess.readAll();
    QList<QByteArray> gccResultList = gccResult.split('\n');
    for(int i=0; i<gccResultList.size(); ++i)
    {
        QString msg = gccResultList.at(i);
        msg = msg.remove(msg.size()-1, 1);
        if(!msg.isEmpty())
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage(msg);
        }
    }

    if(!fmuDir.exists(fmuName + ".dll"))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Import of FMU failed: Compilation error.");
        removeDir(fmuDir.path());
        return;
    }


    progressBar.setLabelText("Removing temporary files");
    progressBar.setValue(9);


    //Cleanup temporary files
//    fmuDir.remove("sim_support.h");
//    fmuDir.remove("sim_support.c");
//    fmuDir.remove("stack.h");
//    fmuDir.remove("stack.cc");
//    fmuDir.remove("xml_parser.h");
//    fmuDir.remove("xml_parser.cc");
//    fmuDir.remove("expat.h");
//    fmuDir.remove("expat_external.h");
//    fmuDir.remove("fmi_me.h");
//    fmuDir.remove("fmiModelFunctions.h");
//    fmuDir.remove("fmiModelTypes.h");
//    fmuDir.remove("compile.bat");
//    fmuComponentHppFile.remove();
//    fmuLibFile.remove();
//    fmuDir.rmdir("component_code");
//    QDir binDir;
//    binDir.setPath(fmuDir.path() + "/binaries");
//    binDir.rmdir("win32");
//    fmuDir.rmdir("binaries");


    progressBar.setLabelText("Loading FMU library");
    progressBar.setValue(9.5);


    //Load the library
    fmuDir.cdUp();
    if(gConfig.hasUserLib(fmuDir.path()))
    {
        gConfig.removeUserLib(fmuDir.path());
        mpContentsTree->findChild("External Libraries")->removeChild(fmuDir.path());
        update();
    }
    gConfig.addUserLib(fmuDir.path());     //Register new library in configuration
    loadExternalLibrary(fmuDir.path());    //Load the library



    progressBar.setValue(10);
    progressBar.setLabelText("Finished!");
    progressBar.setCancelButtonText("Okay!");
}



//! @brief Wrapper function that loads an external library
//! @param libDir Directory to the library
//! @todo Why do we need this?
void LibraryWidget::loadExternalLibrary(QString libDir)
{
    qDebug() << "LOADING Library dir " << libDir;
    gConfig.addUserLib(libDir);     //Register new library in configuration
    loadLibrary(libDir, true);
}

//! @brief Load contents (xml files) of dir into SecretHidden library map that is not vissible in the libary
//! @todo Lots of duplicate code in this function and otehr load function, should try to break out common sub help functions
void LibraryWidget::loadHiddenSecretDir(QString dir)
{
    QDir libDirObject(dir);

    // Append components
    QStringList filters;
    filters << "*.xml";                     //Create the name filter
    libDirObject.setFilter(QDir::NoFilter);
    libDirObject.setNameFilters(filters);   //Set the name filter

    QStringList xmlList  = libDirObject.entryList();    //Create a list with all name of the files in dir libDir
    for (int i=0; i<xmlList.size(); ++i)        //Iterate over the file names
    {
        QString filename = dir + "/" + xmlList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file or not a text file: " + filename);
            continue;
        }

        ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;

        QDomDocument domDocument;        //Read appearance from file, First check if xml
        QString errorStr;
        int errorLine, errorColumn;
        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            //! @todo check caf version
            QDomElement cafRoot = domDocument.documentElement();
            if (cafRoot.tagName() != CAF_ROOT)
            {
                gpMainWindow->mpMessageWidget->printGUIDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
                continue;
            }
            else
            {
                //Read appearance data from the caf xml file, begin with the first
                QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearace objects from same file
                pAppearanceData->setBasePath(dir + "/");
                pAppearanceData->readFromDomElement(xmlModelObjectAppearance);
            }
        }
        else
        {
            gpMainWindow->mpMessageWidget->printGUIDebugMessage(file.fileName() + ": The file is not an Hopsan ComponentAppearance Data file.");
            continue;
        }

        bool success = true;
        //! @todo maybe we need to check appearance data for a minimuma amount of necessary data
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
                //Check that component is registered in core
            success = mpCoreAccess->hasComponent(pAppearanceData->getTypeName()); //Check so that there is such component availible in the Core
            if (!success)
            {
                gpMainWindow->mpMessageWidget->printGUIWarningMessage("When loading graphics, ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)", "componentnotregistered");
            }
        }

        if (success)
        {
            mpSecretHiddenContentsTree->addComponent(pAppearanceData);
        }

        //Close file
        file.close();
    }

}


//! @brief Recursive function that searches through subdirectories and adds components to the library contents tree
//! @param libDir Current directory
//! @param pParentTree Current contents tree node
void LibraryWidget::loadLibraryFolder(QString libDir, LibraryContentsTree *pParentTree)
{
    //qDebug() << "loadLibraryFolder() " << libDir;

    QDir libDirObject(libDir);
    if(!libDirObject.exists() && gConfig.hasUserLib(libDir))
    {
        gConfig.removeUserLib(libDir);      //Remove user lib if it does not exist
    }

    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));

    //qDebug() << "Adding tree entry: " << libName;
    LibraryContentsTree *pTree = pParentTree->addChild(libName);        //Create the node
    pTree->mLibDir = libDir;
    libName = pTree->mName; //Reset name vaariable to new unique name

        // Load DLL or SO files
    QStringList filters;
    #ifdef WIN32
        filters << "*.dll";
    #else
        filters << "*.so";
    #endif
    libDirObject.setNameFilters(filters);
    QStringList libList = libDirObject.entryList();
    bool success=false;
    for (int i = 0; i < libList.size(); ++i)
    {
        QString filename = libDir + "/" + libList.at(i);
        qDebug() << "Trying to load: " << filename << " in Core";
        if(mpCoreAccess->loadComponentLib(filename))
        {
            success=true;
            pTree->mLoadedLibraryDLLs.append(filename); // Remember dlls loaded in this subtree
        }
    }

    if(!success && libList.size()>0)
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage(libDirObject.path() + ": Could not find any working Hopsan library in specified folder!");
        gpMainWindow->mpMessageWidget->checkMessages();
        pParentTree->removeChild(libName);
        gConfig.removeUserLib(libDirObject.path());
        delete pTree;
        return;     //No point in continuing since no library was found
    }
    gpMainWindow->mpMessageWidget->checkMessages();

    // Load XML files and recursively load subfolder

    //Append subnodes recursively
    libDirObject.setFilter(QDir::AllDirs);
    QStringList subDirList = libDirObject.entryList();
    subDirList.removeAll(".");
    subDirList.removeAll("..");
    subDirList.removeAll(".svn");
    for(int i=0; i<subDirList.size(); ++i)
    {
        loadLibraryFolder(libDir+"/"+subDirList.at(i), pTree);
    }

    //Append components
    filters.clear();
    filters << "*.xml";                     //Create the name filter
    libDirObject.setFilter(QDir::NoFilter);
    libDirObject.setNameFilters(filters);   //Set the name filter

    //! @todo Reusing variable libList for xml files as well is not nice programming. Make an xmlList object instead!
    libList  = libDirObject.entryList();    //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)        //Iterate over the file names
    {
        QString filename = libDir + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file or not a text file: " + filename);
            continue;
        }

        ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;

        QDomDocument domDocument;        //Read appearance from file, First check if xml
        QString errorStr;
        int errorLine, errorColumn;
        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            //! @todo check caf version
            QDomElement cafRoot = domDocument.documentElement();
            if (cafRoot.tagName() != CAF_ROOT)
            {
                //QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData"),
//                                         "The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: "
//                                         + cafRoot.tagName() + "!=" + CAF_ROOT);
                gpMainWindow->mpMessageWidget->printGUIDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
                continue;
            }
            else
            {
                //Read appearance data from the caf xml file, begin with the first
                QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearace objects from same file, aslo not hardcode tagnames
                pAppearanceData->setBasePath(libDir + "/");
                pAppearanceData->readFromDomElement(xmlModelObjectAppearance);

                // Check CAF version, and ask user if they want to update to latest version
                QString caf_version = cafRoot.attribute(CAF_VERSION);

                if (caf_version < CAF_VERSIONNUM)
                {
                    bool doSave=false;
                    if (mUpConvertAllCAF==UNDECIDED_TO_ALL)
                    {
                        QMessageBox questionBox(this);
                        QString text;
                        QTextStream ts(&text);
                        ts << file.fileName() << "\n"
                           << "Your appearance data file format is old! " << caf_version << "<" << CAF_VERSIONNUM << " Do you want to auto update to the latest format?\n\n"
                           << "NOTE! You should take a backup of your files BEFORE answering Yes to this question!!\n"
                           << "NOTE! All non standard Hopsan contents will be lost\n"
                           << "NOTE! Attributes may change order within a tag (but the order is not important)\n\n"
                           << "If you want to update manually, see the documantation about the latest format version.";
                        questionBox.setWindowTitle("Your appearance data file format is old!");
                        questionBox.setText(text);
                        QPushButton* pYes = questionBox.addButton(QMessageBox::Yes);
                        questionBox.addButton(QMessageBox::No);
                        QPushButton* pYesToAll = questionBox.addButton(QMessageBox::YesToAll);
                        QPushButton* pNoToAll = questionBox.addButton(QMessageBox::NoToAll);
                        questionBox.setDefaultButton(QMessageBox::No);
                        questionBox.exec();
                        QAbstractButton* pClickedButton = questionBox.clickedButton();

                        if ( (pClickedButton == pYes) || (pClickedButton == pYesToAll) )
                        {
                            doSave = true;
                        }

                        if (pClickedButton == pYesToAll)
                        {
                            mUpConvertAllCAF = YES_TO_ALL;
                        }
                        else if (pClickedButton == pNoToAll)
                        {
                            mUpConvertAllCAF = NO_TO_ALL;
                        }
                    }
                    else if (mUpConvertAllCAF==YES_TO_ALL)
                    {
                        doSave = true;
                    }

                    if (doSave)
                    {
                        //Close file
                        file.close();

                        pAppearanceData->saveToXMLFile(file.fileName());

                    }
                }
            }
        }
        else
        {
            QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData in %4"),
                                     QString(file.fileName() + "Parse error at line %1, column %2:\n%3")
                                     .arg(errorLine)
                                     .arg(errorColumn)
                                     .arg(errorStr)
                                     .arg(file.fileName()));

            //! @todo give smart warning message, this is not an xml file

            continue;
        }

        //Close file
        file.close();

        //! @todo maybe use the convenient helpfunction for the stuff above (open file and check xml and root tagname) now that we have one

        bool success = true;

        //! @todo maybe we need to check appearance data for a minimuma amount of necessary data
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
                //Check that component is registered in core
            success = mpCoreAccess->hasComponent(pAppearanceData->getTypeName()); //Check so that there is such component availible in the Core
            if (!success)
            {
                gpMainWindow->mpMessageWidget->printGUIWarningMessage("When loading graphics, ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)", "componentnotregistered");
            }
        }

        if (success)
        {
            pTree->addComponent(pAppearanceData);
            mLoadedComponents << pAppearanceData->getTypeName();
            qDebug() << "Adding: " << pAppearanceData->getTypeName();
        }
    }

    //Make sure empty external libraries are not loaded (because they would become invisible and not removeable)
    if(pTree->isEmpty())
    {
        pParentTree->removeChild(libName);
        if(gConfig.hasUserLib(libDir))
        {
            gConfig.removeUserLib(libDir);
        }
        delete pTree;
    }
}


void LibraryWidget::unloadExternalLibrary(QString libName)
{
    if(gConfig.hasUserLib(libName))
    {
        gConfig.removeUserLib(libName);
        if(mpContentsTree->findChild("External Libraries")->findChild(libName))
        {
            unLoadLibrarySubTree(mpContentsTree->findChild("External Libraries")->findChild(libName));
        }
        update();
    }
}


void LibraryWidget::unLoadLibrarySubTree(LibraryContentsTree *pTree)
{
    //First call unload on all dlls in core
    for (int i=0; i<pTree->mLoadedLibraryDLLs.size(); ++i)
    {
        mpCoreAccess->unLoadComponentLib(pTree->mLoadedLibraryDLLs[i]);
    }
    //Then remove the tree itself
    mpContentsTree->findChild("External Libraries")->removeChild(pTree->mName);
    gpMainWindow->mpMessageWidget->checkMessages();
}

//! @brief Slot that sets view mode to single tree and redraws the library
void LibraryWidget::setListView()
{
    gConfig.setLibraryStyle(0);
    mViewMode=0;
    update();
}


//! @brief Slot that sets view mode to dual mode and redraws the library
void LibraryWidget::setDualView()
{
    gConfig.setLibraryStyle(1);
    mViewMode=1;
    update();
}


void LibraryWidget::contextMenuEvent(QContextMenuEvent *event)
{
    //Lookup and error checks
    QTreeWidgetItem *pItem = mpTree->itemAt(mpTree->mapFromParent(event->pos()));
    if(pItem == 0) return;
    if(!mTreeItemToContentsTreeMap.contains(pItem)) return;
    LibraryContentsTree *pTree = mTreeItemToContentsTreeMap.find(pItem).value();
    if(pTree == 0) return;

    QMenu menu;

    QAction *pOpenContainingFolder = new QAction(this);
    if(pTree->mName != "External Libraries")
    {
        pOpenContainingFolder = menu.addAction("Open Containing Folder");
    }

    QAction *pUnloadLibraryFolder = new QAction(this);
    if(pItem->parent() != 0 && pItem->parent()->text(0) == "External Libraries")
    {
        pUnloadLibraryFolder = menu.addAction("Unload External Library");
    }


    //-- User interaction --//
    QAction *pSelectedAction = menu.exec(mapToGlobal(event->pos()));
    //----------------------//


    if(pSelectedAction == pOpenContainingFolder)
    {
        QString path = QDir::toNativeSeparators(pTree->mLibDir);
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }

    if(pSelectedAction == pUnloadLibraryFolder)
    {
        gConfig.removeUserLib(pTree->mLibDir);
        unLoadLibrarySubTree(mpContentsTree->findChild("External Libraries")->findChild(pTree->mName));
        update();
    }

    QWidget::contextMenuEvent(event);
}


void LibraryWidget::mouseMoveEvent(QMouseEvent *event)
{
    mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();

    //qDebug() << "You are hovering me!";

    QWidget::mouseMoveEvent(event);
}


//! @brief Constructor for library list widget
//! This is the box with icons which is used in dual view mode
//! @param parent Pointer to parent (library widget)
LibraryListWidget::LibraryListWidget(LibraryWidget *parent)
    : QListWidget(parent)
{
    mpLibraryWidget = parent;
    setMouseTracking(true);
    setSelectionRectVisible(false);
}


//! @brief Reimplementation of mouse move event
//! Used to update the component name display field while hovering the icons.
//! @param event Contains information about the event
void LibraryListWidget::mouseMoveEvent(QMouseEvent *event)
{
    QListWidgetItem *tempItem = itemAt(event->pos());
    if(tempItem != 0)
    {
        gpMainWindow->showHelpPopupMessage("Add a component by dragging it to the workspace.");

        QString componentName;
        componentName = mpLibraryWidget->mListItemToContentsMap.find(tempItem).value()->getName();

        //Change name in component name field. Resize the text if needed, so that the library widget does not change size.
        mpLibraryWidget->mpComponentNameField->setMaximumWidth(this->width());
        mpLibraryWidget->mpComponentNameField->setMinimumHeight(mpLibraryWidget->mpComponentNameField->height());
        mpLibraryWidget->mpComponentNameField->setFont(QFont(mpLibraryWidget->mpComponentNameField->font().family(), min(10.0, .9*mpLibraryWidget->width()/(0.615*componentName.size()))));
        mpLibraryWidget->mpComponentNameField->setText(componentName);
    }
    QListWidget::mouseMoveEvent(event);
}


//! @brief Retrieves the appearance data for a given type name.
//! @param componentType Type name of the component
ModelObjectAppearance *LibraryWidget::getAppearanceData(QString componentType)
{
    LibraryComponent* pLibComp = mpContentsTree->findComponent(componentType);
    if(pLibComp == 0)
    {
        // If we cant find in ordinary tree, then try the secret hidden tree
        pLibComp = mpSecretHiddenContentsTree->findComponent(componentType);
        if (pLibComp == 0)
        {
            // Nothing found return NULL ptr
            return 0;
        }
    }
    // If we found something then return appearance data
    return pLibComp->getAppearanceData();
}


//! @brief Selects graphics type to be used in library (iso or user).
void LibraryWidget::setGfxType(graphicsType gfxType)
{
    mGfxType = gfxType;
    update();       //Redraw the library
}


//! @brief Constructor for a library contents tree node.
//! @param name Name of the node (empty for top node, which is never displayed)
LibraryContentsTree::LibraryContentsTree(QString name)
{
    mName = name;
}


//! @brief Returns whether or not a tree node is empty (contains no child nodes and no components)
bool LibraryContentsTree::isEmpty()
{
    return (mChildNodesPtrs.isEmpty() && mComponentPtrs.isEmpty());
}


//! @brief Adds a new child node to a node.
//! @param name Name of new child node
//! @returns Pointer to the new child node
LibraryContentsTree *LibraryContentsTree::addChild(QString name)
{
    if(findChild(name))
    {
        QString newName = name.append("_0");
        int i=1;
        while(findChild(newName))
        {
            QString num;
            num.setNum(i);
            newName.chop(1);
            newName.append(num);
            ++i;
        }
    }
    LibraryContentsTree *pNewChild = new LibraryContentsTree(name);
    mChildNodesPtrs.append(pNewChild);
    return pNewChild;
}

//! @brief Removes specified child node from the node
//! @param name Name of node to remove
bool LibraryContentsTree::removeChild(QString name)
{
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        if(mChildNodesPtrs.at(i)->mName == name)
        {
            mChildNodesPtrs.remove(i);
            return true;
        }
    }
    return false;       //Not found
}

//! @brief Returns a pointer to sub node with specified name, or zero if it does not exist.
//! @param name Name to look for
LibraryContentsTree *LibraryContentsTree::findChild(QString name)
{
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        if(mChildNodesPtrs.at(i)->mName == name)
            return mChildNodesPtrs.at(i);
    }
    return 0;
}


//! @brief Adds a new component to a node from an appearance data object.
//! @param pAppearanceData Appearance data object to use
//! @returns Pointer to the new component
LibraryComponent *LibraryContentsTree::addComponent(ModelObjectAppearance *pAppearanceData)
{
    LibraryComponent *pNewComponent = new LibraryComponent(pAppearanceData);
    mComponentPtrs.append(pNewComponent);
    return pNewComponent;
}


//! @brief Recursively searches the tree to find component with specified typename.
//! @param typeName Type name to look for
//! @returns Pointer to the component, or zero if not found.
LibraryComponent *LibraryContentsTree::findComponent(QString typeName)
{
    for(int i=0; i<mComponentPtrs.size(); ++i)
    {
        if(mComponentPtrs.at(i)->getTypeName() == typeName)
            return mComponentPtrs.at(i);
    }
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        LibraryComponent *retval = mChildNodesPtrs.at(i)->findComponent(typeName);
        if(retval != 0)
            return retval;
    }
    return 0;
}


//! @brief Constructor for library component class
//! @param pAppearanceData Pointere to appearance data object that is used
LibraryComponent::LibraryComponent(ModelObjectAppearance *pAppearanceData)
{
    mpAppearanceData = pAppearanceData;

    //Load and store component icon
    QString iconPath = mpAppearanceData->getFullAvailableIconPath(USERGRAPHICS);
    QFile iconFile(iconPath);
    if (!iconFile.exists())     //Check if specified file exist, else use unknown icon
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
    mUserIcon.addFile(iconPath,QSize(55,55));
    iconPath = mpAppearanceData->getFullAvailableIconPath(ISOGRAPHICS);
    iconFile.setFileName(iconPath);
    if (!iconFile.exists())     //Check if specified file exist, else use unknown icon
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
    mIsoIcon.addFile(iconPath,QSize(55,55));
}


//! @brief Returns the component's icon.
//! @param graphicsType Graphics type to use (iso or user)
QIcon LibraryComponent::getIcon(graphicsType gfxType)
{
    if(gfxType == USERGRAPHICS)
        return mUserIcon;
    else
        return mIsoIcon;
}


//! @brief Returns the name of the component.
QString LibraryComponent::getName()
{
    return mpAppearanceData->getNonEmptyName();
}


//! @brief Returns the type name of the component.
QString LibraryComponent::getTypeName()
{
    return mpAppearanceData->getTypeName();
}


//! @brief Returns a pointer to the appearance data object used by component.
ModelObjectAppearance *LibraryComponent::getAppearanceData()
{
    return mpAppearanceData;
}
