#include "treewidgetitem.h"

TreeWidgetItem::TreeWidgetItem(QTreeWidget *parent, ListWidget *list)
        : QTreeWidgetItem(parent)
{
    mlist = list;
}

TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem *parent, ListWidget *list)
        : QTreeWidgetItem(parent)
{
    mlist = list;
}

TreeWidgetItem::~TreeWidgetItem()
{
}

ListWidget* TreeWidgetItem::getList()
{
    return mlist;
}
