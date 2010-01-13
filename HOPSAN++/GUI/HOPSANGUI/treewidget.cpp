#include "treewidget.h"
#include <iostream>

TreeWidget::TreeWidget(QWidget *parent)
        : QTreeWidget(parent)
{
    this->setDragEnabled(true);
}

TreeWidget::~TreeWidget()
{
}


void TreeWidget::startDrag(Qt::DropActions)
{
    item = this->currentItem();

    if (item->parent() == 0)
        return;

    data = new QByteArray();

    QDataStream stream(data, QIODevice::WriteOnly);
    stream << item->text(1);

    mimeData = new QMimeData();
    QString mimeType = "application/x-text";
    mimeData->setData(mimeType, *data);

    drag = new QDrag(this);

    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}
