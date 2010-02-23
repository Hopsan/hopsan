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


void ListWidgetItem::setParameterData(QStringList list)
{
    mParameterData = list;
}

QStringList ListWidgetItem::getParameterData()
{
    return mParameterData;
}


