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

#define printVar(x) qDebug() << #x << " = " << x

#include <QtGui>

#include "Configuration.h"
#include "DesktopHandler.h"
#include "LibraryWidget.h"
#include "MainWindow.h"
#include "Widgets/HcomWidget.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Dialogs/ComponentGeneratorDialog.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/ProjectTabWidget.h"
#include "CoreAccess.h"
#include "common.h"
#include "version_gui.h"

using namespace std;
using namespace hopsan;

//! @todo Ok dont know where I should put this, putting it here for now /Peter
QString gHopsanCoreVersion = getHopsanCoreVersion();

//! @todo Make "External Libraries" a reserved word

//! @brief Helpfunction to split full typename into type and subtype
void splitFullTypeString(const QString str, QString &rType, QString &rSubType)
{
    rType.clear(); rSubType.clear();
    QStringList list = str.split("|");
    rType = list[0];
    if (list.size()>1)
    {
        rSubType = list[1];
    }
}

//! @brief Helpfunction to create full typename from type and subtype
//! @returns The full typename type|subtype, or type is subtype was empty
QString makeFullTypeString(const QString &rType, const QString &rSubType)
{
    if (rSubType.isEmpty())
    {
        return rType;
    }
    else
    {
       return rType+"|"+rSubType;
    }
}

class LibraryComponent
{
public:
    LibraryComponent(ModelObjectAppearance *pAppearanceData);
    QIcon getIcon(GraphicsTypeEnumT gfxType);
    QString getName();
    QString getFullTypeName();
    ModelObjectAppearance *getAppearanceData();

private:
    ModelObjectAppearance *mpAppearanceData;
    QIcon mUserIcon;
    QIcon mIsoIcon;
};

//! @brief Constructor for the library widget
//! @param parent Pointer to the parent (main window)
LibraryWidget::LibraryWidget(QWidget *parent)
        :   QWidget(parent)
{
    mUpConvertAllCAF = UndecidedToAll;
    mpCoreAccess = new CoreLibraryAccess();

    //! @todo Dont know if this is the right place to do this, but we need to do it early
    // We want to register certain GUI specific KeyValues in the core to prevent external libs from loading components with theses typenames
    mpCoreAccess->reserveComponentTypeName(HOPSANGUICONTAINERPORTTYPENAME);
    mpCoreAccess->reserveComponentTypeName(HOPSANGUISYSTEMTYPENAME);
    mpCoreAccess->reserveComponentTypeName(HOPSANGUIGROUPTYPENAME);

    mpContentsTree = new LibraryContentsTree();
    mpSecretHiddenContentsTree = new LibraryContentsTree();

    //mpTree = new LibraryTreeWidget(this);
    mpTree = new LibraryTreeWidget(this);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);
    mpTree->setMouseTracking(true);
    this->setAttribute(Qt::WA_MouseNoMask);

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
    mpGenerateComponentButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewComponent.png"));
    mpGenerateComponentButton->setIconSize(iconSize);
    mpGenerateComponentButton->setToolTip(tr("Generate New Component (experimental)"));
    mpLoadExternalButton = new QToolButton();
    mpLoadExternalButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LoadLibrary.png"));
    mpLoadExternalButton->setIconSize(iconSize);
    mpLoadExternalButton->setToolTip(tr("Load External Library"));
    mpHelpAction = new QAction("Open Context Help", this);
    mpHelpAction->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Help.png"));
    mpHelpAction->setToolTip("Open Context Help");
    mpHelpToolBar = new QToolBar(this);
    mpHelpToolBar->addAction(mpHelpAction);

    connect(mpTreeViewButton, SIGNAL(clicked()), this, SLOT(setListView()));
    connect(mpDualViewButton, SIGNAL(clicked()), this, SLOT(setDualView()));
    connect(mpGenerateComponentButton, SIGNAL(clicked()), this, SLOT(generateComponent()));
    connect(mpLoadExternalButton, SIGNAL(clicked()), this, SLOT(addExternalLibrary()));
    connect(mpHelpAction, SIGNAL(triggered()), gpMainWindow, SLOT(openContextHelp()));

    mpGrid = new QGridLayout(this);
    mpGrid->addWidget(mpTree,                       0,0,1,7);
    mpGrid->addWidget(mpComponentNameField,         1,0,1,7);
    mpGrid->addWidget(mpList,                       2,0,1,7);
    mpGrid->addWidget(mpTreeViewButton,             3,0,1,1);
    mpGrid->addWidget(mpDualViewButton,             3,1,1,1);
    mpGrid->addWidget(mpGenerateComponentButton,    3,2,1,1);
    mpGrid->addWidget(mpLoadExternalButton,         3,3,1,1);
#ifdef DEVELOPMENT
    mpGrid->addWidget(mpHelpToolBar,                3,5,1,1);
#endif
    mpGrid->setContentsMargins(4,4,4,4);
    mpGrid->setHorizontalSpacing(0);

    setLayout(mpGrid);
    this->setMouseTracking(true);

    mViewMode = gConfig.getLibraryStyle();
    this->setGfxType(UserGraphics);     //Also updates the widget
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

    //connect(mpTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(editComponent(QTreeWidgetItem*, int)), Qt::UniqueConnection);
    //connect(mpTree, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(initializeDrag(QTreeWidgetItem*, int)), Qt::UniqueConnection);
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
    //connect(mpList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editComponent(QListWidgetItem*)), Qt::UniqueConnection);

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

    connect(mpTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(editComponent(QTreeWidgetItem*, int)), Qt::UniqueConnection);
    //connect(mpList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(initializeDrag(QListWidgetItem*)), Qt::UniqueConnection);
}


