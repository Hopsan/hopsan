/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   LibraryWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

//Qt includes
#include <QApplication>
#include <QDrag>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGridLayout>

//Hopsan includes
#include "global.h"
#include "LibraryWidget.h"
#include "LibraryHandler.h"
#include "CoreAccess.h"
#include "Utilities/HelpPopUpWidget.h"
#include "ModelicaLibrary.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "ModelicaEditor.h"
#include "ProjectTabWidget.h"

//! @todo Ok don't know where I should put this, putting it here for now /Peter
QString gHopsanCoreVersion = getHopsanCoreVersion();

LibraryWidget::LibraryWidget(QWidget *parent)
        :   QWidget(parent)
{
    this->setMouseTracking(true);

    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    bool treeView = true;

    mpTree = new QTreeWidget(this);
    mpTree->setMouseTracking(true);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);
    mpTree->setVisible(treeView);

    mpDualTree = new QTreeWidget(this);
    mpDualTree->setMouseTracking(true);
    mpDualTree->setHeaderHidden(true);
    mpDualTree->setColumnCount(1);
    mpDualTree->setHidden(treeView);

    mpList = new QListWidget(this);
    mpList->setMouseTracking(true);
    mpList->setViewMode(QListView::IconMode);
    mpList->setResizeMode(QListView::Adjust);
    mpList->setIconSize(QSize(48,48));
    mpList->setGridSize(QSize(53,53));
    mpList->setHidden(treeView);

    mpComponentNameLabel = new QLabel(this);
    mpComponentNameLabel->setHidden(treeView);

    QLabel *pFilterLabel = new QLabel("Filter:",this);
    mpFilterEdit = new QLineEdit(this);
    QHBoxLayout *pFilterLayout = new QHBoxLayout();
    pFilterLayout->addWidget(pFilterLabel);
    pFilterLayout->addWidget(mpFilterEdit);

    QToolButton *pClearFilterButton = new QToolButton(this);
    pClearFilterButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    pFilterLayout->addWidget(pClearFilterButton);
    connect(pClearFilterButton, SIGNAL(clicked()), mpFilterEdit, SLOT(clear()));
    connect(pClearFilterButton, SIGNAL(clicked()), this, SLOT(update()));

    QSize iconSize = QSize(24,24);  //Size of library icons

    QToolButton *pTreeViewButton = new QToolButton();
    pTreeViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryTreeView.png"));
    pTreeViewButton->setIconSize(iconSize);
    pTreeViewButton->setToolTip(tr("Single List View"));

    QToolButton *pDualViewButton = new QToolButton();
    pDualViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryDualView.png"));
    pDualViewButton->setIconSize(iconSize);
    pDualViewButton->setToolTip(tr("Dual List View"));

    QToolButton *pHelpButton = new QToolButton();
    pHelpButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Help.png"));
    pHelpButton->setToolTip(tr("Open Context Help"));
    pHelpButton->setIconSize(iconSize);

    connect(gpLibraryHandler, SIGNAL(contentsChanged()), this, SLOT(update()));
    connect(mpTree,     SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpDualTree, SIGNAL(itemEntered(QTreeWidgetItem*,int)),  mpComponentNameLabel,   SLOT(clear()));
    connect(mpDualTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpList,     SIGNAL(itemPressed(QListWidgetItem*)),      this,                   SLOT(handleItemClick(QListWidgetItem*)));
    connect(mpList,     SIGNAL(itemEntered(QListWidgetItem*)),      this,                   SLOT(handleItemEntered(QListWidgetItem*)));
    connect(pTreeViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(show()));
    connect(pTreeViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(hide()));
    connect(pTreeViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(hide()));
    connect(pTreeViewButton, SIGNAL(clicked()),    mpList,                 SLOT(hide()));
    connect(pDualViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(hide()));
    connect(pDualViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(show()));
    connect(pDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(show()));
    connect(pDualViewButton, SIGNAL(clicked()),    mpList,                 SLOT(show()));
    connect(pDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(clear()));
    connect(pHelpButton,     SIGNAL(clicked()),    gpHelpPopupWidget,           SLOT(openContextHelp()));
    connect(mpFilterEdit,   SIGNAL(textEdited(QString)), this, SLOT(update()));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTree,                  0,0,3,4);
    pLayout->addWidget(mpDualTree,              0,0,1,4);
    pLayout->addWidget(mpComponentNameLabel,    1,0,1,4);
    pLayout->addWidget(mpList,                  2,0,1,4);
    pLayout->addLayout(pFilterLayout,           3,0,1,4);
    pLayout->addWidget(pTreeViewButton,        4,0);
    pLayout->addWidget(pDualViewButton,        4,1);
    pLayout->addWidget(new QWidget(this),       4,2);
    pLayout->addWidget(pHelpButton,            4,3);
    pLayout->setColumnStretch(2,1);
    this->setLayout(pLayout);

    this->setGfxType(UserGraphics);
}


