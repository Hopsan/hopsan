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


//! @class LibraryWidget
//! @brief The LibraryWidget class is a class which store and display component libraries.
//!
//! This class is a widget whisch can be included in the main window.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryContent::LibraryContent(QWidget *parent)
    :   QListWidget(parent)
{
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

    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QByteArray *data = new QByteArray;
    QDataStream stream(data,QIODevice::WriteOnly);

    QListWidgetItem *item = this->currentItem();

    //stream << item->data(Qt::UserRole).toString();
    stream << ((ListWidgetItem*)item)->getParameterData();

    QString mimeType = "application/x-text";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, *data);
    drag->setMimeData(mimeData);

    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()));

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);

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
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList parameterData)
void LibraryWidget::addEmptyLibrary(QString libraryName, QString parentLibraryName)
{
    QTreeWidgetItem *newTreePost = new QTreeWidgetItem((QTreeWidget*)0);
    newTreePost->setText(0, QString(libraryName));

    LibraryContent *newLibContent = new LibraryContent((LibraryContent*)0);
    newLibContent->setDragEnabled(true);
    //newLibContent->setDropIndicatorShown(true);
    mLibraryMapPtrs.insert(libraryName, newLibContent);

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
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList parameterData)
void LibraryWidget::addLibrary(QString libDir, QString parentLib)
{
    //If no directory is set, i.e. cancel is presses, do no more
    if (libDir.isEmpty() == true)
        return;

    QDir libDirObject(libDir);  //Create a QDir object that contains the info about the library direction

    //Get the name for the library to be set in the tree
    QString libName = libDirObject.dirName();

    //Add the library to the tree
    addEmptyLibrary(libName,parentLib);

    QStringList filters;        //Create a QStringList object that contains name filters
    filters << "*.txt";         //Create the name filter
    libDirObject.setNameFilters(filters);       //Set the name filter

    QStringList libList = libDirObject.entryList(); //Create a list with all name of the files in dir libDir
    for (int i = 0; i < libList.size(); ++i)    //Iterate over the file names
    {
        //Set up needed variables
        QStringList parameterData;
        QString componentName;
        QIcon icon;
        QString iconPath;
        QString iconRotationBehaviour;
        QString nPorts;
        QString portPosX;
        QString portPosY;
        QString portRot;

        QString filename = libDirObject.absolutePath() + "/" + libList.at(i);
        QFile file(filename);   //Create a QFile object
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open each file
            return;

        QTextStream inFile(&file);  //Create a QTextStream object to stream the content of each file
        while (!inFile.atEnd()) {
            QString line = inFile.readLine();   //line contains each row in the file

            if (line.startsWith("NAME"))
            {
                componentName = line.mid(5);
                parameterData << componentName;
            }

            if (line.startsWith("ICONROTATION"))
            {
                iconRotationBehaviour = line.mid(13);
                parameterData << iconRotationBehaviour;
            }
            else if (line.startsWith("ICON"))
            {
                iconPath = libDirObject.absolutePath() + "/" + line.mid(5);
                icon.addFile(iconPath);
                parameterData << iconPath;
            }

            if (line.startsWith("PORTS"))
            {
                nPorts = line.mid(6);
                parameterData << nPorts;
                for (int i = 0; i < nPorts.toInt(); ++i)
                {
                    line = inFile.readLine();
                    portPosX = line.mid(0);
                    line = inFile.readLine();
                    portPosY = line.mid(0);
                    line = inFile.readLine();
                    portRot = line.mid(0);
                    std::cout << qPrintable(componentName) << " x: " << qPrintable(portPosX) << " y: " << qPrintable(portPosY) << " rot: " << qPrintable(portRot) << std::endl;
                    parameterData << portPosX << portPosY << portRot;
                }
            }
        }
        file.close();
        //Add data to the paremeterData list
  //      parameterData << componentName << iconPath;

        ListWidgetItem *libcomp= new ListWidgetItem(icon,componentName);
      //  std::cout << parameterData.size() << std::endl;
        libcomp->setParameterData(parameterData);

        //Add the component to the library
        //library->addComponent(libName,componentName,icon,parameterData);
        addComponent(libName, libcomp, parameterData);
    }
}


//! Let the user to point out a library and adds it to the library widget.
//! @see addEmptyLibrary(QString libraryName, QString parentLibraryName)
//! @see addLibrary(QString libDir, QString parentLib)
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList parameterData)
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


//! Adds a library to the library widget.
//! @param libraryName is the name of the library where the component should be added.
void LibraryWidget::addComponent(QString libraryName, ListWidgetItem *newComponent, QStringList parameterData)
{
    mLibraryMapPtrs.value(libraryName)->addItem(newComponent);
    QTreeWidgetItemIterator it(mpTree);
    while (*it)
    {
        if (((*it)->text(0) == libraryName) && ((*it)->parent()))
        {
            ListWidgetItem *copyOfNewComponent = new ListWidgetItem(*newComponent); //A QListWidgetItem can only be in one list at the time, therefor a copy...
            QString parentName = (*it)->parent()->text(0);
            addComponent(parentName, copyOfNewComponent, parameterData); //Recursively
        }
        ++it;
    }
    mParameterMap.insert(std::pair<QString, QStringList>(parameterData.at(0), parameterData));
    qDebug() << "Mapping parameters for component: " << parameterData.at(0);
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
        if (item->text(column) == mLibraryMapPtrs.key((*lib)))
        {
            (*lib)->show();
        }

    }

}


QStringList LibraryWidget::getParameterData(QString componentType)
{
    return mParameterMap.find(componentType)->second;
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
