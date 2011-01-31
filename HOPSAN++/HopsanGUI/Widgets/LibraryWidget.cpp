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

//! Constructor
LibraryContentItem::LibraryContentItem(GUIModelObjectAppearance *pAppearanceData, QListWidget *pParent)
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
GUIModelObjectAppearance *LibraryContentItem::getAppearanceData()
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
    //QPixmap testPixMap;

    QString iconPath = mpAppearanceData->getFullIconPath(gfxType);
    //Check if specified file exist, else use unknown icon
    QFile iconFile(iconPath);
    if (!iconFile.exists())
    {
        iconPath = QString(OBJECTICONPATH) + QString("missingcomponenticon.svg");
    }

    icon.addFile(iconPath,QSize(55,55));

    //this->setSizeHint(QSize(55,55));
    this->setIcon(icon);

    //this->setData(Qt::UserRole, QVariant(icon));
}


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryContent::LibraryContent(LibraryContent *pParentLibraryContent, QString mapKey, LibraryWidget *pParentLibraryWidget, QTreeWidgetItem *pParentTreeWidgetItem)
    :   QListWidget(pParentLibraryContent)
{
    mpParentLibraryWidget = pParentLibraryWidget;
    mpParentTreeWidgetItem = pParentTreeWidgetItem;
    mMapKey = mapKey;
    mpHoveredItem = 0x0;
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
        //Make hovered item light blue & display its name
    mpParentLibraryWidget->mpComponentNameField->setText("");
    QListWidgetItem *tempItem = itemAt(event->pos());
    if(tempItem != 0x0)     //The pointer is zero if there is no item beneath the mouse
    {
        if(tempItem != mpHoveredItem)
        {
            tempItem->setBackgroundColor(QColor("lightblue"));
            if(mpHoveredItem != 0x0)
            {
                mpHoveredItem->setBackgroundColor(QColor("white"));
            }
            mpHoveredItem = tempItem;
        }

        //Change name in component name field. Resize the text if needed, so that the library widget does not change size.
        mpParentLibraryWidget->mpComponentNameField->setFont(QFont(mpParentLibraryWidget->mpComponentNameField->font().family(), min(12.0, mpParentLibraryWidget->width()/(0.615*tempItem->toolTip().size()))));
        mpParentLibraryWidget->mpComponentNameField->setText(tempItem->toolTip());
    }
    else
    {
        if(mpHoveredItem != 0x0)
        {
            mpHoveredItem->setBackgroundColor(QColor("white"));
        }
        mpHoveredItem = 0x0;
    }

        //Return if no drag is initialized
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
    //mpParentMainWindow = parent;

    mpTree = new LibraryTreeWidget(this);
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

    this->setGfxType(USERGRAPHICS);

    connect(mpTree, SIGNAL(itemClicked (QTreeWidgetItem*, int)), SLOT(showLib(QTreeWidgetItem*, int)));
}


