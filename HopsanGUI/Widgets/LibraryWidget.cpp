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

//! @todo Ok dont know where I should put this, putting it here for now /Peter
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
    QHBoxLayout *pFilterLayout = new QHBoxLayout(gpMainWindowWidget);
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
    size.rwidth() = 250;            //Set very small width. A minimum apperantly stops at resonable size.
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
                    pItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Folder.png"));
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
        QIcon icon;
//        QString iconPath = entry.pAppearance->getFullAvailableIconPath(mGfxType);
//        icon.addFile(iconPath,QSize(55,55));
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

    if(filter.isEmpty())
    {
        //Append load external library items
        mpLoadLibraryItem = new QTreeWidgetItem();
        mpLoadLibraryItem->setText(0, "Load external library");
        mpLoadLibraryItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpLoadLibraryItem->setToolTip(0, "Load external library");
        mpTree->addTopLevelItem(mpLoadLibraryItem);

        mpAddModelicaComponentItem = new QTreeWidgetItem();
        mpAddModelicaComponentItem->setText(0, "Add Modelica component");
        mpAddModelicaComponentItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddModelicaComponentItem->setToolTip(0, "Add Modelica component");
        mpTree->addTopLevelItem(mpAddModelicaComponentItem);

        mpAddCppComponentItem = new QTreeWidgetItem();
        mpAddCppComponentItem->setText(0, "Add C++ component");
        mpAddCppComponentItem->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddCppComponentItem->setToolTip(0, "Add C++ component");
        mpTree->addTopLevelItem(mpAddCppComponentItem);

        mpLoadLibraryItemDual = new QTreeWidgetItem();
        mpLoadLibraryItemDual->setText(0, "Load external library");
        mpLoadLibraryItemDual->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpLoadLibraryItemDual->setToolTip(0, "Load external library");
        mpDualTree->addTopLevelItem(mpLoadLibraryItemDual);

        mpAddModelicaComponentItemDual = new QTreeWidgetItem();
        mpAddModelicaComponentItemDual->setText(0, "Add Modelica component");
        mpAddModelicaComponentItemDual->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddModelicaComponentItemDual->setToolTip(0, "Add Modelica component");
        mpDualTree->addTopLevelItem(mpAddModelicaComponentItemDual);

        mpAddCppComponentItemDual = new QTreeWidgetItem();
        mpAddCppComponentItemDual->setText(0, "Add C++ component");
        mpAddCppComponentItemDual->setIcon(0, QIcon(QString(ICONPATH)+"Hopsan-Add.png"));
        mpAddCppComponentItemDual->setToolTip(0, "Add C++ component");
        mpDualTree->addTopLevelItem(mpAddCppComponentItemDual);
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
}


void LibraryWidget::handleItemClick(QTreeWidgetItem *item, int /*column*/)
{
    if(mItemToTypeNameMap.contains(item) && qApp->mouseButtons().testFlag(Qt::LeftButton))
    {
        QString typeName = mItemToTypeNameMap.find(item).value();
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
    else if((item == mpAddModelicaComponentItem || item == mpAddModelicaComponentItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        gpLibraryHandler->createNewModelicaComponent();
        return;
    }
    else if((item == mpAddCppComponentItem || item == mpAddCppComponentItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        gpLibraryHandler->createNewCppComponent();
        return;
    }

    if(qApp->mouseButtons() == Qt::RightButton)
    {
        if(item == mpLoadLibraryItem || item == mpLoadLibraryItemDual || item == mpAddCppComponentItem ||
           item == mpAddCppComponentItemDual || item == mpAddModelicaComponentItem || item == mpAddModelicaComponentItemDual)
        {
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
                        gpLibraryHandler->unloadLibrary(typeNames[s]);
                    }
                }
                gpLibraryHandler->unloadLibrary(typeName);
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

            if(item != 0 && mItemToTypeNameMap.contains(item))
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
                        gpLibraryHandler->unloadLibrary(typeNames[s]);
                    }
                }
            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getModelObjectAppearancePtr(mItemToTypeNameMap.find(item).value())->getBasePath()));
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
