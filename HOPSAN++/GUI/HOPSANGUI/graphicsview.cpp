#include "graphicsview.h"
#include <iostream>

GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
}

GraphicsView::~GraphicsView()
{
    delete guiComponent;
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void GraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-text"))
    {
        QByteArray *data = new QByteArray;
        *data = event->mimeData()->data("application/x-text");

        QDataStream stream(data,QIODevice::ReadOnly);

        QString text;
        QString iconDir;
        stream >> iconDir;

        event->accept();

        std::cout << iconDir.toStdString() << std::endl;
        guiComponent = new ComponentGuiClass(iconDir);
        this->scene()->addItem(guiComponent);
        //std::cout << text.toStdString() << std::endl;
        //std::cout << iconDir.toStdString() << std::endl;
        delete data;
    }
}
