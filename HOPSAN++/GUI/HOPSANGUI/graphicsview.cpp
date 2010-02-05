#include "graphicsview.h"
#include <iostream>
#include <math.h>

GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
    //this->setTransformationAnchor(QGraphicsView::NoAnchor);
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

        QString iconDir;
        stream >> iconDir;

        event->accept();

        QCursor cursor;
        QPoint position = this->mapFromGlobal(cursor.pos());

        std::cout << "x=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;

        guiComponent = new ComponentGuiClass(iconDir,position);
        this->scene()->addItem(guiComponent);

        delete data;
    }
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() and Qt::ControlModifier)
    {
        qreal factor = pow(1.41,(-event->delta()/240.0));
        this->scale(factor,factor);
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QCursor cursor;
    std::cout << "X=" << this->mapFromGlobal(cursor.pos()).x() << "  " << "Y=" << this->mapFromGlobal(cursor.pos()).y() << std::endl;
}
