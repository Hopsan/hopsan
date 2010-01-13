#include "graphicsview.h"

GraphicsView::GraphicsView(QWidget *parent)
        : QGraphicsView(parent)
{
    this->setAcceptDrops(true);
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
