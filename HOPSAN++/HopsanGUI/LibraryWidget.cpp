/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

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
#include "MainWindow.h"
#include "MessageWidget.h"
#include "AppearanceData.h"

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

//! Constructor
LibraryContentItem::LibraryContentItem(AppearanceData *pAppearanceData, QListWidget *pParent)
        : QListWidgetItem(pParent, QListWidgetItem::UserType)
{
    //Set font
    //QFont font;
    //font.setPointSizeF(0.001);
    //this->setFont(font);

    this->setToolTip(pAppearanceData->getNonEmptyName());
    this->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    mpAppearanceData = pAppearanceData;
    selectIcon(USERGRAPHICS);
}

//! @brief Copy Constructor
LibraryContentItem::LibraryContentItem(const QListWidgetItem &other)
        : QListWidgetItem(other)
{
}

//! @brief Get a pointer to appearanceData
AppearanceData *LibraryContentItem::getAppearanceData()
{
    return mpAppearanceData;
}

//! @brief Wraps the apperancedata get name function
QString LibraryContentItem::getTypeName()
{
    return mpAppearanceData->getTypeName();
}

//! @brief Selects and loads either user or ISO icon
//! @param [in] gfxType Select wheter to use user (false) or iso (true) icon
void LibraryContentItem::selectIcon(graphicsType gfxType)
{
    //Set Icon, prefere user, if its empty use iso
    QIcon icon;
    QPixmap testPixMap;

    icon.addFile(mpAppearanceData->getFullIconPath(gfxType),QSize(55,55));
    //this->setSizeHint(QSize(55,55));
    this->setIcon(icon);
    //this->setData(Qt::UserRole, QVariant(icon));
}


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryContent::LibraryContent(LibraryContent *pParentLibraryContent, LibraryWidget *pParentLibraryWidget)
    :   QListWidget(pParentLibraryContent)
{
    mpParentLibraryWidget = pParentLibraryWidget;
    this->setViewMode(QListView::IconMode);
    this->setResizeMode(QListView::Adjust);
    this->setMouseTracking(true);
    this->setSelectionRectVisible(false);
    this->setDragEnabled(true);
    this->setIconSize(QSize(40,40));
    this->setGridSize(QSize(50,50));
    this->setAcceptDrops(true);
    this->setDropIndicatorShown(true);
    //this->setSpacing(10);

    connect(this,SIGNAL(itemEntered(QListWidgetItem*)),this,SLOT(highLightItem(QListWidgetItem*)));

}

void LibraryContent::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


void LibraryContent::highLightItem(QListWidgetItem *item)
{
    qDebug() << "itemEntered";
    item->setBackgroundColor(QColor("lightgray"));
}


void LibraryContent::mouseMoveEvent(QMouseEvent *event)
{
    //! @todo maybe try to do this in some not so cpu needing way (setting white backround for all objects VERY often when mouse move)
        //Make hovered item gray & display name//
    QList<LibraryContentItem*> itemlist =  mpParentLibraryWidget->mLibraryContentItemPtrsMap.values();
    QList<LibraryContentItem*>::iterator it = itemlist.begin();
    for( ; it != itemlist.end(); ++it )
    {
        (*it)->setBackgroundColor(QColor("white"));
        (*it)->setSelected(false);
    }

    mpParentLibraryWidget->mpComponentNameField->setText("");
    QListWidgetItem *tempItem = itemAt(event->pos());
    if(tempItem != 0x0)     //! @todo This is perhaps a bit ugly, but the pointer is zero if there are not item beneath the mouse
    {
        tempItem->setBackgroundColor(QColor("lightblue"));
        mpParentLibraryWidget->mpComponentNameField->setText(tempItem->toolTip());
    }
        //***********************//

    if ( !(event->buttons() & Qt::LeftButton) )
        return;
    if ( (event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance() )
        return;

        //Drag is initialized, so remove the highlight and name text stuff
    if(tempItem != 0x0)
    {
        tempItem->setBackgroundColor(QColor("white"));
        tempItem->setSelected(false);
    }
    mpParentLibraryWidget->mpComponentNameField->setText("");

    QListWidgetItem *pItem = this->currentItem();

    if(!pItem)
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(mpParentLibraryWidget->getAppearanceDataByDisplayName(pItem->toolTip())->getTypeName());

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pItem->icon().pixmap(40,40));

    //qDebug() << "Debug stream: " << mimeData->text();

    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()/2));
    drag->exec(Qt::CopyAction | Qt::MoveAction);
    //Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);
}


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryWidget::LibraryWidget(MainWindow *parent)
        :   QWidget(parent)
{
    mpParentMainWindow = parent;

    mpTree = new QTreeWidget(this);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);

    mpGrid = new QVBoxLayout(this);

    mpGrid->addWidget(mpTree);

    mpComponentNameField = new QLabel("No Component Selected", this);
    mpGrid->addWidget(mpComponentNameField);
    mpComponentNameField->setAlignment(Qt::AlignCenter);
    mpComponentNameField->setFont(QFont(mpComponentNameField->font().family(), 12));
    mpComponentNameField->setText("");
    //mpComponentNameField->hide();

    setLayout(mpGrid);
    this->setMouseTracking(true);

    connect(mpTree, SIGNAL(itemClicked (QTreeWidgetItem*, int)), SLOT(showLib(QTreeWidgetItem*, int)));
}


