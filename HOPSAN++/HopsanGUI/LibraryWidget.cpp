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

////! @brief Constructor
//LibraryContentItem::LibraryContentItem(const QIcon &icon, const QString &text, QListWidget *parent)
//        : QListWidgetItem(icon, text, parent)
//{
//    QFont font;
//    font.setPixelSize(8);
//    this->setFont(font);
//}

LibraryContentItem::LibraryContentItem(AppearanceData *pAppearanceData, QListWidget *pParent)
        : QListWidgetItem(pAppearanceData->getTypeName(), pParent)
{
    //Set font
    QFont font;
    font.setPixelSize(8);
    this->setFont(font);

    //Set Icon, prefere user, if its empty use iso
    QIcon icon;
    if ( pAppearanceData->getIconPath().isEmpty() )
    {
        icon.addFile(pAppearanceData->getBasePath() + pAppearanceData->getIconPathISO());
    }
    else
    {
        icon.addFile(pAppearanceData->getBasePath() + pAppearanceData->getIconPath());
    }
    setIcon(icon);

    mpAppearanceData = pAppearanceData;
}

//! @brief Copy Constructor
LibraryContentItem::LibraryContentItem(const QListWidgetItem &other)
        : QListWidgetItem(other)
{
}

AppearanceData *LibraryContentItem::getAppearanceData()
{
    return mpAppearanceData;
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

    QByteArray *data = new QByteArray;
    QString datastr;
    QTextStream stream(&datastr);//, QIODevice::WriteOnly);

    QListWidgetItem *pItem = this->currentItem();

    //stream out appearance data and extra basepath info
    stream << *(mpParentLibraryWidget->getAppearanceData(pItem->text()));
    stream << "BASEPATH " << mpParentLibraryWidget->getAppearanceData(pItem->text())->getBasePath();
    //qDebug() << "moving: appearanceData: " << *(mpParentLibraryWidget->getAppearanceData2(pItem->text()));

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    //QString mimeType = "application/x-text";

    //mimeData->setData(mimeType, *data);
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
    mLibraryMapPtrs.insert(parentLibraryName + libraryName, newLibContent);

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
        //Set up needed variables
        QStringList appearanceData;
        QString componentName;
        QIcon icon;
        QString isoIconPath;
        QString userIconPath;
        QString iconRotationBehaviour;
        QString nPorts;
        QString portPosX;
        QString portPosY;
        QString portRot;

        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
        {
            qDebug() << "Failed to open file or not a text file: " + filename;
            return;
        }


        //userIconPath = QString("");
        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file

//        while (!inFile.atEnd()) {
//            QString line = inFile.readLine();   //line contains each row in the file
//
//            if (line.startsWith("NAME"))
//            {
//                componentName = line.mid(5);
//                appearanceData << componentName;
//            }
//
//            if (line.startsWith("ISOICON"))
//            {
//                isoIconPath = libDirObject.absolutePath() + "/" + line.mid(8);
//                icon.addFile(isoIconPath);
//                appearanceData << isoIconPath;
//            }
//
//            if (line.startsWith("USERICON"))
//            {
//                userIconPath = libDirObject.absolutePath() + "/" + line.mid(9);
//            }
//
//            if (line.startsWith("ICONROTATION"))
//            {
//                appearanceData << userIconPath;     //This is stupid, but it must somehow be executed after USERICON even in case there is not USERICON
//                iconRotationBehaviour = line.mid(13);
//                appearanceData << iconRotationBehaviour;
//            }
//
//            if (line.startsWith("PORTS"))
//            {
//                nPorts = line.mid(6);
//                appearanceData << nPorts;
//                for (int i = 0; i < nPorts.toInt(); ++i)
//                {
//                    inFile >> portPosX;
//                    inFile >> portPosY;
//                    inFile >> portRot;
////                    line = inFile.readLine();
////                    portPosX = line.mid(0);
////                    line = inFile.readLine();
////                    portPosY = line.mid(0);
////                    line = inFile.readLine();
////                    portRot = line.mid(0);
//                    std::cout << qPrintable(componentName) << " x: " << qPrintable(portPosX) << " y: " << qPrintable(portPosY) << " rot: " << qPrintable(portRot) << std::endl;
//                    appearanceData << portPosX << portPosY << portRot;
//                }
//            }
//        }

        //Add data to the paremeterData list
  //      appearanceData << componentName << iconPath;

        AppearanceData *pAppearanceData = new AppearanceData;
        inFile >> *pAppearanceData;
        pAppearanceData->setBasePath(libDirObject.absolutePath() + "/");
        LibraryContentItem *libcomp= new LibraryContentItem(pAppearanceData);

        file.close();
        //LibraryContentItem *libcomp= new LibraryContentItem(icon, componentName);
      //  std::cout << appearanceData.size() << std::endl;
        //libcomp->setAppearanceData(appearanceData);

        //Add the component to the library
        //library->addComponent(libName,componentName,icon,appearanceData);
        addLibraryContentItem(libName, parentLib, libcomp);
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
    mLibraryMapPtrs.value(parentLibraryName + libraryName)->addItem(newComponent);
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
    qDebug() << "Mapping parameters for component: " << newComponent->getAppearanceData()->getTypeName();
}


//! Makes a library visible.
//! @param item is the library to show.
//! @param column is the position of the library name in the tree.
//! @see hideAllLib()
void LibraryWidget::showLib(QTreeWidgetItem *item, int column)
{
    hideAllLib();

   QMap<QString, QListWidget *>::iterator lib;
   for (lib = mLibraryMapPtrs.begin(); lib != mLibraryMapPtrs.end(); ++lib)
    {
        //Not top level list widget, so check if it has the correct parent
        if(item->text(column).size() != mLibraryMapPtrs.key((*lib)).size())
        {
            if (item->text(column) == mLibraryMapPtrs.key((*lib)).right(item->text(column).size()) &&
                item->parent()->text(column) == mLibraryMapPtrs.key((*lib)).left(item->parent()->text(column).size()))
            {
                (*lib)->show();
            }
        }
        else
        //Top level widget, don't check parent (would lead to a segmentation fault since it does not exist)
        {
            if (item->text(column) == mLibraryMapPtrs.key((*lib)).right(item->text(column).size()))
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
    QMap<QString, QListWidget *>::iterator lib;
    for (lib = mLibraryMapPtrs.begin(); lib != mLibraryMapPtrs.end(); ++lib)
    {
        (*lib)->hide();
    }

}