//! @brief Initializes drag operation to workspace from a list widget item
//! @param item List widget item
void LibraryWidget::initializeDrag(QListWidgetItem *item)
{
    if(!mListItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString fullTypeName = mListItemToContentsMap.find(item).value()->getFullTypeName();
    QIcon icon = mListItemToContentsMap.find(item).value()->getIcon(mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(fullTypeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);

    mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();
}


void LibraryWidget::editComponent(QListWidgetItem *item)
{
    if(!mListItemToContentsMap.contains(item)) return;
    mEditComponentTypeName = mListItemToContentsMap.find(item).value()->getFullTypeName();
    editComponent();
}


void LibraryWidget::editComponent(QTreeWidgetItem *item, int /*dummy*/)
{
    if(!mTreeItemToContentsMap.contains(item)) return;
    mEditComponentTypeName = mTreeItemToContentsMap.find(item).value()->getFullTypeName();
    editComponent();
}


void LibraryWidget::editComponent()
{
    //return;     //Disabled because work-in-progress

    qDebug() << "Edit component!";
    QString basePath = getAppearanceData(mEditComponentTypeName)->getBasePath();
    QString fileName = getAppearanceData(mEditComponentTypeName)->getSourceCodeFile();
    QString libPath = getAppearanceData(mEditComponentTypeName)->getLibPath();
    bool isRecompilable = getAppearanceData(mEditComponentTypeName)->isRecompilable();

    printVar(mEditComponentTypeName);
    printVar(basePath);
    printVar(fileName);

    if(fileName.isEmpty() || libPath.isEmpty() || !isRecompilable) return;

    //Read source code from file
    QFile sourceFile(basePath+fileName);
    if(!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString sourceCode;
    while(!sourceFile.atEnd())
    {
        sourceCode.append(sourceFile.readLine());
    }
    sourceFile.close();
    sourceCode = QString(sourceCode.toUtf8());

    //Create the dialog
    QDialog *pDialog = new QDialog(this);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pDialog->setLayout(pLayout);

    mpEditComponentTextEdit = new QTextEdit(this);
    mpEditComponentTextEdit->setPlainText(sourceCode);
    //mpEditComponentTextEdit->resize(640,480);
    pLayout->addWidget(mpEditComponentTextEdit);
    CppHighlighter *pCppHighlighter = new CppHighlighter(mpEditComponentTextEdit->document());

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(this);
    pLayout->addWidget(pButtonBox);

    QPushButton *pCancelButton = new QPushButton("Cancel", this);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);

    QPushButton *pDoneButton = new QPushButton("Recompile", this);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::AcceptRole);

    connect(pDoneButton, SIGNAL(clicked()), pDialog, SLOT(accept()));
    connect(pCancelButton, SIGNAL(clicked()), pDialog, SLOT(reject()));
    connect(pDialog, SIGNAL(destroyed()), pCppHighlighter, SLOT(deleteLater()));
    connect(pDialog, SIGNAL(accepted()), this, SLOT(recompileComponent()));

    pDialog->resize(640,480);
    pDialog->exec();
}


void LibraryWidget::recompileComponent()
{
    qDebug() << "Recompiling!";
    printVar(mEditComponentTypeName);

    QDateTime time = QDateTime();
    uint t = time.currentDateTime().toTime_t();     //Number of milliseconds since 1970
    double rd = rand() / (double)RAND_MAX;
    int r = int(rd*1000000.0);                      //Random number between 0 and 1000000
    QString randomName = mEditComponentTypeName+QString::number(t)+QString::number(r);
    QString libPath = getAppearanceData(mEditComponentTypeName)->getLibPath();
    QString basePath = getAppearanceData(mEditComponentTypeName)->getBasePath();
    QString fileName = getAppearanceData(mEditComponentTypeName)->getSourceCodeFile();
    QString sourceCode = mpEditComponentTextEdit->toPlainText();

    printVar(randomName);
    printVar(libPath);

    //Read source code from file
    QFile oldSourceFile(basePath+fileName);
    if(!oldSourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString oldSourceCode;
    while(!oldSourceFile.atEnd())
    {
        oldSourceCode.append(oldSourceFile.readLine());
    }
    oldSourceFile.close();

    QFile sourceFile(basePath+fileName);
    if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        return;
    sourceFile.write(sourceCode.toStdString().c_str());
    sourceFile.close();

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();

    pCoreAccess->compileComponentLibrary(basePath+libPath, randomName);

    QString newLibFileName = QDir::cleanPath(basePath+libPath)+"/"+randomName+".dll";

    if(!QFile::exists(newLibFileName))
    {
        qDebug() << "Failure!";
        QFile sourceFile(basePath+fileName);
        if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
            return;
        sourceFile.write(oldSourceCode.toStdString().c_str());
        sourceFile.close();
        return;
    }

    qDebug() << "Success!";

    gpMainWindow->mpProjectTabs->saveState();

    if(!mpCoreAccess->loadComponentLib(newLibFileName))
    {
        qDebug() << "Failed to load library!";
        QFile sourceFile(basePath+fileName);
        if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
            return;
        sourceFile.write(oldSourceCode.toStdString().c_str());
        sourceFile.close();
    }

    qDebug() << "Loaded successfully!";

    //Unload all dll:s in folder
    QDir libDir(basePath+libPath);
    QStringList libList = libDir.entryList(QStringList() << "*.dll");
    for(int j=0; j<libList.size(); ++j)
    {
        QString fileName = QDir::cleanPath(basePath+libPath)+"/"+libList[j];
        mpCoreAccess->unLoadComponentLib(fileName);
        if(fileName != newLibFileName)
        {
            QFile::rename(fileName, fileName+"butnotanymore");
        }
    }

    if(!mpCoreAccess->loadComponentLib(newLibFileName))
    {
        qDebug() << "Failed to load library!";
        QFile sourceFile(basePath+fileName);
        if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
            return;
        sourceFile.write(oldSourceCode.toStdString().c_str());
        sourceFile.close();
    }

    qDebug() << "Loaded successfully!";




    gpMainWindow->mpProjectTabs->restoreState();

}


//! @brief Initializes drag operation to workspace from a tree widget item
//! @param item Tree widget item
void LibraryWidget::initializeDrag(QTreeWidgetItem *item, int /*dummy*/)
{
    if(!mTreeItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString fullTypeName = mTreeItemToContentsMap.find(item).value()->getFullTypeName();
    QIcon icon = mTreeItemToContentsMap.find(item).value()->getIcon(mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(fullTypeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}


void LibraryWidget::clearHoverEffects()
{
    mpComponentNameField->setText(QString());
    mpTree->setFrameShape(QFrame::StyledPanel);
    mpList->setFrameShape(QFrame::StyledPanel);
}


//! @brief Loads a component library from specified directory
//! First loads DLL files, then XML. DLL files are not necessary if component is already loaded.
//! @param libDir Directory to check
//! @param external Used to indicate that the library is external (places contents under "External Libraries")
void LibraryWidget::loadLibrary(QString libDir, const InternalExternalEnumT int_ext, QString libName)
{
    // Don't add empty folders to the library
    if (libDir.isEmpty())
    {
        return;
    }

    // Chop the final / if it exists will be added back bellow
    if ( libDir.endsWith('/') )
    {
        libDir.chop(1);
    }

    //Create a QDir object that contains the info about the library directory
    QDir libDirObject(libDir);

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gDesktopHandler.getBackupPath() + "/updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    if(int_ext == External)
    {
        LibraryContentsTree *pExternalTree;
        if(libName.isEmpty())
        {
            libName = "External Libraries";
        }
        if(!mpContentsTree->findChildByName(libName))
        {
            pExternalTree = mpContentsTree->addChild(libName);
        }
        else
        {
            pExternalTree = mpContentsTree->findChildByName(libName);
        }

        libDirObject.cdUp();
        loadLibraryFolder(libDir, libDirObject.absolutePath()+"/", true, pExternalTree);
    }
    else
    {
        loadLibraryFolder(libDir+"/", libDir+"/", false, mpContentsTree);
        libDirObject.setFilter(QDir::AllDirs);
        QStringList subDirList = libDirObject.entryList();
        subDirList.removeAll(".");
        subDirList.removeAll("..");
        subDirList.removeAll(".svn");
        for(int i=0; i<subDirList.size(); ++i)
        {
            loadLibraryFolder(libDir+"/"+subDirList.at(i), libDir+"/", true, mpContentsTree);
        }
    }

    update();       //Redraw the library
}


//! @brief Slots that opens the component generator dialog
void LibraryWidget::generateComponent()
{
    gpMainWindow->getComponentGeneratorDialog()->show();
    gpMainWindow->getComponentGeneratorDialog()->raise();
    gpMainWindow->getComponentGeneratorDialog()->activateWindow();
}


//! @brief Adds a new external library to Hopsan
//! @param libDir Directory to add (empty = user selection)
void LibraryWidget::addExternalLibrary(QString libDir)
{
    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir
    if(libDir.isEmpty())    //Let user select a directory if no directory is specified
    {
        libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                   gConfig.getExternalLibDir(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    }
    libDir.replace("\\","/");   //Enforce unix-style on path

    if(!libDir.isEmpty())
    {
        gConfig.setExternalLibDir(libDir);
    }

    if(!gConfig.hasUserLib(libDir))     //Check so that path does not already exist
    {
        loadAndRememberExternalLibrary(libDir);    //Load and register the library in configuration
    }
    else
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Error: Library " + libDir + " is already loaded!");
    }
}


void LibraryWidget::importFmu()
{
    //Load .fmu file and create paths
    QString filePath = QFileDialog::getOpenFileName(this, tr("Import Functional Mockup Unit (FMU)"),
                                                    gConfig.getFmuImportDir(),
                                                    tr("Functional Mockup Unit (*.fmu)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;

    QFileInfo fmuFileInfo = QFileInfo(filePath);
    gConfig.setFmuImportDir(fmuFileInfo.absolutePath());

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateFromFmu(filePath);
    delete(pCoreAccess);
}



//! @brief Wrapper function that loads an external library
//! @param libDir Directory to the library
void LibraryWidget::loadAndRememberExternalLibrary(const QString libDir, const QString libName)
{
    gConfig.addUserLib(libDir, libName);     //Register new library in configuration
    loadLibrary(libDir, External, libName);
}

//! @brief Load contents (xml files) of dir into SecretHidden library map that is not vissible in the libary
//! @todo Lots of duplicate code in this function and otehr load function, should try to break out common sub help functions
void LibraryWidget::loadHiddenSecretDir(QString dir)
{
    qDebug() << "Trying to load secret dir: " << dir;

    QDir libDirObject(dir);


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
        QString filename = dir + "/" + libList.at(i);
        qDebug() << "Trying to load: " << filename << " in Core";
        mpCoreAccess->loadComponentLib(filename);
    }


    // Append components
    filters.clear();
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
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file or not a text file: " + filename);
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
                gpMainWindow->mpTerminalWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
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
            gpMainWindow->mpTerminalWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan ComponentAppearance Data file.");
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
                gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("When loading graphics, ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)", "componentnotregistered");
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


void LibraryWidget::addReplacement(QString type1, QString type2)
{
    qDebug() << "Adding replacement: " << type1 << ", " << type2;

    if(mReplacementsMap.contains(type1))
    {
        if(!mReplacementsMap.find(type1).value().contains(type2))
        {
            mReplacementsMap.find(type1).value().append(type2);
        }
    }
    else
    {
        mReplacementsMap.insert(type1, QStringList() << type2);
    }

    if(mReplacementsMap.contains(type2))
    {
        if(!mReplacementsMap.find(type2).value().contains(type1))
        {
            mReplacementsMap.find(type2).value().append(type1);
        }
    }
    else
    {
        mReplacementsMap.insert(type2, QStringList() << type1);
    }
}


QStringList LibraryWidget::getReplacements(QString type)
{
    if(mReplacementsMap.contains(type))
    {
        return mReplacementsMap.find(type).value();
    }
    return QStringList();
}


//! @brief Recursive function that searches through subdirectories and adds components to the library contents tree
//! @param libDir Current directory
//! @param pParentTree Current contents tree node
void LibraryWidget::loadLibraryFolder(QString libDir, const QString libRootDir, const bool doRecurse, LibraryContentsTree *pParentTree)
{
    QDir libDirObject(libDir);
    if(!libDirObject.exists() && gConfig.hasUserLib(libDir))
    {
        gConfig.removeUserLib(libDir);      //Remove user lib if it does not exist
        return;
    }

    //QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));
    QString libName = libDirObject.dirName();

    //qDebug() << "Adding tree entry: " << libName;
    LibraryContentsTree *pTree = pParentTree->addChild(libName);        //Create the node
    pTree->mLibDir = libDir;
    libName = pTree->mName; //Reset name variable to new unique name

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
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(libDirObject.path() + ": Could not find any working Hopsan library in specified folder!");
        gpMainWindow->mpTerminalWidget->checkMessages();
        pParentTree->removeChild(libName);
        gConfig.removeUserLib(libDirObject.path());
        delete pTree;
        return;     //No point in continuing since no library was found
    }
    gpMainWindow->mpTerminalWidget->checkMessages();

    // Load Component XML (CAF Files)
    filters.clear();
    filters << "*.xml";                     //Create the name filter
    libDirObject.setFilter(QDir::NoFilter);
    libDirObject.setNameFilters(filters);   //Set the name filter

    QStringList xmlFileList  = libDirObject.entryList();    //Create a list with all name of the files in dir libDir
    for (int i = 0; i < xmlFileList.size(); ++i)        //Iterate over the file names
    {
        QString filename = libDir + "/" + xmlFileList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open file or not a text file: " + filename);
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
                gpMainWindow->mpTerminalWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
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
                    if (mUpConvertAllCAF==UndecidedToAll)
                    {
                        QMessageBox questionBox(this);
                        QString text;
                        QTextStream ts(&text);
                        ts << file.fileName() << "\n"
                           << "Your file format is older than the newest version! " << caf_version << "<" << CAF_VERSIONNUM << " Do you want to auto update to the latest format?\n\n"
                           << "NOTE! Your old files will be copied to the hopsan Backup folder, but you should make sure that you have a backup in case something goes wrong.\n"
                           << "NOTE! All non-standard Hopsan contents will be lost\n"
                           << "NOTE! Attributes may change order within a tag (but the order is not important)\n\n"
                           << "If you want to update manually, see the documantation about the latest format version.";
                        questionBox.setWindowTitle("A NEW appearance data format is available!");
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
                            mUpConvertAllCAF = YesToAll;
                        }
                        else if (pClickedButton == pNoToAll)
                        {
                            mUpConvertAllCAF = NoToAll;
                        }
                    }
                    else if (mUpConvertAllCAF==YesToAll)
                    {
                        doSave = true;
                    }

                    if (doSave)
                    {
                        //Close file
                        file.close();

                        // Make backup of original file
                        QFileInfo newBakFile(mUpdateXmlBackupDir.absolutePath() + "/" + relativePath(file, QDir(libRootDir)));
                        QDir dir;
                        dir.mkpath(newBakFile.absoluteDir().absolutePath());
                        file.copy(newBakFile.absoluteFilePath());

                        // Save (overwrite original file)
                        pAppearanceData->saveToXMLFile(file.fileName());

                    }
                }
            }
        }
        else
        {
            QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData in %4"),
                                     QString(file.fileName() + "\nParse error at line %1, column %2:\n%3")
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
                gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("When loading graphics, ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)", "componentnotregistered");
            }
        }

        if (success)
        {
            pTree->addComponent(pAppearanceData);
            mLoadedComponents << pAppearanceData->getTypeName();
            qDebug() << "Adding: " << pAppearanceData->getTypeName();
        }
    }

    if (doRecurse)
    {
        // Append subfolders recursively
        libDirObject.setFilter(QDir::AllDirs);
        QStringList subDirList = libDirObject.entryList();
        subDirList.removeAll(".");
        subDirList.removeAll("..");
        subDirList.removeAll(".svn");
        for(int i=0; i<subDirList.size(); ++i)
        {
            loadLibraryFolder(libDir+"/"+subDirList.at(i), libRootDir, doRecurse, pTree);
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




void LibraryWidget::updateLibraryFolder(LibraryContentsTree /**pTree*/)
{
//    QDir libDirObject(pTree->mLibDir);
//    //if(!libDirObject.exists() && gConfig.hasUserLib(libDir))
//    //{
//    //    gConfig.removeUserLib(libDir);      //Remove user lib if it does not exist
//        //! @todo Shouldn't we do a return here? The code bellow will probably crash if the directory does not exist.
//    //}

//    //QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));


//    //LibraryContentsTree *pTree = pParentTree->addChild(libName);        //Create the node
//    //pTree->mLibDir = libDir;
//    //libName = pTree->mName; //Reset name variable to new unique name

//        // Load DLL or SO files
//    QStringList filters;
//    #ifdef WIN32
//        filters << "*.dll";
//    #else
//        filters << "*.so";
//    #endif
//    libDirObject.setNameFilters(filters);
//    QStringList libList = libDirObject.entryList();
//    bool success=false;
//    for (int i = 0; i < libList.size(); ++i)
//    {
//        QString filename = libDir + "/" + libList.at(i);
//        if(!pTree->mLoadedLibraryDLLs.contains(filename))       //Only add new components
//        {
//            qDebug() << "Trying to load: " << filename << " in Core";
//            if(mpCoreAccess->loadComponentLib(filename))
//            {
//                success=true;
//                pTree->mLoadedLibraryDLLs.append(filename); // Remember dlls loaded in this subtree
//            }
//        }
//    }

//    if(!success && libList.size()>0)
//    {
//        gpMainWindow->mpHcomWidget->mpConsole->printInfoMessage(libDirObject.path() + ": Could not find any new library files in specified folder.");
//        gpMainWindow->mpMessageWidget->checkMessages();
//        return;     //Nothing to do since no new libraries found
//    }
//    gpMainWindow->mpMessageWidget->checkMessages();

//    // Load XML files and recursively load subfolder

//    //! @todo Subfolders should be updated as well
//    //Append subnodes recursively
////    libDirObject.setFilter(QDir::AllDirs);
////    QStringList subDirList = libDirObject.entryList();
////    subDirList.removeAll(".");
////    subDirList.removeAll("..");
////    subDirList.removeAll(".svn");
////    for(int i=0; i<subDirList.size(); ++i)
////    {
////        loadLibraryFolder(libDir+"/"+subDirList.at(i), libRootDir, pTree);
////    }

//    //Append components
//    filters.clear();
//    filters << "*.xml";                     //Create the name filter
//    libDirObject.setFilter(QDir::NoFilter);
//    libDirObject.setNameFilters(filters);   //Set the name filter

//    QStringList xmlFileList  = libDirObject.entryList();    //Create a list with all name of the files in dir libDir
//    for (int i = 0; i < xmlFileList.size(); ++i)        //Iterate over the file names
//    {
//        QString filename = libDir + "/" + xmlFileList.at(i);
//        QFile file(filename);   //Create a QFile object
//        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
//        {
//            gpMainWindow->mpHcomWidget->mpConsole->printErrorMessage("Failed to open file or not a text file: " + filename);
//            continue;
//        }

//        ModelObjectAppearance *pAppearanceData = new ModelObjectAppearance;

//        QDomDocument domDocument;        //Read appearance from file, First check if xml
//        QString errorStr;
//        int errorLine, errorColumn;
//        if (domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))      //Load dom document
//        {
//            QDomElement cafRoot = domDocument.documentElement();
//            if (cafRoot.tagName() != CAF_ROOT)                      //Not an appearance file
//            {
//                gpMainWindow->mpHcomWidget->mpConsole->printDebugMessage(file.fileName() + ": The file is not an Hopsan Component Appearance Data file. Incorrect caf root tag name: " + cafRoot.tagName() + "!=" + CAF_ROOT);
//                continue;
//            }
//            else
//            {
//                //Read appearance data from the caf xml file, begin with the first
//                QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearace objects from same file, aslo not hardcode tagnames
//                pAppearanceData->setBasePath(libDir + "/");
//                pAppearanceData->readFromDomElement(xmlModelObjectAppearance);

//                // Check CAF version, and ask user if they want to update to latest version
//                QString caf_version = cafRoot.attribute(CAF_VERSION);

//                if (caf_version < CAF_VERSIONNUM)
//                {
//                    bool doSave=false;
//                    if (mUpConvertAllCAF==UNDECIDED_TO_ALL)
//                    {
//                        QMessageBox questionBox(this);
//                        QString text;
//                        QTextStream ts(&text);
//                        ts << file.fileName() << "\n"
//                           << "Your file format is older than the newest version! " << caf_version << "<" << CAF_VERSIONNUM << " Do you want to auto update to the latest format?\n\n"
//                           << "NOTE! Your old files will be copied to the hopsan Backup folder, but you should make sure that you have a backup in case something goes wrong.\n"
//                           << "NOTE! All non-standard Hopsan contents will be lost\n"
//                           << "NOTE! Attributes may change order within a tag (but the order is not important)\n\n"
//                           << "If you want to update manually, see the documantation about the latest format version.";
//                        questionBox.setWindowTitle("A NEW appearance data format is available!");
//                        questionBox.setText(text);
//                        QPushButton* pYes = questionBox.addButton(QMessageBox::Yes);
//                        questionBox.addButton(QMessageBox::No);
//                        QPushButton* pYesToAll = questionBox.addButton(QMessageBox::YesToAll);
//                        QPushButton* pNoToAll = questionBox.addButton(QMessageBox::NoToAll);
//                        questionBox.setDefaultButton(QMessageBox::No);
//                        questionBox.exec();
//                        QAbstractButton* pClickedButton = questionBox.clickedButton();

//                        if ( (pClickedButton == pYes) || (pClickedButton == pYesToAll) )
//                        {
//                            doSave = true;
//                        }

//                        if (pClickedButton == pYesToAll)
//                        {
//                            mUpConvertAllCAF = YES_TO_ALL;
//                        }
//                        else if (pClickedButton == pNoToAll)
//                        {
//                            mUpConvertAllCAF = NO_TO_ALL;
//                        }
//                    }
//                    else if (mUpConvertAllCAF==YES_TO_ALL)
//                    {
//                        doSave = true;
//                    }

//                    if (doSave)
//                    {
//                        //Close file
//                        file.close();

//                        // Make backup of original file
//                        QFileInfo newBakFile(mUpdateXmlBackupDir.absolutePath() + "/" + relativePath(file, QDir(libRootDir)));
//                        QDir dir;
//                        dir.mkpath(newBakFile.absoluteDir().absolutePath());
//                        file.copy(newBakFile.absoluteFilePath());

//                        // Save (overwrite original file)
//                        pAppearanceData->saveToXMLFile(file.fileName());

//                    }
//                }
//            }
//        }
//        else
//        {
//            QMessageBox::information(window(), tr("Hopsan GUI read AppearanceData in %4"),
//                                     QString(file.fileName() + "Parse error at line %1, column %2:\n%3")
//                                     .arg(errorLine)
//                                     .arg(errorColumn)
//                                     .arg(errorStr)
//                                     .arg(file.fileName()));

//            //! @todo give smart warning message, this is not an xml file

//            continue;
//        }

//        //Close file
//        file.close();

//        //! @todo maybe use the convenient helpfunction for the stuff above (open file and check xml and root tagname) now that we have one

//        bool success = true;

//        //! @todo maybe we need to check appearance data for a minimuma amount of necessary data
//        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
//        {
//            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
//                //Check that component is registered in core
//            success = mpCoreAccess->hasComponent(pAppearanceData->getTypeName()); //Check so that there is such component availible in the Core
//            if (!success)
//            {
//                gpMainWindow->mpHcomWidget->mpConsole->printWarningMessage("When loading graphics, ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)", "componentnotregistered");
//            }
//        }

//        if (success)
//        {
//            if(!mLoadedComponents.contains(pAppearanceData->getTypename()))
//            {
//                pTree->addComponent(pAppearanceData);
//                mLoadedComponents << pAppearanceData->getTypeName();
//                qDebug() << "Adding: " << pAppearanceData->getTypeName();
//            }
//        }
//    }

//    //Make sure empty external libraries are not loaded (because they would become invisible and not removeable)
//    if(pTree->isEmpty())
//    {
//        pParentTree->removeChild(libName);
//        if(gConfig.hasUserLib(libDir))
//        {
//            gConfig.removeUserLib(libDir);
//        }
//        delete pTree;
//    }
}



void LibraryWidget::unloadExternalLibrary(const QString libName, const QString parentLibName)
{
    //Check both by name, absolute and relative path to be sure
    LibraryContentsTree* pLibContTree = 0;
    if(mpContentsTree->findChildByName(parentLibName))
    {
        pLibContTree = mpContentsTree->findChildByName(parentLibName)->findChildByName(libName);
        if (!pLibContTree)
        {
            pLibContTree = mpContentsTree->findChildByName(parentLibName)->findChildByPath(QDir::cleanPath(gDesktopHandler.getExecPath()+libName));
        }
        if (!pLibContTree)
        {
            pLibContTree = mpContentsTree->findChildByName(parentLibName)->findChildByPath(QDir::cleanPath(libName));
        }
    }


    if (pLibContTree)
    {
        QStringList components, nodes;
        getSubTreeComponentsAndNodes(pLibContTree, components, nodes);

        qDebug() << "Components in library: " << components;
        qDebug() << "Nodes in library: " << nodes;

        bool doWarn = false;
        for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
        {
            QStringList modelComponents = gpMainWindow->mpProjectTabs->getContainer(i)->getModelObjectNames();
            for(int c=0; c<modelComponents.size(); ++c)
            {
                QString type = gpMainWindow->mpProjectTabs->getContainer(i)->getModelObject(modelComponents[c])->getTypeName();
                if(components.contains(type))
                {
                    doWarn = true;
                }
            }
        }

        QMessageBox::StandardButton button = QMessageBox::Ok;
        if (gpMainWindow->mpProjectTabs->count() > 0 && doWarn)
        {
            button = QMessageBox::question(this, "Unload Warning!",
                                           "You have open models containing components from the library you are trying to unload. Unloading will likely result in a program crash.\n\nDo you want to continue?",
                                           QMessageBox::Ok | QMessageBox::Cancel );
        }

        if (button == QMessageBox::Ok)
        {
            gConfig.removeUserLib(pLibContTree->mLibDir);
            unLoadLibrarySubTree(pLibContTree, parentLibName);
            update();
        }
    }

}


void LibraryWidget::getSubTreeComponentsAndNodes(const LibraryContentsTree *pTree, QStringList &rComponents, QStringList &rNodes)
{
    if(pTree == 0)
        return;

    //First call unload on all dlls in core
    for (int i=0; i<pTree->mLoadedLibraryDLLs.size(); ++i)
    {
        mpCoreAccess->getLibraryContents(pTree->mLoadedLibraryDLLs[i], rComponents, rNodes);
    }

}


void LibraryWidget::unLoadLibrarySubTree(LibraryContentsTree *pTree, const QString parentLibDir)
{
    if(pTree == 0)
        return;

    //First call unload on all dlls in core
    for (int i=0; i<pTree->mLoadedLibraryDLLs.size(); ++i)
    {
        mpCoreAccess->unLoadComponentLib(pTree->mLoadedLibraryDLLs[i]);
    }
    //Then remove the tree itself
    mpContentsTree->findChildByName(parentLibDir)->removeChild(pTree->mName);
    gpMainWindow->mpTerminalWidget->checkMessages();
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
    QString path = pTree->mLibDir;
    QStringList userLibs = gConfig.getUserLibs();
    if(userLibs.contains(path) /*pItem->parent()->text(0) == "External Libraries"*/)
    {
        pUnloadLibraryFolder = menu.addAction("Unload External Library");
    }


    //-- User interaction --//
    QAction *pSelectedAction = menu.exec(mapToGlobal(event->pos()));
    //----------------------//


    if(pSelectedAction == pOpenContainingFolder)
    {
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }

    if(pSelectedAction == pUnloadLibraryFolder)
    {
        unloadExternalLibrary(pTree->mName, pTree->mpParent->mName);
    }

    QWidget::contextMenuEvent(event);
}


void LibraryWidget::mouseMoveEvent(QMouseEvent *event)
{
    mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();

    qDebug() << "You are hovering me!";

    mpTree->setFrameShape(QFrame::StyledPanel);
    mpList->setFrameShape(QFrame::StyledPanel);

    QWidget::mouseMoveEvent(event);
}


LibraryTreeWidget::LibraryTreeWidget(LibraryWidget *parent)
    : QTreeWidget(parent)
{
    //Nothing to do yet
}


void LibraryTreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if(event->button() == Qt::RightButton) return;

    QTreeWidgetItem *item = currentItem();

    if(!gpMainWindow->mpLibrary->mTreeItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString fullTypeName = gpMainWindow->mpLibrary->mTreeItemToContentsMap.find(item).value()->getFullTypeName();
    QIcon icon = gpMainWindow->mpLibrary->mTreeItemToContentsMap.find(item).value()->getIcon(gpMainWindow->mpLibrary->mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(fullTypeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);

    //mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();
}



//! @brief Reimplementation of mouse move event
//! Used to update the component name display field while hovering the icons.
//! @param event Contains information about the event
void LibraryTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    this->setFrameShape(QFrame::Box);

    QTreeWidget::mouseMoveEvent(event);
}


void LibraryTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;

    QAction *pEditComponentAction=0;
    if(gpMainWindow->mpLibrary->mTreeItemToContentsMap.contains(this->currentItem()))
    {
        pEditComponentAction = menu.addAction("Edit source code");
        ModelObjectAppearance *pAppearanceData = gpMainWindow->mpLibrary->mTreeItemToContentsMap.find(this->currentItem()).value()->getAppearanceData();
        pEditComponentAction->setEnabled(pAppearanceData->isRecompilable());
    }

    //-- User interaction --//
    QAction *pSelectedAction = menu.exec(mapToGlobal(event->pos()));
    //----------------------//

    if(pSelectedAction == pEditComponentAction)
    {
        gpMainWindow->mpLibrary->editComponent(this->currentItem(),0);
    }

    delete(pEditComponentAction);
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


void LibraryListWidget::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    if(event->button() == Qt::RightButton) return;

    QListWidgetItem *item = currentItem();

    if(!gpMainWindow->mpLibrary->mListItemToContentsMap.contains(item)) return;      //Do nothing if item does not exist in map (= not a component)

    //Fetch type name and icon from component in the contents tree
    QString fullTypeName = gpMainWindow->mpLibrary->mListItemToContentsMap.find(item).value()->getFullTypeName();
    QIcon icon = gpMainWindow->mpLibrary->mListItemToContentsMap.find(item).value()->getIcon(gpMainWindow->mpLibrary->mGfxType);

    //Create the mimedata (text with type name)
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(fullTypeName);

    //Initiate the drag operation
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(icon.pixmap(40,40));
    drag->setHotSpot(QPoint(20, 20));
    drag->exec(Qt::CopyAction | Qt::MoveAction);

    //mpComponentNameField->setText(QString());
    gpMainWindow->hideHelpPopupMessage();
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

    mpLibraryWidget->mpList->setFrameShape(QFrame::Box);

    QListWidget::mouseMoveEvent(event);
}


void LibraryListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;

    QAction *pEditComponentAction = menu.addAction("Edit source code");
    ModelObjectAppearance *pAppearanceData = mpLibraryWidget->mListItemToContentsMap.find(this->currentItem()).value()->getAppearanceData();
    pEditComponentAction->setEnabled(pAppearanceData->isRecompilable());

    //-- User interaction --//
    QAction *pSelectedAction = menu.exec(mapToGlobal(event->pos()));
    //----------------------//

    if(pSelectedAction == pEditComponentAction)
    {
        mpLibraryWidget->editComponent(this->currentItem());
    }

    delete(pEditComponentAction);
}


//! @brief Retrieves the appearance data for a given type name.
//! @param compType The typename of the component
//! @param compSubType SubType of the component (if any)
ModelObjectAppearance *LibraryWidget::getAppearanceData(const QString compType, const QString compSubType)
{
    LibraryComponent* pLibComp = mpContentsTree->findComponent(compType, compSubType);
    if(pLibComp == 0)
    {
        // If we cant find in ordinary tree, then try the secret hidden tree
        pLibComp = mpSecretHiddenContentsTree->findComponent(compType, compSubType);
        if (pLibComp == 0)
        {
            // Nothing found return NULL ptr
            return 0;
        }
    }
    // If we found something then return appearance data
    return pLibComp->getAppearanceData();
}

//! @brief Retrieves the appearance data for a given full type name (type|subtype).
ModelObjectAppearance *LibraryWidget::getAppearanceData(const QString fullCompType)
{
    QString type, subtype;
    splitFullTypeString(fullCompType, type, subtype);
    return getAppearanceData(type, subtype);
}


//! @brief Selects graphics type to be used in library (iso or user).
void LibraryWidget::setGfxType(GraphicsTypeEnumT gfxType)
{
    mGfxType = gfxType;
    update();       //Redraw the library
}


//! @brief Constructor for a library contents tree node.
//! @param name Name of the node (empty for top node, which is never displayed)
LibraryContentsTree::LibraryContentsTree(QString name, LibraryContentsTree *pParent)
{
    mName = name;
    mpParent = pParent;
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
    if(findChildByName(name))
    {
        QString newName = name.append("_0");
        int i=1;
        while(findChildByName(newName))
        {
            QString num;
            num.setNum(i);
            newName.chop(1);
            newName.append(num);
            ++i;
        }
    }
    LibraryContentsTree *pNewChild = new LibraryContentsTree(name, this);
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
            for(int j=0; j<mChildNodesPtrs[i]->mComponentPtrs.size(); ++j)
            {
                delete(mChildNodesPtrs[i]->mComponentPtrs[j]);
            }
            mChildNodesPtrs.remove(i);
            return true;
        }
    }
    return false;       //Not found
}

