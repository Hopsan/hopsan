#include "treewidget.h"

TreeWidget::TreeWidget(QWidget *parent)
        : QTreeWidget(parent)
{

}

TreeWidget::~TreeWidget()
{
}


void TreeWidget::startDrag(Qt::DropActions)
{
    item = this->currentItem();

    data = new QByteArray();

    QDataStream stream(data, QIODevice::WriteOnly);
    stream << item->text(1);

    mimeData = new QMimeData();
    QString mimeType = "application/x-text";
    mimeData->setData(mimeType, *data);

    drag->setMimeData(mimeData);
    drag->start(Qt::MoveAction);
}
