#include "GUIConnector.h"

GUIConnector::GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent, QGraphicsScene *scene)
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
//    mTempLine = new QGraphicsLineItem(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this);
//    mLines.push_back(mTempLine);
//    mTempLine = new QGraphicsLineItem(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this);
//    mLines.push_back(mTempLine);
    this->setPen(QPen(color, 2));
    //this->mScene->addItem(mLine1);
   // this->mScene->addItem(mLine2);
}

GUIConnector::~GUIConnector()
{
}

void GUIConnector::SetEndPos(qreal x2, qreal y2)
{
    this->endPos.setX(x2);
    this->endPos.setY(y2);
}



void GUIConnector::setStartPort(GUIPort *port)
{
    this->mStartPort = port;
}

void GUIConnector::setEndPort(GUIPort *port)
{
    this->mEndPort = port;
}

GUIPort *GUIConnector::getStartPort()
{
    return this->mStartPort;
}

GUIPort *GUIConnector::getEndPort()
{
    return this->mEndPort;
}

void GUIConnector::updatePos()
{
    this->drawLine(this->getStartPort()->pos(), this->getEndPort()->pos());
}


void GUIConnector::drawLine(QPointF startPos, QPointF endPos)
{
    startPos = this->mapFromScene(startPos);
//    QPointF tempPos;
//    if (mLines.size() == 2)
//    {
//        tempPos = mLines[mLines.size()-2]->line().p1();
//    }
//    else
//    {
//        tempPos = mLines[mLines.size()-3]->line().p2();
//    }
    endPos = this->mapFromScene(endPos);

    mLines[0]->setLine(startPos.x(),
                       startPos.y(),
                       mLines[0]->line().p2().x(),
                       mLines[0]->line().p2().y());
    mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                     mLines[mLines.size()-1]->line().p1().y(),
                                     endPos.x(),
                                     endPos.y());

//    if (abs(tempPos.x()-endPos.x()) < abs(tempPos.y()-endPos.y()))
//    {
//        mLines[mLines.size()-2]->setLine(tempPos.x(), tempPos.y(), tempPos.x(), endPos.y());
//        mLines[mLines.size()-1]->setLine(tempPos.x(), endPos.y(), endPos.x(), endPos.y());
//    }
//    else
//    {
//        mLines[mLines.size()-2]->setLine(tempPos.x(), tempPos.y(), endPos.x(), tempPos.y());
//        mLines[mLines.size()-1]->setLine(endPos.x(), tempPos.y(), endPos.x(), endPos.y());
//    }
}

void GUIConnector::setPen(QPen pen)
{
    for (std::size_t i=0; i!=mLines.size(); ++i )
    {
        mLines[i]->setPen(pen);
    }
}


void GUIConnector::addLine()
{
    mTempLine = new QGraphicsLineItem(mLines[mLines.size()-1]->line().p2().x(),
                                      mLines[mLines.size()-1]->line().p2().y(),
                                      mLines[mLines.size()-1]->line().p2().x(),
                                      mLines[mLines.size()-1]->line().p2().y(),
                                      this);
    mTempLine->setPen(QPen(mColor,2));
    mLines.push_back(mTempLine);
    mLines[mLines.size()-2]->setPen(QPen(QColor("black"),2));
}

void GUIConnector::removeLine(QPointF cursorPos)
{
    if (mLines.size() > 1)
    {
        this->scene()->removeItem(mLines.back());
        mLines.pop_back();
    }
    this->drawLine(this->mapToScene(mLines[0]->line().p1()), cursorPos);
}

int GUIConnector::getNumberOfLines()
{
    return mLines.size();
}
