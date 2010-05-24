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
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Based on: HopsanGUI
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
#include <map>
#include <iostream>

#include "LibraryWidget.h"
#include "listwidget.h"
#include "mainwindow.h"


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
        : QListWidgetItem(pAppearanceData->getTypeName(), pParent, QListWidgetItem::UserType)
{
    //Set font
    QFont font;
    font.setPixelSize(8);
    this->setFont(font);

    mpAppearanceData = pAppearanceData;
    selectIcon(false);
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

//! @brief Selects and loads either user or ISO icon
//! @param [in] useIso Select wheter to use user (false) or iso (true) icon
void LibraryContentItem::selectIcon(bool useIso)
{
    //Set Icon, prefere user, if its empty use iso
    QIcon icon;
 
//    //! @todo to speedup we might find out som of this stuff when we load appearanceData, so that we dont have to check every time, (dont know if necessary)
//    if ( !mpAppearanceData->getIconPath().isEmpty() && !useIso )
//    {
//        //Use user icon
//        icon.addFile(mpAppearanceData->getBasePath() + mpAppearanceData->getIconPath());
//    }
//    else if ( !mpAppearanceData->getIconPathISO().isEmpty() && useIso )
//    {
//        //use iso icon
//        icon.addFile(mpAppearanceData->getBasePath() + mpAppearanceData->getIconPathISO());
//    }
//    else if ( mpAppearanceData->getIconPath().isEmpty() && !mpAppearanceData->getIconPathISO().isEmpty() )
//    {
//        //Want user icon but not available, use iso icon
//        icon.addFile(mpAppearanceData->getBasePath() + mpAppearanceData->getIconPathISO());
//    }
//    else if ( !mpAppearanceData->getIconPath().isEmpty() && mpAppearanceData->getIconPathISO().isEmpty() )
//    {
//        //Want ISO icon but not available, Use user icon
//        icon.addFile(mpAppearanceData->getBasePath() + mpAppearanceData->getIconPath());
//    }
//    else
//    {
//        //No icon available use som noname icon
//        icon.addFile("som noname file"); //!< @todo Fix this, noname library icon
//    }
    icon.addFile(mpAppearanceData->getFullIconPath(useIso));

    setIcon(icon);
}


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryContent::LibraryContent(LibraryContent *pParentLibraryContent, LibraryWidget *pParentLibraryWidget)
    :   QListWidget(pParentLibraryContent)
{
    mpParentLibraryWidget = pParentLibraryWidget;
    setViewMode(QListView::IconMode);
    setAcceptDrops(false);
    setResizeMode(QListView::Adjust);
    //setIconSize(QSize(25,25));
}


void LibraryContent::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


void LibraryContent::mouseMoveEvent(QMouseEvent *event)
{
    if ( !(event->buttons() & Qt::LeftButton) )
        return;
    if ( (event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance() )
        return;

    //QByteArray *data = new QByteArray;
    QString datastr;
    QTextStream stream(&datastr);//, QIODevice::WriteOnly);

    QListWidgetItem *pItem = this->currentItem();

    //stream out appearance data and extra basepath info
    stream << *(mpParentLibraryWidget->getAppearanceData(pItem->text()));
    stream << "BASEPATH " << mpParentLibraryWidget->getAppearanceData(pItem->text())->getBasePath();

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    drag->setMimeData(mimeData);
    mimeData->setText(datastr);

    qDebug() << "Debug stream: " << mimeData->text();

    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()));
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

    setLayout(mpGrid);

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
    mLibraryContentMapPtrs.insert(parentLibraryName + libraryName, newLibContent);

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

        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file

        AppearanceData *pAppearanceData = new AppearanceData;
        bool sucess = pAppearanceData->setAppearanceData(inFile); //Read appearance from file
        pAppearanceData->setBasePath(libDirObject.absolutePath() + "/");

        if (sucess)
        {
            //Create library content item
            LibraryContentItem *libcomp= new LibraryContentItem(pAppearanceData);

            //Add the component to the library
            addLibraryContentItem(libName, parentLib, libcomp);
            qDebug() << "Loaded item: " << pAppearanceData->getTypeName();
        }
        else
        {
            qDebug() << "Error reading appearanceFile: " << filename;
            mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Failure when reading appearanceData file: " + filename);
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
    addLibrary(libDir,QString("User defined libraries"));
    //std::cout << qPrintable(libDir) << std::endl;
}


//! Adds a library content item to the library widget.
//! @param libraryName is the name of the library where the component should be added.
void LibraryWidget::addLibraryContentItem(QString libraryName, QString parentLibraryName, LibraryContentItem *newComponent)
{
    mLibraryContentMapPtrs.value(parentLibraryName + libraryName)->addItem(newComponent);
    QTreeWidgetItemIterator it(mpTree);
    while (*it)
    {
        if (((*it)->text(0) == libraryName) && ((*it)->parent()))
        {
            if((*it)->parent()->text(0) == parentLibraryName)      //Only add component if in the correct set of libraries
            {
                LibraryContentItem *copyOfNewComponent = new LibraryContentItem(*newComponent); //A QListWidgetItem can only be in one list at the time, therefor a copy...
                addLibraryContentItem(parentLibraryName, "", copyOfNewComponent); //Recursively
            }
        }
        ++it;
    }
    mAppearanceDataMap.insert(newComponent->getAppearanceData()->getTypeName(), newComponent->getAppearanceData());
}


//! Makes a library visible.
//! @param item is the library to show.
//! @param column is the position of the library name in the tree.
//! @see hideAllLib()
void LibraryWidget::showLib(QTreeWidgetItem *item, int column)
{
   hideAllLib();

   QMap<QString, LibraryContent*>::iterator lib;
   for (lib = mLibraryContentMapPtrs.begin(); lib != mLibraryContentMapPtrs.end(); ++lib)
   {
        //Not top level list widget, so check if it has the correct parent
        if(item->text(column).size() != mLibraryContentMapPtrs.key((*lib)).size())
        {
            if (item->text(column) == mLibraryContentMapPtrs.key((*lib)).right(item->text(column).size()) &&
                item->parent()->text(column) == mLibraryContentMapPtrs.key((*lib)).left(item->parent()->text(column).size()))
            {
                (*lib)->show();
            }
        }
        else
        //Top level widget, don't check parent (would lead to a segmentation fault since it does not exist)
        {
            if (item->text(column) == mLibraryContentMapPtrs.key((*lib)).right(item->text(column).size()))
            {
                (*lib)->show();
            }
        }
    }
}


AppearanceData *LibraryWidget::getAppearanceData(QString componentType)
{
    qDebug() << "LibraryWidget::getAppearanceData: " + componentType;
    if (mAppearanceDataMap.count(componentType) == 0)
    {
        qDebug() << "Trying to fetch appearanceData for " + componentType + " which does not appear to exist in the Map, returning empty data";
        mpParentMainWindow->mpMessageWidget->printGUIWarningMessage("Trying to fetch appearanceData for " + componentType + " which does not appear to exist in the Map, returning empty data");
    }

    return mAppearanceDataMap.value(componentType);
}

//! Hide all libraries.
//! @see showLib(QTreeWidgetItem *item, int column)
void LibraryWidget::hideAllLib()
{
    QMap<QString, LibraryContent*>::iterator lib;
    for (lib = mLibraryContentMapPtrs.begin(); lib != mLibraryContentMapPtrs.end(); ++lib)
    {
        (*lib)->hide();
    }
}

void LibraryWidget::useIsoGraphics(bool useISO)
{
    QMap<QString, LibraryContent*>::iterator lib;
    for (lib = mLibraryContentMapPtrs.begin(); lib != mLibraryContentMapPtrs.end(); ++lib)
    {
        for (int i=0; i<(*lib)->count(); i++)
        {
            //! @todo q casting will not work in this cas need to rewrite and use some otehr way
            //LibraryContentItem* libcontit =
            //qobject_cast<LibraryContentItem*>( (*lib)->item(i) )->selectIcon(useISO);
            //libcontit->selectIcon(useISO);
        }
    }
}
