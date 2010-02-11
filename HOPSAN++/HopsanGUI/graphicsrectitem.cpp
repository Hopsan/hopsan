#include "graphicsrectitem.h"

GraphicsRectItem::GraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
        : QGraphicsRectItem(x, y, width, height,parent)
{
    this->setAcceptHoverEvents(true);

    QBrush brush(Qt::green);
    this->setBrush(brush);
}

GraphicsRectItem::~GraphicsRectItem()
{
}

void GraphicsRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::CrossCursor);
}
