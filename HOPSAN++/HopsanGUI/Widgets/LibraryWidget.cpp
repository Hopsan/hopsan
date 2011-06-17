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

#include "LibraryWidget.h"
#include "../MainWindow.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "MessageWidget.h"
#include "../GUIObjects/GUIModelObjectAppearance.h"
#include "../Configuration.h"
#include "../common.h"

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
    mpCoreAccess = new CoreLibraryAccess();
    mpContentsTree = new LibraryContentsTree();

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

    mpTreeViewButton = new QToolButton();
    mpTreeViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryTreeView.png"));
    mpTreeViewButton->setIconSize(QSize(24,24));
    mpTreeViewButton->setToolTip(tr("Single List View"));
    mpDualViewButton = new QToolButton();
    mpDualViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryDualView.png"));
    mpDualViewButton->setIconSize(QSize(24,24));
    mpDualViewButton->setToolTip(tr("Dual List View"));
    mpLoadExternalButton = new QToolButton();
    mpLoadExternalButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Open.png"));
    mpLoadExternalButton->setIconSize(QSize(24,24));
    mpLoadExternalButton->setToolTip(tr("Load External Library"));

    connect(mpTreeViewButton, SIGNAL(clicked()), this, SLOT(setListView()));
    connect(mpDualViewButton, SIGNAL(clicked()), this, SLOT(setDualView()));
    connect(mpLoadExternalButton, SIGNAL(clicked()), this, SLOT(addExternalLibrary()));

    mpGrid = new QGridLayout(this);
    mpGrid->addWidget(mpTree,               0,0,1,5);
    mpGrid->addWidget(mpComponentNameField, 1,0,1,5);
    mpGrid->addWidget(mpList,               2,0,1,5);
    mpGrid->addWidget(mpTreeViewButton,     3,0,1,1);
    mpGrid->addWidget(mpDualViewButton,     3,1,1,1);
    mpGrid->addWidget(mpLoadExternalButton, 3,2,1,1);
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
//! @param column Not used (required to be there because signal-slot must match parameters)
void LibraryWidget::showLib(QTreeWidgetItem *item, int column)
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

    //Add components
    for(int i=0; i<tree->mComponentPtrs.size(); ++i)        //Add own components
    {
        QListWidgetItem *tempItem = new QListWidgetItem();
        tempItem->setIcon(tree->mComponentPtrs.at(i)->getIcon(mGfxType));
        mListItemToContentsMap.insert(tempItem, tree->mComponentPtrs.at(i));
        tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        mpList->addItem(tempItem);
    }
    for(int j=0; j<tree->mChildNodesPtrs.size(); ++j)       //Add components from child libraries too
    {
        for(int i=0; i<tree->mChildNodesPtrs.at(j)->mComponentPtrs.size(); ++i)
        {
            QListWidgetItem *tempItem = new QListWidgetItem();
            tempItem->setIcon(tree->mChildNodesPtrs.at(j)->mComponentPtrs.at(i)->getIcon(mGfxType));
            mListItemToContentsMap.insert(tempItem, tree->mChildNodesPtrs.at(j)->mComponentPtrs.at(i));
            tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpList->addItem(tempItem);
        }
    }

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
}


//! @brief Initializes drag operation to workspace from a tree widget item
//! @param item Tree widget item
//! @param dummy Does nothing (must be there due to signal-slot parameter mataching)
void LibraryWidget::initializeDrag(QTreeWidgetItem *item, int dummy)
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


//! @brief Wrapper function that loads an external library
//! @param libDir Directory to the library
//! @todo Why do we need this?
void LibraryWidget::loadExternalLibrary(QString libDir)
{
    loadLibrary(libDir, true);
}


