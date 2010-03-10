//$Id$
#include "listwidgetitem.h"
#include <QStringList>
#include <QtGui>

ListWidgetItem::ListWidgetItem(const QIcon &icon, const QString &text, QListWidget *parent)
        : QListWidgetItem(icon, text, parent)
{
    QFont font;
    font.setPixelSize(8);
    this->setFont(font);
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


