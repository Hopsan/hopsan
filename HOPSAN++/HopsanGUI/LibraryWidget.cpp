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
#include "listwidget.h"
#include <map>


//! Constructor.
//! @param parent defines a parent to the new instanced object.
LibraryContent::LibraryContent(QWidget *parent)
    :   QListWidget(parent)
{
    setViewMode(QListView::IconMode);
    setAcceptDrops(false);
}

/*QMimeData *LibraryContent::mimeData(const QList<QListWidgetItem*> items) const
{
    QString mess = items.first()->text();

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(mess);
    return mimeData;
}*/

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
LibraryWidget::LibraryWidget(QWidget *parent)
        :   QWidget(parent)
{
    mpTree = new QTreeWidget(this);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);

    mpGrid = new QVBoxLayout(this);

    mpGrid->addWidget(mpTree);

    setLayout(mpGrid);

    connect(mpTree, SIGNAL(itemClicked (QTreeWidgetItem*, int)), SLOT(showLib(QTreeWidgetItem*, int)));

}


//void LibraryWidget::addLibrary(QString libraryName)
//{
//    QTreeWidgetItem *newTreePost = new QTreeWidgetItem((QTreeWidget*)0);
//    newTreePost->setText(0, QString(libraryName));
//    tree->insertTopLevelItem(0, newTreePost);
//
//    LibraryContent *newLibContent = new LibraryContent((LibraryContent*)0);
//    newLibContent->setDragEnabled(true);
//    //newLibContent->setDropIndicatorShown(true);
//    libraryMap.insert(libraryName, newLibContent);
//
//    grid->addWidget(newLibContent);
//    newLibContent->hide();
//
//}

//! Adds a library to the library widget.
//! @param libraryName is the name of the new library.
//! @param parentLibraryName is the name of an eventually parent library.
//! @see addComponent(QString libraryName, ListWidgetItem *newComponent)
void LibraryWidget::addLibrary(QString libraryName, QString parentLibraryName)
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


//void LibraryWidget::addComponent(QString libraryName, QString componentName, QIcon icon, QStringList list)
//{
//    ListWidgetItem *newComponent = new ListWidgetItem(icon, componentName);
//    newComponent->setParameterData(list);
//    addComponent(libraryName, newComponent);
//
//}


//! Adds a library to the library widget.
//! @param libraryName is the name of the library where the component should be added.
//! @see addComponent(QString libraryName, QString parentLibraryName)
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