//! Adds an empty library to the library widget.
//! @param libraryName is the name of the new library.
//! @param parentLibraryName is the name of an eventually parent library.
//! @see addLibrary(QString libDir, QString parentLib)
//! @see addLibrary()
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList appearanceData)
void LibraryWidget::addEmptyLibrary(QString libraryName, QString parentLibraryName, QString libraryPath)
{
    QTreeWidgetItem *newTreePost = new QTreeWidgetItem((QTreeWidget*)0);
    newTreePost->setText(0, QString(libraryName));
    newTreePost->setToolTip(0,libraryPath);

    LibraryContent *newLibContent = new LibraryContent((LibraryContent*)0, parentLibraryName + libraryName, this, newTreePost);
    newLibContent->setDragEnabled(true);
    newLibContent->mIsUserLib = (parentLibraryName == "User defined libraries");

    //newLibContent->setDropIndicatorShown(true);
    mLibraryContentPtrsMap.insert(parentLibraryName + libraryName, newLibContent);

    mpGrid->addWidget(newLibContent);
    newLibContent->hide();

    if (parentLibraryName.isEmpty())
    {
        QFont tempFont = newTreePost->font(0);
        tempFont.setBold(true);
        newTreePost->setFont(0, tempFont);
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


//! @brief Adds a library to the library widget.
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

    //Create a QDir object that contains the info about the library direction
    QDir libDirObject(libDir);

    //Get the name for the library to be set in the tree
    QString libName = QString(libDirObject.dirName().left(1).toUpper() + libDirObject.dirName().right(libDirObject.dirName().size()-1));

    //Add the library to the tree
    addEmptyLibrary(libName,parentLib,libDir);

    //Create a QStringList object that contains name filters
    QStringList filters;
    filters << "*.xml";                     //Create the name filter
    libDirObject.setNameFilters(filters);   //Set the name filter

    //Create a list with all name of the files in dir libDir
    QStringList libList = libDirObject.entryList();
    for (int i = 0; i < libList.size(); ++i)        //Iterate over the file names
    {
        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            qDebug() << "Failed to open file or not a text file: " + filename;
            return;
        }


        GUIModelObjectAppearance *pAppearanceData = new GUIModelObjectAppearance;

        //Read appearance from file, First check if xml
        QDomDocument domDocument;
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

        bool sucess = true;
        pAppearanceData->setBaseIconPath(libDirObject.absolutePath() + "/");

        //! @todo maybe we need to check appearance data for a minimuma amount of necessary data
        //*****Core Interaction*****
        HopsanEssentials *pHopsanCore = HopsanEssentials::getInstance();
        if(!((pAppearanceData->getTypeName()==HOPSANGUISYSTEMTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUIGROUPTYPENAME) || (pAppearanceData->getTypeName()==HOPSANGUICONTAINERPORTTYPENAME)) ) //Do not check if it is Subsystem or SystemPort
        {
            //! @todo this check (hasComponent) should be wrapped inside some coreaccess class
            //! @todo maybe systemport should be in the core component factory (HopsanCore related), not like that right now
            sucess = pHopsanCore->hasComponent(pAppearanceData->getTypeName().toStdString()); //Check so that there is such component availible in the Core
            if (!sucess)
            {
                gpMainWindow->mpMessageWidget->printGUIWarningMessage("ComponentType: " + pAppearanceData->getTypeName() + " is not registered in core, (Will not be availiable)");
            }
        }
        //**************************

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
    QDir fileDialogOpenDir; //This dir object is used for setting the open directory of the QFileDialog, i.e. apps working dir

    QString libDir = QFileDialog::getExistingDirectory(this, tr("Choose Library Directory"),
                                                 fileDialogOpenDir.currentPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(!gConfig.hasUserLib(libDir))
    {
        gConfig.addUserLib(libDir);
        addExternalLibrary(libDir);
    }
    else
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Library " + libDir + " is already loaded!");
    }
    //std::cout << qPrintable(libDir) << std::endl;
}


//! Load a external library and adds it to the 'User defined libraries'.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary(QString libDir, QString parentLib)
void LibraryWidget::addExternalLibrary(QString libDir)
{
    qDebug() << "looking for dll or so in: " << libDir;
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
        qDebug() << "Trying to load: " << filename << " in Core";
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
    mName2TypeMap.insert(newComponent->getAppearanceData()->getNonEmptyName(), newComponent->getAppearanceData()->getTypeName()); //! @todo this is a temporary workaround, what happens if two components have sae displayname
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
GUIModelObjectAppearance *LibraryWidget::getAppearanceData(QString componentType)
{
    //qDebug() << "LibraryWidget::getAppearanceData: " + componentType;
    if (mLibraryContentItemPtrsMap.count(componentType) == 0)
    {
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("Trying to fetch appearanceData for " + componentType + " which does not appear to exist in the Map, returning empty data");
        return 0;
    }
    return mLibraryContentItemPtrsMap.value(componentType)->getAppearanceData();
}

//! @brief This function retrieves the appearance data given a display name
//! @todo This is a temporary hack
GUIModelObjectAppearance *LibraryWidget::getAppearanceDataByDisplayName(QString displayName)
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
    mGfxType = gfxType;
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





LibraryTreeWidget::LibraryTreeWidget(LibraryWidget *parent)
        : QTreeWidget(parent)
{
    mpParentLibraryWidget = parent;
}



void LibraryTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidget::contextMenuEvent(event);


    QMenu menu;

    QAction *loadAction;
    QAction *unloadAction;
    loadAction = menu.addAction(QString("Load External Library"));


        //! @todo This is an ugly check to make sure the right clicked object is a library with contents
    if( (this->currentItem() != 0) && (this->currentItem()->parent() != 0) )
    {
        // This will check if the library is a user library (which can be removed)
        if(mpParentLibraryWidget->mLibraryContentPtrsMap.find(QString(this->currentItem()->parent()->text(0) + this->currentItem()->text(0))).value()->mIsUserLib)
        {
            unloadAction = menu.addAction(QString("Unload Library \"" + this->currentItem()->text(0) + "\""));
        }
    }


    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    if ((selectedAction == unloadAction) && (unloadAction != 0))
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::information(this, tr("Information"), tr("Program must be restarted for this to take effect."));
        qDebug() << "Trying to remove " << this->currentItem()->text(0);
//        for(size_t i=0; i<mpParentLibraryWidget->mpParentMainWindow->mUserLibs.size(); ++i)
//        {
//            if(mpParentLibraryWidget->mpParentMainWindow->mUserLibs.at(i).endsWith("/"+this->currentItem()->text(0)))
//            {
//                qDebug() << "Removing at " << i;
//                mpParentLibraryWidget->mpParentMainWindow->mUserLibs.removeAt(i);
//                --i;
//            }
//        }
        gConfig.removeUserLib(this->currentItem()->toolTip(0));
    }
    else if (selectedAction == loadAction)
    {
        this->mpParentLibraryWidget->addLibrary();
    }
}
