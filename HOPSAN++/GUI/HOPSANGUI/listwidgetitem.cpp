#include "listwidgetitem.h"

ListWidgetItem::ListWidgetItem(QListWidget *parent,int type)
        : QListWidgetItem(parent,type)
{
}

ListWidgetItem::~ListWidgetItem()
{
}


void ListWidgetItem::setIconPath(QString iconPath)
{
    miconPath = iconPath;
}

QString ListWidgetItem::getIconPath()
{
    return miconPath;
}

void ListWidgetItem::setNumberOfPorts(QString nrPorts)
{
    mnrPorts = nrPorts;
}

QString ListWidgetItem::getNumberOfPorts()
{
    return mnrPorts;
}
