/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   LibraryWidget.h
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
#include <QCompleter>
#include <QStringListModel>
#include <QInputDialog>

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
#include "MessageHandler.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GeneratorUtils.h"
#include "ModelHandler.h"

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
    mpFilterEdit->setCompleter(new QCompleter(mpFilterEdit));
    mpFilterEdit->completer()->setCompletionMode(QCompleter::PopupCompletion);
    mpFilterEdit->completer()->setCaseSensitivity(Qt::CaseInsensitive);
#if QT_VERSION >= 0x050000
    mpFilterEdit->completer()->setFilterMode(Qt::MatchContains);
#endif

    QToolButton *pClearFilterButton = new QToolButton(this);
    pClearFilterButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Discard.svg"));
    pFilterLayout->addWidget(pClearFilterButton);
    connect(pClearFilterButton, SIGNAL(clicked()), mpFilterEdit, SLOT(clear()));
    connect(pClearFilterButton, SIGNAL(clicked()), this, SLOT(update()));

//    QSize iconSize = QSize(24,24);  //Size of library icons

//    QToolButton *pTreeViewButton = new QToolButton();
//    pTreeViewButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-LibraryTreeView.svg"));
//    pTreeViewButton->setIconSize(iconSize);
//    pTreeViewButton->setToolTip(tr("Single List View"));

//    QToolButton *pDualViewButton = new QToolButton();
//    pDualViewButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-LibraryDualView.svg"));
//    pDualViewButton->setIconSize(iconSize);
//    pDualViewButton->setToolTip(tr("Dual List View"));

//    QToolButton *pHelpButton = new QToolButton();
//    pHelpButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Help.svg"));
//    pHelpButton->setToolTip(tr("Open Context Help"));
//    pHelpButton->setIconSize(iconSize);

    connect(gpLibraryHandler, SIGNAL(contentsChanged()), this, SLOT(update()));
    connect(mpTree,     SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpDualTree, SIGNAL(itemEntered(QTreeWidgetItem*,int)),  mpComponentNameLabel,   SLOT(clear()));
    connect(mpDualTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpList,     SIGNAL(itemPressed(QListWidgetItem*)),      this,                   SLOT(handleItemClick(QListWidgetItem*)));
    connect(mpList,     SIGNAL(itemEntered(QListWidgetItem*)),      this,                   SLOT(handleItemEntered(QListWidgetItem*)));
//    connect(pTreeViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(show()));
//    connect(pTreeViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(hide()));
//    connect(pTreeViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(hide()));
//    connect(pTreeViewButton, SIGNAL(clicked()),    mpList,                 SLOT(hide()));
//    connect(pDualViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(hide()));
//    connect(pDualViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(show()));
//    connect(pDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(show()));
//    connect(pDualViewButton, SIGNAL(clicked()),    mpList,                 SLOT(show()));
//    connect(pDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(clear()));
//    connect(pHelpButton,     SIGNAL(clicked()),    gpHelpPopupWidget,           SLOT(openContextHelp()));
    connect(mpFilterEdit,   SIGNAL(textChanged(QString)), this, SLOT(update()));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTree,                  0,0,3,4);
    pLayout->addWidget(mpDualTree,              0,0,1,4);
    pLayout->addWidget(mpComponentNameLabel,    1,0,1,4);
    pLayout->addWidget(mpList,                  2,0,1,4);
    pLayout->addLayout(pFilterLayout,           3,0,1,4);