//! @brief Reimplementation of QWidget::sizeHint()
//! Used to reduce the size of the library widget when docked.
QSize LibraryWidget::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    size.rwidth() = 250;            //Set very small width. A minimum apparently stops at reasonable size.
    return size;
}


//! @brief Selects graphics type to be used in library (iso or user).
void LibraryWidget::setGfxType(GraphicsTypeEnumT gfxType)
{
    mGfxType = gfxType;
    update();       //Redraw the library
}


void LibraryWidget::update()
{
    //Remember opened folders
    QTreeWidgetItemIterator it(mpTree);

    QList<QStringList> expandedItems;
    while(*it)
    {
        QTreeWidgetItem *pItem = (*it);
        if(pItem->isExpanded())
        {
            QStringList list;
            list << pItem->text(0);
            QTreeWidgetItem *pParent = pItem->parent();
            while(pParent)
            {
                list.prepend(pParent->text(0));
                pParent = pParent->parent();
            }
            expandedItems.append(list);
            qDebug() << "List: " << list;

        }
        ++it;
    }


    QString filter = mpFilterEdit->text();

    mpList->clear();
    mListItemToTypeNameMap.clear();

    mpTree->clear();
    mpDualTree->clear();
    mpList->clear();
    mItemToTypeNameMap.clear();
    mFolderToContentsMap.clear();

    QFont boldFont = qApp->font();
    boldFont.setBold(true);

    Q_FOREACH(const QString typeName, gpLibraryHandler->getLoadedTypeNames())
    {
        LibraryEntry entry = gpLibraryHandler->getEntry(typeName);
        if(entry.visibility == Hidden || !entry.pAppearance->getDisplayName().toLower().contains(filter.toLower()))
        {
            continue;
        }

        QStringList path;
        if(filter.isEmpty())
        {
            path = entry.path;
        }

        QTreeWidgetItem *pItem = 0;
        QTreeWidgetItem *pDualItem = 0;
        while(!path.isEmpty())
        {
            QString folder = path.first();
            path.removeFirst();
            if(pItem == 0)
            {
                for(int i=0; i<mpTree->topLevelItemCount(); ++i)
                {
                    if(mpTree->topLevelItem(i)->text(0) == folder)
                    {
                        pItem = mpTree->topLevelItem(i);
                        pDualItem = mpDualTree->topLevelItem(i);
                        break;
                    }
                }
                if(pItem == 0)
                {
                    //Add top-level folder to tree view
                    pItem = new QTreeWidgetItem();
                    pItem->setFont(0,boldFont);
                    if(folder == EXTLIBSTR)
                    {
                        pItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-FolderExternal.png"));
                    }
                    else
                    {
                        pItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                    }
                    pItem->setText(0, folder);
                    pItem->setToolTip(0, folder);
                    mpTree->addTopLevelItem(pItem);

                    //Duplicate folder to dual view
                    pDualItem = pItem->clone();
                    mpDualTree->addTopLevelItem(pDualItem);
                }
            }
            else
            {
                bool exists=false;
                for(int i=0; i<pItem->childCount(); ++i)
                {
                    if(pItem->child(i)->text(0) == folder)
                    {
                        pItem = pItem->child(i);
                        pDualItem = pDualItem->child(i);
                        exists=true;
                        break;
                    }
                }
                if(!exists)
                {
                    //Add folder to tree view
                    QTreeWidgetItem *pNewItem = new QTreeWidgetItem();
                    pNewItem->setFont(0, boldFont);
                    QTreeWidgetItem *pTopItem = pItem;
                    while(pTopItem->parent())
                        pTopItem = pTopItem->parent();
                    if( pTopItem->text(0) == EXTLIBSTR)
                        pNewItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-FolderExternal.png"));
                    else
                        pNewItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
                    pNewItem->setText(0, folder);
                    pNewItem->setToolTip(0, folder);
                    pItem->addChild(pNewItem);
                    pItem = pNewItem;

                    //Duplicate folder to dual view
                    QTreeWidgetItem *pDualNewItem = pNewItem->clone();
                    pDualItem->addChild(pDualNewItem);
                    pDualItem = pDualNewItem;
                }
            }
        }

        //Add component to tree view
        QTreeWidgetItem *pComponentItem = new QTreeWidgetItem();
        pComponentItem->setIcon(0, entry.pAppearance->getIcon(mGfxType));
        pComponentItem->setText(0, entry.pAppearance->getDisplayName());
        pComponentItem->setToolTip(0, entry.pAppearance->getDisplayName());
        if(pItem)
        {
            pItem->addChild(pComponentItem);
        }
        else
        {
            mpTree->addTopLevelItem(pComponentItem);
        }
        mItemToTypeNameMap.insert(pComponentItem, typeName);

        //Register component to dual view
        if(!mFolderToContentsMap.contains(pDualItem))
        {
            mFolderToContentsMap.insert(pDualItem, QStringList() << typeName);
        }
        else
        {
            QStringList list = mFolderToContentsMap.find(pDualItem).value();
            list << typeName;
            mFolderToContentsMap.insert(pDualItem, list);
        }

        while(pDualItem && pDualItem->parent())
        {
            pDualItem = pDualItem->parent();
            if(!mFolderToContentsMap.contains(pDualItem))
            {
                mFolderToContentsMap.insert(pDualItem, QStringList() << typeName);
            }
            else
            {
                QStringList list = mFolderToContentsMap.find(pDualItem).value();
                list << typeName;
                mFolderToContentsMap.insert(pDualItem, list);
            }
        }

        if(!filter.isEmpty())
        {
            ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
            QListWidgetItem *tempItem = new QListWidgetItem();
            tempItem->setIcon(/*QIcon(pAppearance->getFullAvailableIconPath())*/pAppearance->getIcon(mGfxType));
            tempItem->setToolTip(pAppearance->getDisplayName());
            mListItemToTypeNameMap.insert(tempItem, typeName);
            tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpList->addItem(tempItem);
        }
    }

    //Sort trees, and make sure external libraries are shown at the bottom
    QTreeWidgetItemIterator itt(mpTree);
    while(*itt)
    {

        if((*itt)->childCount() > 0 && (*itt)->text(0) != EXTLIBSTR)
        {
            (*itt)->setText(0, "0000000000"+(*itt)->text(0));       //Prepends a lot of zeros to subfolders, to make sure they are sorted on top (REALLY ugly, but it works)
        }
        ++itt;
    }
    QTreeWidgetItem *pExternalItem = 0;
    for(int t=0; t<mpTree->topLevelItemCount(); ++t)
    {
        if(mpTree->topLevelItem(t)->text(0) == EXTLIBSTR)
        {
            pExternalItem = mpTree->takeTopLevelItem(t);
            break;
        }
    }
    mpTree->sortItems(0, Qt::AscendingOrder);
    if(pExternalItem)
    {
        mpTree->insertTopLevelItem(mpTree->topLevelItemCount(),pExternalItem);
    }
    pExternalItem = 0;
    for(int t=0; t<mpDualTree->topLevelItemCount(); ++t)
    {
        if(mpDualTree->topLevelItem(t)->text(0) == EXTLIBSTR)
        {
            pExternalItem = mpDualTree->takeTopLevelItem(t);
            break;
        }
    }
    mpDualTree->sortItems(0, Qt::AscendingOrder);
    if(pExternalItem)
    {
        mpDualTree->insertTopLevelItem(mpDualTree->topLevelItemCount(),pExternalItem);
    }
    QTreeWidgetItemIterator itt2(mpTree);
    while(*itt2)
    {
        if((*itt2)->childCount() > 0 && (*itt2)->text(0) != EXTLIBSTR)
        {
            (*itt2)->setText(0, (*itt2)->text(0).remove(0,10)); //Remove the extra zeros from subfolders (see above)
        }
        ++itt2;
    }

    QTreeWidgetItem *pModelicaComponentsItem = new QTreeWidgetItem();
    pModelicaComponentsItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-FolderModelica.png"));
    pModelicaComponentsItem->setText(0, MODELICALIBSTR);
    pModelicaComponentsItem->setFont(0,boldFont);
    mpTree->addTopLevelItem(pModelicaComponentsItem);
    foreach(const QString &model, gpModelicaLibrary->getModelNames())
    {
        QTreeWidgetItem *pModelicaComponentItem = new QTreeWidgetItem();
        pModelicaComponentItem->setText(0,model);

        QString annotations = gpModelicaLibrary->getModel(model).getAnnotations();
        if(!annotations.isEmpty())
        {
            QString cafFilePath = annotations.section("cafFile",1,1).section("\"",1,1).section("\"",0,0);
            QString icon = annotations.section("hopsanIcon",1,1).section("\"",1,1).section("\"",0,0);
            if(!icon.isEmpty())
            {
                pModelicaComponentItem->setIcon(0, QIcon(icon));
            }
            else if(!cafFilePath.isEmpty())
            {
                ModelObjectAppearance appearane;
                QFile cafFile(cafFilePath);
                if (cafFile.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QDomDocument domDocument;
                    QDomElement cafRoot = loadXMLDomDocument(cafFile, domDocument, CAF_ROOT);
                    cafFile.close();
                    if(!cafRoot.isNull())
                    {
                        //Read appearance data from the caf xml file
                        QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearance objects from same file, also not hardcode tagnames
                        appearane.setBasePath(QFileInfo(cafFile).absolutePath()+"/");
                        appearane.readFromDomElement(xmlModelObjectAppearance);
                        appearane.cacheIcons();
                        pModelicaComponentItem->setIcon(0, QIcon(appearane.getIconPath(UserGraphics,Absolute)));
                    }
                }
            }
        }

        //! @todo Fix icon!
        mItemToTypeNameMap.insert(pModelicaComponentItem, QString(MODELICATYPENAME)+"_"+model);
        pModelicaComponentsItem->addChild(pModelicaComponentItem);
    }

    if(filter.isEmpty())
    {
        //Append load external library items
        mpLoadLibraryItem = new QTreeWidgetItem();
        mpLoadLibraryItem->setText(0, "Load external library");
        mpLoadLibraryItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpLoadLibraryItem->setToolTip(0, "Load external library");
        mpTree->addTopLevelItem(mpLoadLibraryItem);

        mpAddModelicaFileItem = new QTreeWidgetItem();
        mpAddModelicaFileItem->setText(0, "Load Modelica file");
        mpAddModelicaFileItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddModelicaFileItem->setToolTip(0, "Load Modelica file");
        mpTree->addTopLevelItem(mpAddModelicaFileItem);

        mpLoadLibraryItemDual = new QTreeWidgetItem();
        mpLoadLibraryItemDual->setText(0, "Load external library");
        mpLoadLibraryItemDual->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpLoadLibraryItemDual->setToolTip(0, "Load external library");
        mpDualTree->addTopLevelItem(mpLoadLibraryItemDual);

        mpAddModelicaFileItemDual = new QTreeWidgetItem();
        mpAddModelicaFileItemDual->setText(0, "Load Modelica file");
        mpAddModelicaFileItemDual->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddModelicaFileItemDual->setToolTip(0, "Load Modelica file");
        mpDualTree->addTopLevelItem(mpAddModelicaFileItemDual);
    }

    //Append Modelica files
    QStringList paths;
    gpModelicaLibrary->getModelicaFiles(paths);

    foreach(const QString &path, paths)
    {
        QTreeWidgetItem *pModelicaItem = new QTreeWidgetItem();
        pModelicaItem->setText(0, QFileInfo(path).fileName());
        pModelicaItem->setToolTip(0, path);
        pModelicaItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-New.png"));
        mItemToModelicaFileNameMap.insert(pModelicaItem, path);
        mpTree->addTopLevelItem(pModelicaItem);
    }

    if(!filter.isEmpty())
    {
        mpDualTree->setFixedSize(0,0);
        //mpDualTree->hide();
    }
    else
    {
        //mpDualTree->setFixedSize(100,100);
        mpDualTree->setMaximumSize(5000,5000);
        mpDualTree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }


    //Expand previously expanded folders
    foreach(const QStringList &list, expandedItems)
    {
        for(int i=0; i<mpTree->topLevelItemCount(); ++i)
        {
            if(mpTree->topLevelItem(i)->text(0) == list[0])
            {
                QTreeWidgetItem *pItem = mpTree->topLevelItem(i);
                pItem->setExpanded(true);
                for(int j=1; j<list.size(); ++j)
                {
                    for(int k=0; k<pItem->childCount(); ++k)
                    {
                        if(pItem->child(k)->text(0) == list[j])
                        {
                            pItem = pItem->child(k);
                            pItem->setExpanded(true);
                            break;
                        }
                    }
                }
            }
        }
    }
}


