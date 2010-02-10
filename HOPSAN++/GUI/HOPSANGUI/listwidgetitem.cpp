#include "listwidgetitem.h"

ListWidgetItem::ListWidgetItem(const QIcon &icon, const QString &text, QListWidget *parent)
        : QListWidgetItem(icon, text, parent)
{
}

ListWidgetItem::ListWidgetItem(const QListWidgetItem &other)
        : QListWidgetItem(other)
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
