#include "treewidget.h"
#include <iostream>

TreeWidget::TreeWidget(QWidget *parent)
        : QTreeWidget(parent)
{
    this->setDragEnabled(true);

    this->connect(this,SIGNAL(itemActivated(QTreeWidgetItem*,int)),SLOT(test(QTreeWidgetItem*,int)));
}

TreeWidget::~TreeWidget()
{
}


/*void TreeWidget::startDrag(Qt::DropActions)
{

    data = new QByteArray;
    stream = new QDataStream( data, QIODevice::WriteOnly);
    mimeData = new QMimeData;
    //drag = new QDrag(this);

    QTreeWidgetItem *item;
    item = this->currentItem();

    if (item->parent() == 0)
        return;

    *stream << item->text(1);

    QString mimeType = "application/x-text";
    mimeData->setData( mimeType, *data);

    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);

    delete data;
    delete stream;
    delete mimeData;
    //delete drag;
    delete item;
}
*/


void TreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


void TreeWidget::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QByteArray *data = new QByteArray;
    QDataStream stream(data,QIODevice::WriteOnly);

    QTreeWidgetItem *item = this->currentItem();//this->currentItem();
    if (item->parent() == 0)
            return;

    stream << item->text(0) << item->data(0,Qt::UserRole).toString();

    QString mimeType = "application/x-text";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, *data);
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);

    //delete data;
    //delete mimeData;
    //delete item;
}

void TreeWidget::test(QTreeWidgetItem *item,int num)
{
    std::cout << item->text(0).toStdString() << std::endl;
    if ( ((TreeWidgetItem*)item)->mlist->isVisible() == true)
    {
        ((TreeWidgetItem*)item)->mlist->hide();
    }
    else
    {
        ((TreeWidgetItem*)item)->mlist->show();
    }
}
