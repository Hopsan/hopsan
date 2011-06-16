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

//! @class LibraryContentItem
//! @brief The LibraryContentItem contains the typename and icon to show in the library when selecting component or other guiobjects
//!
//! The LibraryContentItem only contains the typename and icon to show in the library when selecting component or other guiobjects.
//! The actual appearance of the GUIObject after drag and drop is stored in a Map in the LibraryWidget
//!

//! @class LibraryWidget
//! @brief The LibraryWidget class is a class which store and display component libraries and other GUIObjects.
//!
//! This class is a widget that can be be included in the main window. It contains among other things a Map with appearance data for all loaded components and other GUIObjects.
//!

#include <QtGui>


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryWidget::LibraryWidget(MainWindow *parent)
        :   QWidget(parent)
{
    //mpParentMainWindow = parent;

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
    mpTreeViewButton->setToolTip(tr("Single List View"));
    mpDualViewButton = new QToolButton();
    mpDualViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryDualView.png"));
    mpDualViewButton->setToolTip(tr("Dual List View"));

    connect(mpTreeViewButton, SIGNAL(clicked()), this, SLOT(setListView()));
    connect(mpDualViewButton, SIGNAL(clicked()), this, SLOT(setDualView()));

    mpGrid = new QGridLayout(this);
    mpGrid->addWidget(mpTree,               0,0,1,3);
    mpGrid->addWidget(mpComponentNameField, 1,0,1,3);
    mpGrid->addWidget(mpList,               2,0,1,3);
    mpGrid->addWidget(mpTreeViewButton,     3,0,1,1);
    mpGrid->addWidget(mpDualViewButton,     3,1,1,1);
    mpGrid->setContentsMargins(4,4,4,4);

    setLayout(mpGrid);
    this->setMouseTracking(true);

    this->setListView();    //! @todo Should not be hard coded, load from config
    this->setGfxType(USERGRAPHICS);
}



//! @brief Reimplementation of QWidget::sizeHint(), used to reduce the size of the library widget when docked
QSize LibraryWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    //Set very small width. A minimum apperantly stops at resonable size.
    size.rwidth() = 210; //pixels
    return size;
}


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


class LibraryComponent;

