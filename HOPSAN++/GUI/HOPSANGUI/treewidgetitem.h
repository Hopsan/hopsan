#ifndef TREEWIDGETITEM_H
#define TREEWIDGETITEM_H

#include <QTreeWidgetItem>
#include <QString>

#include "listwidget.h"

class TreeWidgetItem : public QTreeWidgetItem
{
public:
    TreeWidgetItem(QTreeWidget *parent = 0, ListWidget *list = 0);
    TreeWidgetItem(QTreeWidgetItem *parent = 0, ListWidget *list = 0);
    ~TreeWidgetItem();

    ListWidget *mlist;

};

#endif // TREEWIDGETITEM_H