//    pLayout->addWidget(pTreeViewButton,        4,0);
//    pLayout->addWidget(pDualViewButton,        4,1);
//    pLayout->addWidget(new QWidget(this),       4,2);
//    pLayout->addWidget(pHelpButton,            4,3);
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

    //Make sure all libraries have a folder, even if they contain no components
    QTreeWidgetItem *pExternalItem = new QTreeWidgetItem();
    QTreeWidgetItem *pExternalDualItem = nullptr;
    pExternalItem->setFont(0,boldFont);
    pExternalItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-FolderExternal.svg"));
    pExternalItem->setText(0, componentlibrary::roots::externalLibraries);
    pExternalItem->setToolTip(0, componentlibrary::roots::externalLibraries);
    mpTree->addTopLevelItem(pExternalItem);
    pExternalDualItem = pExternalItem->clone();
    mpDualTree->addTopLevelItem(pExternalDualItem);
    for(auto lib : gpLibraryHandler->getLibraries(ExternalLib)) {
        QTreeWidgetItem *pItem = new QTreeWidgetItem();
        QTreeWidgetItem *pDualItem = nullptr;
        pItem->setFont(0,boldFont);
        pItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-FolderExternal.svg"));
        pItem->setText(0, lib->name);
        pItem->setToolTip(0, lib->name);
        pExternalItem->addChild(pItem);
        pDualItem = pItem->clone();
        pExternalDualItem->addChild(pDualItem);
        mItemToLibraryMap[pItem] = lib;
        mItemToLibraryMap[pDualItem] = lib;
    }

    for(const QString typeName : gpLibraryHandler->getLoadedTypeNames()) {
        ComponentLibraryEntry entry = gpLibraryHandler->getEntry(typeName);
        if(entry.visibility == Hidden || !(entry.pAppearance->getDisplayName().toLower().contains(filter.toLower())))
        {
            continue;
        }

        QStringList path;
        if(filter.isEmpty())
        {
            path = entry.displayPath;
        }

        QTreeWidgetItem *pItem = nullptr;
        QTreeWidgetItem *pDualItem = nullptr;
        while(!path.isEmpty())
        {
            QString folder = path.first();
            path.removeFirst();
            if(pItem == nullptr)
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
                if(pItem == nullptr)
                {
                    //Add top-level folder to tree view
                    pItem = new QTreeWidgetItem();
                    pItem->setFont(0,boldFont);
                    if(folder == componentlibrary::roots::externalLibraries || folder == componentlibrary::roots::fmus)
                    {
                        pItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-FolderExternal.svg"));
                    }
                    else
                    {
                        pItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-Folder.svg"));
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
                    if( pTopItem->text(0) == componentlibrary::roots::externalLibraries || pTopItem->text(0) == componentlibrary::roots::fmus)
                        pNewItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-FolderExternal.svg"));
                    else
                        pNewItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-Folder.svg"));
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
        if(Disabled == entry.disabled) {
            pComponentItem->setTextColor(0, QColor("red"));
        }
        if(pItem)
        {
            pItem->addChild(pComponentItem);
            mItemToLibraryMap[pItem] = entry.pLibrary;
            mItemToLibraryMap[pComponentItem] = entry.pLibrary;
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
            SharedModelObjectAppearanceT pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
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

        if((*itt)->childCount() > 0 && (*itt)->text(0) != componentlibrary::roots::externalLibraries)
        {
            (*itt)->setText(0, "0000000000"+(*itt)->text(0));       //Prepends a lot of zeros to subfolders, to make sure they are sorted on top (REALLY ugly, but it works)
        }
        ++itt;
    }
    pExternalItem = nullptr;
    for(int t=0; t<mpTree->topLevelItemCount(); ++t)
    {
        if(mpTree->topLevelItem(t)->text(0) == componentlibrary::roots::externalLibraries)
        {
            pExternalItem = mpTree->takeTopLevelItem(t);
            break;
        }
    }
    mpTree->sortItems(0, Qt::AscendingOrder);
    if(pExternalItem)
    {
        mpTree->insertTopLevelItem(mpTree->topLevelItemCount(),pExternalItem);
        pExternalItem->setExpanded(true);
    }
    pExternalItem = nullptr;
    for(int t=0; t<mpDualTree->topLevelItemCount(); ++t)
    {
        if(mpDualTree->topLevelItem(t)->text(0) == componentlibrary::roots::externalLibraries)
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
        if((*itt2)->childCount() > 0 && (*itt2)->text(0) != componentlibrary::roots::externalLibraries)
        {
            (*itt2)->setText(0, (*itt2)->text(0).remove(0,10)); //Remove the extra zeros from subfolders (see above)
        }
        ++itt2;
    }

    QTreeWidgetItem *pModelicaComponentsItem = new QTreeWidgetItem();
    pModelicaComponentsItem->setIcon(0, QIcon(QString(ICONPATH)+"svg/Hopsan-FolderModelica.svg"));
    pModelicaComponentsItem->setText(0, componentlibrary::roots::modelicaComponents);
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
        mpLoadLibraryItem->setIcon(0, QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
        mpLoadLibraryItem->setToolTip(0, "Load external library");
        mpTree->addTopLevelItem(mpLoadLibraryItem);

        mpAddModelicaFileItem = new QTreeWidgetItem();
        mpAddModelicaFileItem->setText(0, "Load Modelica file");
        mpAddModelicaFileItem->setIcon(0, QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
        mpAddModelicaFileItem->setToolTip(0, "Load Modelica file");
        mpTree->addTopLevelItem(mpAddModelicaFileItem);

        mpLoadLibraryItemDual = new QTreeWidgetItem();
        mpLoadLibraryItemDual->setText(0, "Load external library");
        mpLoadLibraryItemDual->setIcon(0, QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
        mpLoadLibraryItemDual->setToolTip(0, "Load external library");
        mpDualTree->addTopLevelItem(mpLoadLibraryItemDual);

        mpAddModelicaFileItemDual = new QTreeWidgetItem();
        mpAddModelicaFileItemDual->setText(0, "Load Modelica file");
        mpAddModelicaFileItemDual->setIcon(0, QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
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
        pModelicaItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-New.svg"));
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

    //Update auto completer in filter line edit
    QStringList allDisplayNames;
    for(const QString &typeName : gpLibraryHandler->getLoadedTypeNames())
    {
        ComponentLibraryEntry entry = gpLibraryHandler->getEntry(typeName);
        allDisplayNames << entry.pAppearance->getDisplayName();
    }
    mpFilterEdit->completer()->setModel(new QStringListModel(allDisplayNames, mpFilterEdit->completer()));
}


void LibraryWidget::handleItemClick(QTreeWidgetItem *item, int column)
{
    qDebug() << "Item click on: " << item->text(0);

    Q_UNUSED(column)
    if(isComponentItem(item) && qApp->mouseButtons().testFlag(Qt::LeftButton))
    {
        QString typeName = mItemToTypeNameMap.find(item).value();
        if(gpLibraryHandler->getEntry(typeName).disabled == Enabled) {
            QIcon icon;
            if(typeName.startsWith(QString(MODELICATYPENAME)+"_"))
            {
                icon = item->icon(0);
            }
            else
            {
                SharedModelObjectAppearanceT pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
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
            SharedModelObjectAppearanceT pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeNames[i]);
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

            if(item->text(0) != componentlibrary::roots::externalLibraries && gpLibraryHandler->getEntry(typeName).displayPath.startsWith(componentlibrary::roots::externalLibraries))
            {
                pUnloadAction->setEnabled(true);
            }

            if(item != nullptr && !typeName.isEmpty())
            {
                pOpenFolderAction->setEnabled(true);
            }

            if(contextMenu.actions().isEmpty())
                return;

            QAction *pReply = contextMenu.exec(QCursor::pos());

            if(pReply == pUnloadAction)
            {
                QStringList typeNames;
                if(isComponentItem(item))
                {
                    typeNames.append(mItemToTypeNameMap.find(item).value());
                }
                else
                {
                    for(int c=0; c<mpList->count(); ++c)
                    {
                        typeNames.append(mListItemToTypeNameMap.find(mpList->item(c)).value());
                    }
                    if(!gpLibraryHandler->isTypeNamesOkToUnload(typeNames))
                    {
                        return;
                    }
                }
                for(const QString &typeName : typeNames)
                {
                    gpLibraryHandler->unloadLibraryByComponentType(typeName);
                }
            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getModelObjectAppearancePtr(typeName)->getBasePath()));
            }
        }
        else        //Tree view mode
        {
            QMenu contextMenu;
            QAction *pUnloadAllAction = contextMenu.addAction("Unload All External Libraries");
            QAction *pUnloadAction = contextMenu.addAction("Unload External Library");
            QAction *pOpenFolderAction = contextMenu.addAction("Open Containing Folder");
            QAction *pEditXMLAction = contextMenu.addAction("Edit XML Description");
            QAction *pEditCodeAction = contextMenu.addAction("Edit Source Code");
            QAction *pRecompileAction = contextMenu.addAction("Recompile");
            QAction *pReloadAction = contextMenu.addAction("Reload");
            QAction *pCheckConsistenceAction = contextMenu.addAction("Check source/XML consistency");
            QAction *pAddComponentAction = contextMenu.addAction("Add New Component");
            pUnloadAllAction->setEnabled(false);
            pUnloadAction->setEnabled(false);
            pOpenFolderAction->setEnabled(false);
            pEditXMLAction->setEnabled(false);
            pEditCodeAction->setEnabled(false);
            pRecompileAction->setEnabled(false);
            pReloadAction->setEnabled(false);
            pCheckConsistenceAction->setEnabled(false);
            pAddComponentAction->setEnabled(false);

            QTreeWidgetItem *pFirstSubComponentItem = item;

            QStringList typeNames;

            while(!isComponentItem(pFirstSubComponentItem))
            {
                if(nullptr == pFirstSubComponentItem) {
                    break;
                }
                pFirstSubComponentItem = pFirstSubComponentItem->child(0);
            }

            //Enable unload all only for top-level external libraries folder
            if(item->text(0) == componentlibrary::roots::externalLibraries)
            {
                pUnloadAllAction->setEnabled(true);
            }

            //Enable external library actions (also for empty libraries)
            if(item->text(0) != componentlibrary::roots::externalLibraries &&
              (item->parent() != nullptr && item->parent()->text(0) == componentlibrary::roots::externalLibraries ||
              (pFirstSubComponentItem != nullptr && gpLibraryHandler->getEntry(mItemToTypeNameMap.find(pFirstSubComponentItem).value()).displayPath.startsWith(componentlibrary::roots::externalLibraries))))
            {
                pRecompileAction->setEnabled(true);
                pEditXMLAction->setEnabled(true);
                pEditCodeAction->setEnabled(true);
                pUnloadAction->setEnabled(true);
                pReloadAction->setEnabled(true);
                pCheckConsistenceAction->setEnabled(true);
                pAddComponentAction->setEnabled(true);
            }

            //Enable unloading of FMUs
            if(pFirstSubComponentItem != nullptr &&
               item->text(0) != componentlibrary::roots::fmus &&
               gpLibraryHandler->getEntry(mItemToTypeNameMap.find(pFirstSubComponentItem).value()).displayPath.startsWith(componentlibrary::roots::fmus)) {
                pUnloadAction->setEnabled(true);
            }

            if(item) {
                pOpenFolderAction->setEnabled(true);
            }

            if(contextMenu.actions().isEmpty())
                return;

            // Execute pop-up menu
            QAction *pReply = contextMenu.exec(QCursor::pos());

            // Handle unload
            if (pReply == pUnloadAction) {
                gpLibraryHandler->unloadLibrary(mItemToLibraryMap[item]);
            }
            // Handle unload all
            else if(pReply == pUnloadAllAction) {
                QVector<SharedComponentLibraryPtrT> libs = gpLibraryHandler->getLibraries(ExternalLib);
                for(SharedComponentLibraryPtrT pLib : libs) {
                    gpLibraryHandler->unloadLibrary(pLib);
                }
            }
            // Handle reload
            else if (pReply == pReloadAction) {
                gpModelHandler->saveState();
                // First unload the library
                SharedComponentLibraryPtrT pLib = mItemToLibraryMap[item];
                QString libPath = pLib->xmlFilePath;
                   if (gpLibraryHandler->unloadLibrary(pLib)) {
                    // Now reload the library
                    gpLibraryHandler->loadLibrary(libPath);
                }
                gpModelHandler->restoreState();
            }
            // Handle recompile
            else if (pReply == pRecompileAction) {
                gpModelHandler->saveState();
                SharedComponentLibraryPtrT pLib = mItemToLibraryMap[item];
                // First unload the library
                QString libPath = pLib->xmlFilePath;
                if (gpLibraryHandler->unloadLibrary(pLib)) {
                    // We use the core generator directly to avoid calling the save state code in the library handler it does not seem to be working so well
                    // But since we only need to unload one particular library this should work
                    //! @todo fix the problem with save state
                    auto spGenerator = createDefaultImportGenerator();
                    if (!spGenerator->compileComponentLibrary(libPath)) {
                        gpMessageHandler->addErrorMessage("Library compiler failed");
                    }

                    // Now reload the library
                    gpLibraryHandler->loadLibrary(libPath);
                }
                gpModelHandler->restoreState();
            }
            // Handle check consistency
            else if (pReply == pCheckConsistenceAction) {
                SharedComponentLibraryPtrT pLib = mItemToLibraryMap[item];
                auto spGenerator = createDefaultGenerator(false);
                if (!spGenerator->checkComponentLibrary(pLib->xmlFilePath)) {
                    gpMessageHandler->addWarningMessage(QString("The library '%1' has inconsistent component registration, this may cause exported models to fail.").arg(pLib->xmlFilePath));
                }
            }
            else if(pReply == pAddComponentAction) {
                SharedComponentLibraryPtrT pLib = mItemToLibraryMap[item];
                //QString typeName = QInputDialog::getText(this,"Add component","Type name: ");
                gpLibraryHandler->addComponentToLibrary(pLib);
            }
            else if(pReply == pOpenFolderAction) {
                QString path;
                if(pFirstSubComponentItem == nullptr) {
                    path = QFileInfo(mItemToLibraryMap[item]->libFilePath).absolutePath();
                }
                else {
                    path = gpLibraryHandler->getModelObjectAppearancePtr(mItemToTypeNameMap.find(pFirstSubComponentItem).value())->getBasePath();
                }
                QDesktopServices::openUrl(QUrl("file:///" + path));
            }
            else if(pReply == pEditXMLAction) {
                if(!isComponentItem(item)) {
                    //Edit library XML file
                    SharedComponentLibraryPtrT pLibrary = mItemToLibraryMap[item];
                    if (pLibrary) {
                        gpModelHandler->loadTextFile(pLibrary->xmlFilePath);
                    }
                }
                else {
                    QFileInfo xmlFile = gpLibraryHandler->getModelObjectAppearancePtr(mItemToTypeNameMap.find(pFirstSubComponentItem).value())->getXMLFile();
                    gpModelHandler->loadTextFile(xmlFile.absoluteFilePath());
                }
            }
            else if(pReply == pEditCodeAction) {
                if(!isComponentItem(item)) {
                    //Edit library source files
                    SharedComponentLibraryPtrT pLibrary = mItemToLibraryMap[item];
                    if (pLibrary) {
                        for(QString file : pLibrary->sourceFiles) {
                            gpModelHandler->loadTextFile(file);
                        }
                    }
                }
                else {
                    //Edit component source file
                    QString typeName = gpLibraryHandler->getModelObjectAppearancePtr(mItemToTypeNameMap.find(pFirstSubComponentItem).value())->getTypeName();
                    auto appearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
                    QString basePath = appearance->getBasePath();
                    if(!basePath.isEmpty()) {
                        basePath.append("/");
                    }
                    QString sourceFile = appearance->getSourceCodeFile();
                    gpModelHandler->loadTextFile(basePath+sourceFile);
                }
            }
        }
    }
}


void LibraryWidget::handleItemClick(QListWidgetItem *item)
{
    if(mListItemToTypeNameMap.contains(item) && qApp->mouseButtons() == Qt::LeftButton)
    {
        QString typeName = mListItemToTypeNameMap.find(item).value();
        SharedModelObjectAppearanceT pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName);
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

bool LibraryWidget::isComponentItem(QTreeWidgetItem *item)
{
    return mItemToTypeNameMap.contains(item);
}