//! Adds an empty library to the library widget.
//! @param libraryName is the name of the new library.
//! @param parentLibraryName is the name of an eventually parent library.
//! @see addLibrary(QString libDir, QString parentLib)
//! @see addLibrary()
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::addEmptyLibrary(QString libraryName, QString parentLibraryName)
{
    QTreeWidgetItem *newTreePost = new QTreeWidgetItem((QTreeWidget*)0);
    newTreePost->setText(0, QString(libraryName));

    LibraryContent *newLibContent = new LibraryContent((LibraryContent*)0, this);
    newLibContent->setDragEnabled(true);
    //newLibContent->setDropIndicatorShown(true);
    mLibraryContentPtrsMap.insert(parentLibraryName + libraryName, newLibContent);

    mpGrid->addWidget(newLibContent);
    newLibContent->hide();

    if (parentLibraryName.isEmpty())
    {
        mpTree->insertTopLevelItem(0, newTreePost);
    }
    else
    {
        QTreeWidgetItemIterator it(mpTree);
        while (*it)
        {
            if ((*it)->text(0) == parentLibraryName)
            {
                (*it)->addChild(newTreePost);
                mpTree->expandItem(*it);
            }

            ++it;
        }
    }
}


//! Adds a library to the library widget.
//! @param libDir is the library directory.
//! @param parentLib is the name of an eventually parent library.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary()
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::addLibrary(QString libDir, QString parentLib)
{
    //If no directory is set, i.e. cancel is presses, do no more
    if (libDir.isEmpty() == true)
        return;

    QDir libDirObject(libDir);  //Create a QDir object that contains the info about the library direction

    //Get the name for the library to be set in the tree
    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));

    //Add the library to the tree
    addEmptyLibrary(libName,parentLib);

    QStringList filters;        //Create a QStringList object that contains name filters
    filters << "*.txt";         //Create the name filter
    libDirObject.setNameFilters(filters);       //Set the name filter

    QStringList libList = libDirObject.entryList(); //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)    //Iterate over the file names
    {
        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            qDebug() << "Failed to open file or not a text file: " + filename;
            return;
        }

        bool sucess = true;
        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file
        AppearanceData *pAppearanceData = new AppearanceData;
        pAppearanceData->readFromTextStream(inFile); //Read appearance from file
        pAppearanceData->setBasePath(libDirObject.absolutePath() + "/");
        if (!pAppearanceData->mIsReadOK)
        {
            mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Error when reading appearance data from file: " + filename);
            sucess = false;
        }
        else
        {
            //*****Core Interaction*****
            HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
            if(!((pAppearanceData->getTypeName()=="Subsystem") || (pAppearanceData->getTypeName()=="SystemPort"))) //Do not check if it is Subsystem or SystemPort
            {
                sucess = pHopsanCore->hasComponent(pAppearanceData->getTypeName().toStdString()); //Check so that there is such component availible in the Core
                if (!sucess)
                {
                    mpParentMainWindow->mpMessageWidget->printGUIWarningMessage("Warning: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)");
                }
            }
            //**************************
        }

        if (sucess)
        {
            //Create library content item
            LibraryContentItem *libcomp= new LibraryContentItem(pAppearanceData);
            //Add the component to the library
            addLibraryContentItem(libName, parentLib, libcomp);
        }

        //Close file
        file.close();
    }
}


//! Let the user to point out a library and adds it to the library widget.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary(QString libDir, QString parentLib)
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::addLibrary()
{
    /*QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    fileName = QFileDialog::getExistingDirectory();*/

    /*Alt. way
    fileName = QFileDialog::getOpenFileName(this,
     tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));*/

    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir

    QString libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                 fileDialogOpenDir.currentPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(!mpParentMainWindow->mUserLibs.contains(libDir))
    {
        mpParentMainWindow->mUserLibs.append(libDir);
        addExternalLibrary(libDir);
    }
    else
    {
        mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Library " + libDir + " is already loaded!");
    }
    //std::cout << qPrintable(libDir) << std::endl;
}


//! Load a external library and adds it to the 'User defined libraries'.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary(QString libDir, QString parentLib)
void LibraryWidget::addExternalLibrary(QString libDir)
{
    //QString libDir = "../../HopsanGUI/componentData/UserLibs/TestLib";

    //*****Core Interaction*****

        // Load all .dll or .so files in specified folder
    HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
    QDir libDirObject(libDir + "/");
    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));
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
        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        pHopsanCore->loadExternalComponent(filename.toStdString());
    }
    //**************************

    addLibrary(libDir,QString("User defined libraries"));
}


