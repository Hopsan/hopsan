//!
//! @file   LibraryWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Library Widgets
//!
//$Id$

#include "LibraryWidget.h"
#include <iostream>

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



LibraryWidget::LibraryWidget(QWidget *parent)
        :   QWidget(parent)
{
    tree = new QTreeWidget(this);
    tree->setHeaderHidden(true);
    tree->setColumnCount(1);

    grid = new QVBoxLayout(this);

    grid->addWidget(tree);

    setLayout(grid);

    connect(tree, SIGNAL(itemClicked (QTreeWidgetItem*, int)), SLOT(showLib(QTreeWidgetItem*, int)));

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


void LibraryWidget::addLibrary(QString libraryName, QString parentLibraryName)
{
    QTreeWidgetItem *newTreePost = new QTreeWidgetItem((QTreeWidget*)0);
    newTreePost->setText(0, QString(libraryName));

    LibraryContent *newLibContent = new LibraryContent((LibraryContent*)0);
    newLibContent->setDragEnabled(true);
    //newLibContent->setDropIndicatorShown(true);
    libraryMap.insert(libraryName, newLibContent);

    grid->addWidget(newLibContent);
    newLibContent->hide();

    if (parentLibraryName.isEmpty())
    {
        tree->insertTopLevelItem(0, newTreePost);
    }
    else
    {
    QTreeWidgetItemIterator it(tree);
    while (*it)
    {
        if ((*it)->text(0) == parentLibraryName)
        {
            (*it)->addChild(newTreePost);
            tree->expandItem(*it);
        }

        ++it;
    }
    }
}


void LibraryWidget::addComponent(QString libraryName, QString componentName, QIcon icon, QStringList list)
{
    ListWidgetItem *newComponent = new ListWidgetItem(icon, componentName);
    newComponent->setParameterData(list);
    addComponent(libraryName, newComponent);

}


void LibraryWidget::addComponent(QString libraryName, ListWidgetItem *newComponent)
{
    libraryMap.value(libraryName)->addItem(newComponent);

    QTreeWidgetItemIterator it(tree);
    while (*it)
    {
        if (((*it)->text(0) == libraryName) && ((*it)->parent()))
        {
            ListWidgetItem *copyOfNewComponent = new ListWidgetItem(*newComponent); //A QListWidgetItem can only be in one list at the time, therefor a copy...
            QString parentName = (*it)->parent()->text(0);

            addComponent(parentName, copyOfNewComponent); //Recursively
        }

        ++it;
    }

}


void LibraryWidget::showLib(QTreeWidgetItem * item, int column)
{
    hideAllLib();

   QMap<QString, QListWidget *>::iterator lib;
   for (lib = libraryMap.begin(); lib != libraryMap.end(); ++lib)
    {
        if (item->text(column) == libraryMap.key((*lib)))
        {
            (*lib)->show();
        }

    }

}


void LibraryWidget::hideAllLib()
{
    QMap<QString, QListWidget *>::iterator lib;
    for (lib = libraryMap.begin(); lib != libraryMap.end(); ++lib)
    {
        (*lib)->hide();
    }

}
