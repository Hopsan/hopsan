#include "LibraryWidget.h"
#include <iostream>

LibraryContent::LibraryContent(QWidget *parent)
    :   QListWidget(parent)
{
    setViewMode(QListView::IconMode);
}

QMimeData *LibraryContent::mimeData(const QList<QListWidgetItem*> items) const
{
    QString mess = items.first()->text();

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(mess);
    return mimeData;
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


void LibraryWidget::addComponent(QString libraryName, QString componentName, QIcon icon)
{
    QListWidgetItem *newComponent = new QListWidgetItem(icon, componentName);
    addComponent(libraryName, newComponent);

}


void LibraryWidget::addComponent(QString libraryName, QListWidgetItem *newComponent)
{
    libraryMap.value(libraryName)->addItem(newComponent);

    QTreeWidgetItemIterator it(tree);
    while (*it)
    {
        if (((*it)->text(0) == libraryName) && ((*it)->parent()))
        {
            QListWidgetItem *copyOfNewComponent = new QListWidgetItem(*newComponent); //A QListWidgetItem can only be in one list at the time, therefor a copy...
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