void LibraryWidget::handleItemClick(QTreeWidgetItem *item, int /*column*/)
{
    if(mItemToTypeNameMap.contains(item) && qApp->mouseButtons().testFlag(Qt::LeftButton))
    {
        QString typeName = mItemToTypeNameMap.find(item).value();

        ModelObjectAppearance *pAppearance;
        QIcon icon;
        if(typeName.startsWith(QString(MODELICATYPENAME)+"_"))
        {
            icon = item->icon(0);
        }
        else
        {
            pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
            QString iconPath = pAppearance->getFullAvailableIconPath(mGfxType);
            icon.addFile(iconPath,QSize(55,55));
        }

        //Create the mimedata (text with type name)
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(typeName);

        //Initiate the drag operation
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(icon.pixmap(40,40));
        drag->setHotSpot(QPoint(20, 20));
        drag->exec(Qt::CopyAction | Qt::MoveAction);

        gpHelpPopupWidget->hide();
    }
    else if(mItemToModelicaFileNameMap.contains(item) && qApp->mouseButtons().testFlag(Qt::LeftButton))
    {
        QString filePath = mItemToModelicaFileNameMap.find(item).value();
        qDebug() << "Opening: " << filePath;

        ModelicaEditor *pEditor = new ModelicaEditor(filePath, gpCentralTabWidget);
        gpCentralTabWidget->setCurrentIndex(gpCentralTabWidget->addTab(pEditor, QFileInfo(filePath).fileName()));


    }
    else if(mFolderToContentsMap.contains(item))
    {
        QStringList typeNames = mFolderToContentsMap.find(item).value();

        mpList->clear();
        mListItemToTypeNameMap.clear();

        //Populate list widget with components
        for(int i=0; i<typeNames.size(); ++i)
        {
            ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeNames[i]);
            QListWidgetItem *tempItem = new QListWidgetItem();
            tempItem->setIcon(QIcon(pAppearance->getFullAvailableIconPath()));
            tempItem->setToolTip(pAppearance->getDisplayName());
            mListItemToTypeNameMap.insert(tempItem, typeNames[i]);
            tempItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpList->addItem(tempItem);
        }
    }
    else if((item == mpLoadLibraryItem || item == mpLoadLibraryItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        gpLibraryHandler->loadLibrary();
        return;
    }
    else if((item == mpAddModelicaFileItem || item == mpAddModelicaFileItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        //gpLibraryHandler->createNewModelicaComponent();
        gpModelicaLibrary->loadModelicaFile();
        return;
    }

    if(qApp->mouseButtons() == Qt::RightButton)
    {
        //Ignore right-click for load library and add modelica file items
        if(item == mpLoadLibraryItem || item == mpLoadLibraryItemDual ||
           item == mpAddModelicaFileItem || item == mpAddModelicaFileItemDual)
        {
            return;
        }

        //Context menu for Modelica file items
        if(mItemToModelicaFileNameMap.keys().contains(item))
        {
            QMenu contextMenu;
            QAction *pUnloadAction = contextMenu.addAction("Unload Modelica File");

            QAction *pSelectedAction = contextMenu.exec(QCursor::pos());

            if(pSelectedAction == pUnloadAction)
            {
                gpModelicaLibrary->unloadModelicaFile(mItemToModelicaFileNameMap.find(item).value());
            }
            return;
        }

        if(mpList->isVisible())     //Dual view mode
        {
            QMenu contextMenu;
            QAction *pUnloadAction = contextMenu.addAction("Unload External Library");
            QAction *pOpenFolderAction = contextMenu.addAction("Open Containing Folder");
            pUnloadAction->setEnabled(false);
            pOpenFolderAction->setEnabled(false);

            QListWidgetItem *pFirstSubComponentItem = mpList->item(0);
            QString typeName;
            if(pFirstSubComponentItem)
            {
                typeName = mListItemToTypeNameMap.find(pFirstSubComponentItem).value();
            }

            if(item->text(0) != EXTLIBSTR && gpLibraryHandler->getEntry(typeName).path.startsWith(EXTLIBSTR))
            {
                pUnloadAction->setEnabled(true);
            }

            if(item != 0 && !typeName.isEmpty())
            {
                pOpenFolderAction->setEnabled(true);
            }

            if(contextMenu.actions().isEmpty())
                return;

            QAction *pReply = contextMenu.exec(QCursor::pos());

            if(pReply == pUnloadAction)
            {
                QStringList typeNames;
                if(mItemToTypeNameMap.contains(item))
                {
                    typeNames.append(mItemToTypeNameMap.find(item).value());
                }
                if(!mItemToTypeNameMap.contains(item))
                {
                    QStringList typeNames;
                    for(int c=0; c<mpList->count(); ++c)
                    {
                        typeNames.append(mListItemToTypeNameMap.find(mpList->item(c)).value());
                    }
                    if(!gpLibraryHandler->isTypeNamesOkToUnload(typeNames))
                    {
                        return;
                    }
                    for(int s=0; s<typeNames.size(); ++s)
                    {
                        gpLibraryHandler->unloadLibraryByComponentType(typeNames[s]);
                    }
                }
                gpLibraryHandler->unloadLibraryByComponentType(typeName);
            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getModelObjectAppearancePtr(typeName)->getBasePath()));
            }
        }
        else        //Tree view mode
        {
            QMenu contextMenu;
            QAction *pUnloadAction = contextMenu.addAction("Unload External Library");
            QAction *pOpenFolderAction = contextMenu.addAction("Open Containing Folder");
            pUnloadAction->setEnabled(false);
            pOpenFolderAction->setEnabled(false);

            QTreeWidgetItem *pFirstSubComponentItem = item;

            QStringList typeNames;

            while(!mItemToTypeNameMap.contains(pFirstSubComponentItem))
            {
                pFirstSubComponentItem = pFirstSubComponentItem->child(0);
            }

            if(item->text(0) != EXTLIBSTR && gpLibraryHandler->getEntry(mItemToTypeNameMap.find(pFirstSubComponentItem).value()).path.startsWith(EXTLIBSTR))
            {
                pUnloadAction->setEnabled(true);
            }

            if(item != 0 && mItemToTypeNameMap.contains(pFirstSubComponentItem))
            {
                pOpenFolderAction->setEnabled(true);
            }

            if(contextMenu.actions().isEmpty())
                return;

            QAction *pReply = contextMenu.exec(QCursor::pos());

            if(pReply == pUnloadAction)
            {
                if(mItemToTypeNameMap.contains(item))
                {
                    typeNames.append(mItemToTypeNameMap.find(item).value());
                }
                if(!mItemToTypeNameMap.contains(item))
                {
                    QList<QTreeWidgetItem *> subItems;
                    getAllSubTreeItems(item, subItems);
                    QStringList typeNames;
                    for(int s=0; s<subItems.size(); ++s)
                    {
                        if(mItemToTypeNameMap.contains(subItems[s]))
                        {
                            typeNames.append(mItemToTypeNameMap.find(subItems[s]).value());
                        }
                    }
                    if(!gpLibraryHandler->isTypeNamesOkToUnload(typeNames))
                    {
                        return;
                    }
                    for(int s=0; s<typeNames.size(); ++s)
                    {
                        gpLibraryHandler->unloadLibraryByComponentType(typeNames[s]);
                    }
                }
            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getModelObjectAppearancePtr(mItemToTypeNameMap.find(pFirstSubComponentItem).value())->getBasePath()));
            }
        }
    }
}