void LibraryWidget::loadTreeView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem)
{
    mpList->hide();

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


void LibraryWidget::loadDualView(LibraryContentsTree *tree, QTreeWidgetItem *parentItem)
{
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


void LibraryWidget::initializeDrag(QListWidgetItem *item)
{
    if(!mListItemToContentsMap.contains(item)) return;

    QString typeName = mListItemToContentsMap.find(item).value()->getTypeName();
    QIcon icon = mListItemToContentsMap.find(item).value()->getIcon(mGfxType);

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(typeName);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}


void LibraryWidget::initializeDrag(QTreeWidgetItem *item, int dummy)
{
    if(!mTreeItemToContentsMap.contains(item)) return;

    QString typeName = mTreeItemToContentsMap.find(item).value()->getTypeName();
    QIcon icon = mTreeItemToContentsMap.find(item).value()->getIcon(mGfxType);

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(typeName);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}




//! @brief Adds a library to the library widget.
//! @param libDir is the library directory.
//! @param parentLib is the name of a possible parent library.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary()
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::loadLibrary(QString libDir, bool external)
{
    if (libDir.isEmpty() == true)       //! @todo Do we need this check?
        return;

    //Get the name for the library to be set in the tree

    if(external)
        return;     //! @todo Fix so that this works too!
//    if(external)
//    {
////        //*****Core Interaction*****

////            // Load all .dll or .so files in specified folder
////        HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
////        QDir libDirObject(libDir + "/");
////        QStringList filters;
////        #ifdef WIN32
////            filters << "*.dll";
////        #else
////            filters << "*.so";
////        #endif

////        libDirObject.setNameFilters(filters);
////        QStringList libList = libDirObject.entryList();
////        for (int i = 0; i < libList.size(); ++i)
////        {
////            QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
////            qDebug() << "Trying to load: " << filename << " in Core";
////            pHopsanCore->loadExternalComponent(filename.toStdString());
////        }
////        //**************************

////        //Check any core messages from external lib loading
////        gpMainWindow->mpMessageWidget->checkMessages();

////        addLibrary(libDir,QString("User defined libraries"));
//    }



    //Create a QDir object that contains the info about the library directory
    QDir libDirObject(libDir);
    //QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));

    libDirObject.setFilter(QDir::AllDirs);
    QStringList subDirList = libDirObject.entryList();
    subDirList.removeAll(".");
    subDirList.removeAll("..");
    subDirList.removeAll(".svn");
    for(int i=0; i<subDirList.size(); ++i)
    {
        loadLibraryFolder(libDir+"/"+subDirList.at(i), mpContentsTree);
    }

    update();
}


//! Let the user to point out a library and adds it to the library widget.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary(QString libDir, QString parentLib)
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::loadUserDefinedLibrary(QString libDir)
{
    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir

    if(libDir.isEmpty())    //Let user select a directory if no directory is specified
    {
        libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                   fileDialogOpenDir.currentPath(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    }
    if(!gConfig.hasUserLib(libDir))
    {
        gConfig.addUserLib(libDir);
        loadLibrary(libDir, true);
    }
    else
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Library " + libDir + " is already loaded!");
    }
}

void LibraryWidget::loadLibraryFolder(QString libDir, LibraryContentsTree *pParentTree)
{
    QDir libDirObject(libDir);
    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));

    if(pParentTree == 0)        // Create the actual tree (This one shall not be a node)
    {
        delete(mpContentsTree);         //Clear the tree and recreate it
        mpContentsTree = new LibraryContentsTree();

        //Append subnodes recursively
        libDirObject.setFilter(QDir::AllDirs);
        QStringList subDirList = libDirObject.entryList();
        subDirList.removeAll(".");
        subDirList.removeAll("..");
        subDirList.removeAll(".svn");
        for(int i=0; i<subDirList.size(); ++i)
        {
            loadLibraryFolder(libDir+"/"+subDirList.at(i), mpContentsTree);
        }
        return;
    }

    //Create the node
    LibraryContentsTree *pTree = pParentTree->addChild(libName);

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
    QStringList filters;
    filters << "*.xml";                     //Create the name filter
    libDirObject.setFilter(QDir::NoFilter);
    libDirObject.setNameFilters(filters);   //Set the name filter

    QStringList libList = libDirObject.entryList();    //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)        //Iterate over the file names
    {
        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
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
                pAppearanceData->setBasePath(libDirObject.absolutePath() + "/");
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


void LibraryWidget::setListView()
{
    mViewMode=0;
    update();
}


void LibraryWidget::setDualView()
{
    mViewMode=1;
    update();
}



LibraryListWidget::LibraryListWidget(LibraryWidget *parent)
    : QListWidget(parent)
{
    mpLibraryWidget = parent;
    setMouseTracking(true);
    setSelectionRectVisible(false);
}

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


class LibraryComponent;

//! @brief This function retrieves the appearance data given the TypeName
GUIModelObjectAppearance *LibraryWidget::getAppearanceData(QString componentType)
{
    return mpContentsTree->findComponent(componentType)->getAppearanceData();
}

//! @brief This function retrieves the appearance data given a display name
//! @todo This will mean trouble if two components have the same display name
GUIModelObjectAppearance *LibraryWidget::getAppearanceDataByDisplayName(QString displayName)
{
    return 0;//return getAppearanceData(mName2TypeMap.value(displayName));
}


void LibraryWidget::setGfxType(graphicsType gfxType)
{
    mGfxType = gfxType;
    update();
}


//void LibraryWidget::mouseMoveEvent(QMouseEvent *event)
//{
//    //! @todo maybe try to do this in some not so cpu needing way (setting white backround for all objects VERY often when mouse move)
//    QList<LibraryContentItem*> itemlist =  mLibraryContentItemPtrsMap.values();
//    QList<LibraryContentItem*>::iterator it = itemlist.begin();
//    for( ; it != itemlist.end(); ++it )
//    {
//        (*it)->setBackgroundColor(QColor("white"));
//        (*it)->setSelected(false);
//    }
//    mpComponentNameField->setText("");
//    gpMainWindow->mpHelpPopup->hide();
//    QWidget::mouseMoveEvent(event);
//}





//LibraryTreeWidget::LibraryTreeWidget(LibraryWidget *parent)
//        : QTreeWidget(parent)
//{
//    mpParentLibraryWidget = parent;
//}



//void LibraryTreeWidget::contextMenuEvent(QContextMenuEvent *event)
//{
//    QTreeWidget::contextMenuEvent(event);


//    QMenu menu;

//    QAction *loadAction;
//    QAction *unloadAction = new QAction(this);
//    loadAction = menu.addAction(QString("Load External Library"));


//        //! @todo This is an ugly check to make sure the right clicked object is a library with contents
//    if( (this->currentItem() != 0) && (this->currentItem()->parent() != 0) )
//    {
//        // This will check if the library is a user library (which can be removed)
//        if(mpParentLibraryWidget->mLibraryContentPtrsMap.find(QString(this->currentItem()->parent()->text(0) + this->currentItem()->text(0))).value()->mIsUserLib)
//        {
//            unloadAction = menu.addAction(QString("Unload Library \"" + this->currentItem()->text(0) + "\""));
//        }
//    }


//    QCursor *cursor;
//    QAction *selectedAction = menu.exec(cursor->pos());

//    if ((selectedAction == unloadAction) && (unloadAction != 0))
//    {
//        QMessageBox::StandardButton reply;
//        reply = QMessageBox::information(this, tr("Information"), tr("Program must be restarted for this to take effect."));
//        qDebug() << "Trying to remove " << this->currentItem()->text(0);
////        for(int i=0; i<mpParentLibraryWidget->mpParentMainWindow->mUserLibs.size(); ++i)
////        {
////            if(mpParentLibraryWidget->mpParentMainWindow->mUserLibs.at(i).endsWith("/"+this->currentItem()->text(0)))
////            {
////                qDebug() << "Removing at " << i;
////                mpParentLibraryWidget->mpParentMainWindow->mUserLibs.removeAt(i);
////                --i;
////            }
////        }
//        gConfig.removeUserLib(this->currentItem()->toolTip(0));
//    }
//    else if (selectedAction == loadAction)
//    {
//        this->mpParentLibraryWidget->addLibrary();
//    }
//}



LibraryContentsTree::LibraryContentsTree(QString name)
{
    mName = name;
}

bool LibraryContentsTree::isEmpty()
{
    return (mChildNodesPtrs.isEmpty() && mComponentPtrs.isEmpty());
}

bool LibraryContentsTree::hasChild(QString name)
{
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        if(mChildNodesPtrs.at(i)->mName == name)
            return true;
    }
    return false;
}

LibraryContentsTree *LibraryContentsTree::addChild(QString name)
{
    if(!hasChild(name))
    {
        LibraryContentsTree *pNewChild = new LibraryContentsTree(name);
        mChildNodesPtrs.append(pNewChild);
        return pNewChild;
    }
    return 0;
}


//! @todo This is not good, because several tree nodes should be able to have the same name
LibraryContentsTree *LibraryContentsTree::findChild(QString name)
{
    if(mName == name)
        return this;

    LibraryContentsTree *retval;
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        retval = mChildNodesPtrs.at(i)->findChild(name);
        if(retval != 0)
        {
            return retval;
        }
    }
    return 0;
}


LibraryComponent *LibraryContentsTree::addComponent(GUIModelObjectAppearance *pAppearanceData)
{
    LibraryComponent *pNewComponent = new LibraryComponent(pAppearanceData);
    mComponentPtrs.append(pNewComponent);
    return pNewComponent;
}


LibraryComponent *LibraryContentsTree::findComponent(QString typeName)
{
    LibraryContentsTree *retval;
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

LibraryComponent::LibraryComponent(GUIModelObjectAppearance *pAppearanceData)
{
    mpAppearanceData = pAppearanceData;
}


QIcon LibraryComponent::getIcon(graphicsType gfxType)
{
    QIcon icon;
    QString iconPath = mpAppearanceData->getFullAvailableIconPath(gfxType);
    QFile iconFile(iconPath);
    if (!iconFile.exists())     //Check if specified file exist, else use unknown icon
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
    icon.addFile(iconPath,QSize(55,55));
    return icon;
}


QString LibraryComponent::getName()
{
    return mpAppearanceData->getNonEmptyName();
}


QString LibraryComponent::getTypeName()
{
    return mpAppearanceData->getTypeName();
}


GUIModelObjectAppearance *LibraryComponent::getAppearanceData()
{
    return mpAppearanceData;
}
