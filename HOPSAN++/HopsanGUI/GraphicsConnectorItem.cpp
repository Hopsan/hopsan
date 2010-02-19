#include "GraphicsConnectorItem.h"

GraphicsConnectorItem::GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent, QGraphicsScene *scene)
        : QGraphicsWidget(parent)
{
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->mScene = scene;
    mLine1 = new QGraphicsLineItem(0.0, 0.0, 0.0, 0.0, this);
    mLine2 = new QGraphicsLineItem(0.0, 0.0, 0.0, 0.0, this);
    this->setPen(QPen(color, 2));
    //this->mScene->addItem(mLine1);
   // this->mScene->addItem(mLine2);
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
    this->drawLine(this->getStartPort()->pos(), this->getEndPort()->pos());
//    QColor color = QColor("blue");
//    this->setPen(QPen(color, 2));
//    QPointF startPos = this->getStartPort()->pos();
//    QPointF endPos = this->getEndPort()->pos();
//    this->setLine(0.0, 0.0, endPos.x(), endPos.y());
}


void GraphicsConnectorItem::drawLine(QPointF startPos, QPointF endPos)
{
    startPos = this->mapFromScene(startPos);
    endPos = this->mapFromScene(endPos);
    if (abs(startPos.x()-endPos.x()) < abs(startPos.y()-endPos.y()))
    {
        mLine1->setLine(startPos.x(), startPos.y(), startPos.x(), endPos.y());
        mLine2->setLine(startPos.x(), endPos.y(), endPos.x(), endPos.y());
    }
    else
    {
        mLine1->setLine(startPos.x(), startPos.y(), endPos.x(), startPos.y());
        mLine2->setLine(endPos.x(), startPos.y(), endPos.x(), endPos.y());
    }
}

void GraphicsConnectorItem::setPen(QPen pen)
{
    mLine1->setPen(pen);
    mLine2->setPen(pen);
}