//! Adds a library content item to the library widget.
//! @param libraryName is the name of the library where the component should be added.
void LibraryWidget::addLibraryContentItem(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent)
{
    //qDebug() << "Adding componentType: " << newComponent->getTypeName();

    //First add the item to the overview LibraryContent (This will cast to QListWidget Item and not preserver our stuff)
    mLibraryContentPtrsMap.value(parentLibraryName + libraryName)->addItem(newComponent);
    //Now add it to our own MultiMap to retain a pointer the the LibraryContentItem with our own stuff
    mLibraryContentItemPtrsMap.insertMulti(newComponent->getTypeName(), newComponent);

    //Now add to sub library content
    QTreeWidgetItemIterator it(mpTree);
    while (*it)
    {
        if (((*it)->text(0) == libraryName) && ((*it)->parent()))
        {
            if((*it)->parent()->text(0) == parentLibraryName)      //Only add component if in the correct set of libraries
            {
                LibraryContentItem *copyOfNewComponent = new LibraryContentItem(*newComponent); //A QListWidgetItem can only be in one list at the time, therefor a copy...
                mLibraryContentItemPtrsMap.insertMulti(newComponent->getTypeName(), copyOfNewComponent);
                addLibraryContentItem(parentLibraryName, "", copyOfNewComponent); //Recursively
            }
        }
        ++it;
    }
    mName2TypeMap.insert(newComponent->getAppearanceData()->getNonEmptyName(), newComponent->getAppearanceData()->getTypeName()); //! @todo this is a temporary workaround
}


//! Makes a library visible.
//! @param item is the library to show.
//! @param column is the position of the library name in the tree.
//! @see hideAllLib()
void LibraryWidget::showLib(QTreeWidgetItem *item, int column)
{
   hideAllLib();

   QHash<QString, LibraryContent*>::iterator lib;
   for (lib = mLibraryContentPtrsMap.begin(); lib != mLibraryContentPtrsMap.end(); ++lib)
   {
        //Not top level list widget, so check if it has the correct parent
        if(item->text(column).size() != mLibraryContentPtrsMap.key((*lib)).size())
        {
            if (item->text(column) == mLibraryContentPtrsMap.key((*lib)).right(item->text(column).size()) &&
                item->parent()->text(column) == mLibraryContentPtrsMap.key((*lib)).left(item->parent()->text(column).size()))
            {
                (*lib)->show();
            }
        }
        else
        //Top level widget, don't check parent (would lead to a segmentation fault since it does not exist)
        {
            if (item->text(column) == mLibraryContentPtrsMap.key((*lib)).right(item->text(column).size()))
            {
                (*lib)->show();
            }
        }
    }
}

//! @brief This function retrieves the appearance data given the TypeName
AppearanceData *LibraryWidget::getAppearanceData(QString componentType)
{
    //qDebug() << "LibraryWidget::getAppearanceData: " + componentType;
    if (mLibraryContentItemPtrsMap.count(componentType) == 0)
    {
        qDebug() << "Trying to fetch appearanceData for " + componentType + " which does not appear to exist in the Map, returning empty data";
        mpParentMainWindow->mpMessageWidget->printGUIWarningMessage("Trying to fetch appearanceData for " + componentType + " which does not appear to exist in the Map, returning empty data");
        return 0;
    }
    return mLibraryContentItemPtrsMap.value(componentType)->getAppearanceData();
}

//! @brief This function retrieves the appearance data given a display name
//! @todo This is a temporary hack
AppearanceData *LibraryWidget::getAppearanceDataByDisplayName(QString displayName)
{
    return getAppearanceData(mName2TypeMap.value(displayName));
}

//! Hide all libraries.
//! @see showLib(QTreeWidgetItem *item, int column)
void LibraryWidget::hideAllLib()
{
    QHash<QString, LibraryContent*>::iterator lib;
    for (lib = mLibraryContentPtrsMap.begin(); lib != mLibraryContentPtrsMap.end(); ++lib)
    {
        (*lib)->hide();
    }
}

void LibraryWidget::setGfxType(graphicsType gfxType)
{
    //qDebug() << "setGfxType gfxType";
    QList<LibraryContentItem*> itemlist =  mLibraryContentItemPtrsMap.values();
    QList<LibraryContentItem*>::iterator it = itemlist.begin();
    for( ; it != itemlist.end(); ++it )
    {
        (*it)->selectIcon(gfxType);
    }
}


void LibraryWidget::mouseMoveEvent(QMouseEvent *event)
{
    //! @todo maybe try to do this in some not so cpu needing way (setting white backround for all objects VERY often when mouse move)
    QList<LibraryContentItem*> itemlist =  mLibraryContentItemPtrsMap.values();
    QList<LibraryContentItem*>::iterator it = itemlist.begin();
    for( ; it != itemlist.end(); ++it )
    {
        (*it)->setBackgroundColor(QColor("white"));
        (*it)->setSelected(false);
    }
    mpComponentNameField->setText("");
    QWidget::mouseMoveEvent(event);
}
