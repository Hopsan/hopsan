#include "graphicsconnectoritem.h"

GraphicsConnectorItem::GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent)
        : QGraphicsLineItem(x1, y1, x2, y2)
{
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->setPen(QPen(color, 2));
    //this-
}

GraphicsConnectorItem::~GraphicsConnectorItem()
{
}


void GraphicsConnectorItem::SetEndPos(qreal x2, qreal y2)
{
    this->endPos.setX(x2);
    this->endPos.setY(y2);
}