void LibraryWidget::handleItemClick(QListWidgetItem *item)
{
    if(mListItemToTypeNameMap.contains(item) && qApp->mouseButtons() == Qt::LeftButton)
    {
        QString typeName = mListItemToTypeNameMap.find(item).value();
        ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
        QString iconPath = pAppearance->getFullAvailableIconPath(mGfxType);
        QIcon icon;
        icon.addFile(iconPath,QSize(55,55));

        //Create the mimedata (text with type name)
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(typeName);

        //Initiate the drag operation
        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(icon.pixmap(40,40));
        drag->setHotSpot(QPoint(20, 20));
        drag->exec(Qt::CopyAction | Qt::MoveAction);

        gpHelpPopupWidget->hide();
    }
}

void LibraryWidget::handleItemEntered(QListWidgetItem *item)
{
    QString componentName = item->toolTip();
    mpComponentNameLabel->setMaximumWidth(this->width());
    mpComponentNameLabel->setMinimumHeight(mpComponentNameLabel->height());
    mpComponentNameLabel->setFont(QFont(qApp->font().family(), std::min(10.0, .9*this->width()/(0.615*componentName.size()))));
    mpComponentNameLabel->setText(componentName);
    qDebug() << "Hovering: " << item->text() << ", " << item->toolTip();
}


void LibraryWidget::mouseMoveEvent(QMouseEvent *event)
{
    mpComponentNameLabel->clear();
    QWidget::mouseMoveEvent(event);
}

void LibraryWidget::getAllSubTreeItems(QTreeWidgetItem *pParentItem, QList<QTreeWidgetItem *> &rSubItems)
{
    rSubItems.append(pParentItem);
    for(int c=0; c<pParentItem->childCount(); ++c)
    {
        getAllSubTreeItems(pParentItem->child(c), rSubItems);
    }
}

