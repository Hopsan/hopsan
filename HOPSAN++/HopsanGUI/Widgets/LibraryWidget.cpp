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

//Hopsan includes
#include "common.h"
#include "global.h"
#include "version_gui.h"
#include "LibraryWidget.h"
#include "LibraryHandler.h"
#include "CoreAccess.h"
#include "MainWindow.h"
#include "HcomWidget.h"
#include "DesktopHandler.h"
#include "Configuration.h"
#include "Dialogs/EditComponentDialog.h"


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

    QSize iconSize = QSize(24,24);  //Size of library icons

    mpTreeViewButton = new QToolButton();
    mpTreeViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryTreeView.png"));
    mpTreeViewButton->setIconSize(iconSize);
    mpTreeViewButton->setToolTip(tr("Single List View"));

    mpDualViewButton = new QToolButton();
    mpDualViewButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LibraryDualView.png"));
    mpDualViewButton->setIconSize(iconSize);
    mpDualViewButton->setToolTip(tr("Dual List View"));

    mpHelpButton = new QToolButton();
    mpHelpButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Help.png"));
    mpHelpButton->setToolTip(tr("Open Context Help"));
    mpHelpButton->setIconSize(iconSize);

    connect(gpLibraryHandler, SIGNAL(contentsChanged()), this, SLOT(update()));
    connect(mpTree,     SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpDualTree, SIGNAL(itemEntered(QTreeWidgetItem*,int)),  mpComponentNameLabel,   SLOT(clear()));
    connect(mpDualTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)),  this,                   SLOT(handleItemClick(QTreeWidgetItem*,int)));
    connect(mpList,     SIGNAL(itemPressed(QListWidgetItem*)),      this,                   SLOT(handleItemClick(QListWidgetItem*)));
    connect(mpList,     SIGNAL(itemEntered(QListWidgetItem*)),      this,                   SLOT(handleItemEntered(QListWidgetItem*)));
    connect(mpTreeViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(show()));
    connect(mpTreeViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(hide()));
    connect(mpTreeViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(hide()));
    connect(mpTreeViewButton, SIGNAL(clicked()),    mpList,                 SLOT(hide()));
    connect(mpDualViewButton, SIGNAL(clicked()),    mpTree,                 SLOT(hide()));
    connect(mpDualViewButton, SIGNAL(clicked()),    mpDualTree,             SLOT(show()));
    connect(mpDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(show()));
    connect(mpDualViewButton, SIGNAL(clicked()),    mpList,                 SLOT(show()));
    connect(mpDualViewButton, SIGNAL(clicked()),    mpComponentNameLabel,   SLOT(clear()));
    connect(mpHelpButton,     SIGNAL(clicked()),    gpMainWindow,           SLOT(openContextHelp()));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTree,                  0,0,3,4);
    pLayout->addWidget(mpDualTree,              0,0,1,4);
    pLayout->addWidget(mpComponentNameLabel,    1,0,1,4);
    pLayout->addWidget(mpList,                  2,0,1,4);
    pLayout->addWidget(mpTreeViewButton,        3,0);
    pLayout->addWidget(mpDualViewButton,        3,1);
    pLayout->addWidget(new QWidget(this),       3,2);
    pLayout->addWidget(mpHelpButton,            3,3);
    pLayout->setColumnStretch(2,1);
    this->setLayout(pLayout);
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
        if(entry.visibility == Hidden)
        {
            continue;
        }

        QStringList path = entry.path;
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
        QString iconPath = entry.pAppearance->getFullAvailableIconPath(mGfxType);
        icon.addFile(iconPath,QSize(55,55));
        pComponentItem->setIcon(0, icon);
        pComponentItem->setText(0, entry.pAppearance->getDisplayName());
        pComponentItem->setToolTip(0, entry.pAppearance->getDisplayName());
        pItem->addChild(pComponentItem);
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


