#include "GraphicsConnectorItem.h"

GraphicsConnectorItem::GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent, QGraphicsScene *scene)
        : QGraphicsWidget(parent)
{
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->mScene = scene;
    this->mColor = color;
    mTempLine = new QGraphicsLineItem(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this);
    mLines.push_back(mTempLine);
    mTempLine = new QGraphicsLineItem(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this);
    mLines.push_back(mTempLine);
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
}


void GraphicsConnectorItem::drawLine(QPointF startPos, QPointF endPos)
{
    startPos = this->mapFromScene(startPos);
    QPointF tempPos;
    if (mLines.size() == 2)
    {
        tempPos = mLines[mLines.size()-2]->line().p1();
    }
    else
    {
        tempPos = mLines[mLines.size()-3]->line().p2();
    }
    endPos = this->mapFromScene(endPos);

    if (abs(tempPos.x()-endPos.x()) < abs(tempPos.y()-endPos.y()))
    {
        mLines[mLines.size()-2]->setLine(tempPos.x(), tempPos.y(), tempPos.x(), endPos.y());
        mLines[mLines.size()-1]->setLine(tempPos.x(), endPos.y(), endPos.x(), endPos.y());
    }
    else
    {
        mLines[mLines.size()-2]->setLine(tempPos.x(), tempPos.y(), endPos.x(), tempPos.y());
        mLines[mLines.size()-1]->setLine(endPos.x(), tempPos.y(), endPos.x(), endPos.y());
    }
}

void GraphicsConnectorItem::setPen(QPen pen)
{
    for (std::size_t i=0; i!=mLines.size(); ++i )
    {
        mLines[i]->setPen(pen);
    }
}


void GraphicsConnectorItem::addLine()
{
    mTempLine = new QGraphicsLineItem(0.0, 0.0, 0.0, 0.0, this);
    mTempLine->setPen(QPen(mColor,2));
    mLines.push_back(mTempLine);
    mLines[mLines.size()-3]->setPen(QPen(QColor("black"),2));
}

void GraphicsConnectorItem::removeLine()
{
    mLines.pop_back();
}