//! @brief Recursive function that searches through subdirectories and adds components to the library contents tree
//! @param libDir Current directory
//! @param pParentTree Current contents tree node
void LibraryWidget::loadLibraryFolder(QString libDir, LibraryContentsTree *pParentTree)
{
    QDir libDirObject(libDir);
    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));


        // Load DLL or SO files
    QStringList filters;
    #ifdef WIN32
        filters << "*.dll";
    #else
        filters << "*.so";
    #endif
    libDirObject.setNameFilters(filters);
    QStringList libList = libDirObject.entryList();
    for (int i = 0; i < libList.size(); ++i)
    {
        QString filename = libDir + "/" + libList.at(i);
        qDebug() << "Trying to load: " << filename << " in Core";
        mpCoreAccess->loadComponent(filename);
    }
    gpMainWindow->mpMessageWidget->checkMessages();


        // Load XML files and recursively load subfolders
    LibraryContentsTree *pTree = pParentTree->addChild(libName);        //Create the node
    pTree->mLibDir = libDir;

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
    filters << "*.xml";                     //Create the name filter
    libDirObject.setFilter(QDir::NoFilter);
    libDirObject.setNameFilters(filters);   //Set the name filter

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

        GUIModelObjectAppearance *pAppearanceData = new GUIModelObjectAppearance;

        QDomDocument domDocument;        //Read appearance from file, First check if xml
        QString errorStr;
        int errorLine, errorColumn;
        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
        {
            //! @todo check caf version
            QDomElement cafRoot = domDocument.documentElement();
            if (cafRoot.tagName() != CAF_ROOTTAG)
            {
                QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData"),
                                         "The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: "
                                         + cafRoot.tagName() + "!=" + CAF_ROOTTAG);
            }
            else
            {
                //Read appearance data from the caf xml file, begin with the first
                QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement("modelobject"); //! @todo extend this code to be able to read many appearace objects from same file, aslo not hardcode tagnames
                pAppearanceData->setBasePath(libDir + "/");
                pAppearanceData->readFromDomElement(xmlModelObjectAppearance);
            }
        }
        else
        {
            QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData in %4"),
                                     tr("Parse error at line %1, column %2:\n%3")
                                     .arg(errorLine)
                                     .arg(errorColumn)
                                     .arg(errorStr)
                                     .arg(file.fileName()));

            //! @todo give smart warning message, this is not an xml file
        }
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
                gpMainWindow->mpMessageWidget->printGUIWarningMessage("ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)");
            }
        }

        if (success)
        {
            pTree->addComponent(pAppearanceData);
        }

        //Close file
        file.close();
    }
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

    QAction *pOpenContainingFolder;
    if(pTree->mName != "External Libraries")
    {
        pOpenContainingFolder = menu.addAction("Open Containing Folder");
    }

    QAction *pUnloadLibraryFolder;
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
        mpContentsTree->findChild("External Libraries")->removeChild(pTree->mName);
        update();
    }

    QWidget::contextMenuEvent(event);
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
        QString componentName;
        componentName = mpLibraryWidget->mListItemToContentsMap.find(tempItem).value()->getName();

        //Change name in component name field. Resize the text if needed, so that the library widget does not change size.
        mpLibraryWidget->mpComponentNameField->setMaximumWidth(this->width());
        mpLibraryWidget->mpComponentNameField->setFont(QFont(mpLibraryWidget->mpComponentNameField->font().family(), min(10.0, .9*mpLibraryWidget->width()/(0.615*componentName.size()))));
        mpLibraryWidget->mpComponentNameField->setText(componentName);
    }
    QListWidget::mouseMoveEvent(event);
}


//! @brief Retrieves the appearance data for a given type name.
//! @param componentType Type name of the component
GUIModelObjectAppearance *LibraryWidget::getAppearanceData(QString componentType)
{
    //! @todo need error handling here, if we can not find component type we should not crash, should return 0 ptr or something and at some point (probably not here) giva a error message
    return mpContentsTree->findComponent(componentType)->getAppearanceData();
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
    if(!findChild(name))
    {
        LibraryContentsTree *pNewChild = new LibraryContentsTree(name);
        mChildNodesPtrs.append(pNewChild);
        return pNewChild;
    }
    return 0;
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
LibraryComponent *LibraryContentsTree::addComponent(GUIModelObjectAppearance *pAppearanceData)
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
LibraryComponent::LibraryComponent(GUIModelObjectAppearance *pAppearanceData)
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
GUIModelObjectAppearance *LibraryComponent::getAppearanceData()
{
    return mpAppearanceData;
}