//! @brief Returns a pointer to sub library with specified name, or zero if it does not exist.
//! @param name Name to look for
LibraryContentsTree *LibraryContentsTree::findChildByName(QString name)
{
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        if(mChildNodesPtrs.at(i)->mName == name)
            return mChildNodesPtrs.at(i);
    }
    return 0;
}


//! @brief Returns a pointer to sub library with specified path, or zero if it does not exist.
//! @param name Name to look for
LibraryContentsTree *LibraryContentsTree::findChildByPath(QString path)
{
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        qDebug() << "Comparing: " << mChildNodesPtrs.at(i)->mLibDir << " with " << path;
        if(mChildNodesPtrs.at(i)->mLibDir == path)
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
LibraryComponent *LibraryContentsTree::findComponent(const QString type, const QString subType)
{
    const QString fullType = makeFullTypeString(type, subType);
    for(int i=0; i<mComponentPtrs.size(); ++i)
    {
        if(mComponentPtrs.at(i)->getFullTypeName() == fullType)
            return mComponentPtrs.at(i);
    }
    for(int i=0; i<mChildNodesPtrs.size(); ++i)
    {
        LibraryComponent *retval = mChildNodesPtrs.at(i)->findComponent(type, subType);
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
    QString iconPath = mpAppearanceData->getFullAvailableIconPath(UserGraphics);
    QFile iconFile(iconPath);
    if (!iconFile.exists())     //Check if specified file exist, else use unknown icon
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
    mUserIcon.addFile(iconPath,QSize(55,55));
    iconFile.close();
    iconPath = mpAppearanceData->getFullAvailableIconPath(ISOGraphics);
    iconFile.setFileName(iconPath);
    if (!iconFile.exists())     //Check if specified file exist, else use unknown icon
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }
    mIsoIcon.addFile(iconPath,QSize(55,55));
    iconFile.close();
}


//! @brief Returns the component's icon.
//! @param graphicsType Graphics type to use (iso or user)
QIcon LibraryComponent::getIcon(GraphicsTypeEnumT gfxType)
{
    if(gfxType == UserGraphics)
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
QString LibraryComponent::getFullTypeName()
{
    return makeFullTypeString(mpAppearanceData->getTypeName(), mpAppearanceData->getSubTypeName());
}


//! @brief Returns a pointer to the appearance data object used by component.
ModelObjectAppearance *LibraryComponent::getAppearanceData()
{
    return mpAppearanceData;
}
