#include "GraphicsConnectorItem.h"

GraphicsConnectorItem::GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent, QGraphicsScene *scene)
        : QGraphicsLineItem(x1, y1, x2, y2)
{
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->setPen(QPen(color, 2));
    this->mScene = scene;
}

GraphicsConnectorItem::~GraphicsConnectorItem()
{
}

void GraphicsConnectorItem::SetEndPos(qreal x2, qreal y2)
{
    this->endPos.setX(x2);
    this->endPos.setY(y2);
}



void GraphicsConnectorItem::setStartPort(GraphicsRectItem *port)
{
    this->mStartPort = port;
}

void GraphicsConnectorItem::setEndPort(GraphicsRectItem *port)
{
    this->mEndPort = port;
}

GraphicsRectItem *GraphicsConnectorItem::getStartPort()
{
    return this->mStartPort;
}

GraphicsRectItem *GraphicsConnectorItem::getEndPort()
{
    return this->mEndPort;
}

void GraphicsConnectorItem::updatePos()
{
    QColor color = QColor("blue");
    this->setPen(QPen(color, 2));
    QPointF startPos = this->getStartPort()->pos();
    QPointF endPos = this->getEndPort()->pos();
    this->setLine(0.0, 0.0, endPos.x(), endPos.y());
}


void GraphicsConnectorItem::drawLine(QPointF startPos, QPointF endPos)
{
    //qreal x2 = endPos.x();
    //qreal y2 = endPos.y();
    //line->setLine(line->startPos.x(), line->startPos.y(), x2, y2);
}