void LibraryWidget::handleItemClick(QTreeWidgetItem *item, int /*column*/)
{
    qDebug() << "Clicked on: " << item->text(0);
    if(mItemToTypeNameMap.contains(item) && qApp->mouseButtons() == Qt::LeftButton)
    {
        QString typeName = mItemToTypeNameMap.find(item).value();
        qDebug() << "Clicked on: " << typeName;
        ModelObjectAppearance *pAppearance = gpLibraryHandler->getEntry(typeName).pAppearance;
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

        gpMainWindow->hideHelpPopupMessage();
    }
    else if(mFolderToContentsMap.contains(item))
    {
        QStringList typeNames = mFolderToContentsMap.find(item).value();

        mpList->clear();
        mListItemToTypeNameMap.clear();

        //Populate list widget with components
        for(int i=0; i<typeNames.size(); ++i)
        {
            ModelObjectAppearance *pAppearance = gpLibraryHandler->getEntry(typeNames[i]).pAppearance;
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
#ifdef DEVELOPMENT
        QString libDir = QFileDialog::getOpenFileName(this, tr("Choose file"), gpConfig->getExternalLibDir());
        libDir.replace("\\","/");   //Enforce unix-style on path
#else
        QString libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                       gpConfig->getExternalLibDir(),
                                                       QFileDialog::ShowDirsOnly
                                                       | QFileDialog::DontResolveSymlinks);
#endif


        if(libDir.isEmpty())
        {
            return;
        }
        else
        {
            gpConfig->setExternalLibDir(libDir);

            if(!gpConfig->hasUserLib(libDir))     //Check so that path does not already exist
            {
                gpLibraryHandler->loadLibrary(libDir/*, QStringList() << EXTLIBSTR << libDir.section("/",-1,-1)*/);    //Load and register the library in configuration
            }
            else
            {
                gpTerminalWidget->mpConsole->printErrorMessage("Error: Library " + libDir + " is already loaded!");
            }

            //checkForFailedComponents();
        }
    }
    else if((item == mpAddModelicaComponentItem || item == mpAddModelicaComponentItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Modelica);
        pEditDialog->exec();
        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0);
            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);
            int solver = pEditDialog->getSolver();

            QFile moFile(libPath+typeName+".mo");
            moFile.open(QFile::WriteOnly | QFile::Truncate);
            moFile.write(pEditDialog->getCode().toUtf8());
            moFile.close();

            coreAccess.generateFromModelica(libPath+typeName+".mo", true, solver, true);
            gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");
            update();
        }
        delete(pEditDialog);
        return;
    }
    else if((item == mpAddCppComponentItem || item == mpAddCppComponentItemDual) && qApp->mouseButtons() == Qt::LeftButton)
    {
        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Cpp);
        pEditDialog->exec();
        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);

            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);

            QFile hppFile(libPath+typeName+".hpp");
            hppFile.open(QFile::WriteOnly | QFile::Truncate);
            hppFile.write(pEditDialog->getCode().toUtf8());
            hppFile.close();

            coreAccess.generateFromCpp(libPath+typeName+".hpp");
            coreAccess.generateLibrary(libPath, QStringList() << typeName+".hpp");
            coreAccess.compileComponentLibrary(libPath+typeName+"_lib.xml");
            gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");
            update();
        }
        delete(pEditDialog);
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
                gpLibraryHandler->unloadLibrary(typeName);
            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getEntry(typeName).pAppearance->getBasePath()));
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
                while(!mItemToTypeNameMap.contains(item))
                {
                    item = item->child(0);
                }

                gpLibraryHandler->unloadLibrary(mItemToTypeNameMap.find(item).value());

            }
            else if(pReply == pOpenFolderAction)
            {
                QDesktopServices::openUrl(QUrl("file:///" + gpLibraryHandler->getEntry(mItemToTypeNameMap.find(item).value()).pAppearance->getBasePath()));
            }
        }
    }
}


void LibraryWidget::handleItemClick(QListWidgetItem *item)
{
    if(mListItemToTypeNameMap.contains(item) && qApp->mouseButtons() == Qt::LeftButton)
    {
        QString typeName = mListItemToTypeNameMap.find(item).value();
        ModelObjectAppearance *pAppearance = gpLibraryHandler->getEntry(typeName).pAppearance;
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

        gpMainWindow->hideHelpPopupMessage();
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
