//$Id$

#include "GUIConnector.h"

GUIConnector::GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QColor activecolor, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->setPos(x1, y1);

    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->mpParentView = parentView;
    this->mPrimaryColor = color;
    this->mActiveColor = activecolor;
    this->mWidth = width;
    mpTempLine = new QGraphicsLineItem(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(), this);
    mLines.push_back(mpTempLine);
    this->setPen(QPen(mActiveColor, mWidth));
    this->mStraigth = false;
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
    this->mpStartPort = port;
    connect(this->mpStartPort->getComponent(),SIGNAL(componentMoved()),this,SLOT(updatePos()));
}

void GUIConnector::setEndPort(GUIPort *port)
{
    this->mpEndPort = port;
    connect(this->mpEndPort->getComponent(),SIGNAL(componentMoved()),this,SLOT(updatePos()));
}

GUIPort *GUIConnector::getStartPort()
{
    return this->mpStartPort;
}

GUIPort *GUIConnector::getEndPort()
{
    return this->mpEndPort;
}

void GUIConnector::updatePos()
{
    QPointF startPort = this->getStartPort()->mapToScene(this->getStartPort()->boundingRect().center());
    QPointF endPort = this->getEndPort()->mapToScene(this->getEndPort()->boundingRect().center());
    this->drawLine(startPort, endPort);
}

void GUIConnector::drawLine(QPointF startPos, QPointF endPos)
{
    startPos = this->mapFromScene(startPos);
    endPos = this->mapFromScene(endPos);

    mLines[0]->setLine(startPos.x(),
                       startPos.y(),
                       mLines[0]->line().p2().x(),
                       mLines[0]->line().p2().y());
    if (mStraigth)
    {
        if (abs(mLines[mLines.size()-1]->line().p1().x()-endPos.x()) > abs(mLines[mLines.size()-1]->line().p1().y()-endPos.y()))
        {
            mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                             mLines[mLines.size()-1]->line().p1().y(),
                                             endPos.x(),
                                             mLines[mLines.size()-1]->line().p1().y());
        }
        else
        {
            mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                             mLines[mLines.size()-1]->line().p1().y(),
                                             mLines[mLines.size()-1]->line().p1().x(),
                                             endPos.y());
        }
    }
    else
    {
        mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                         mLines[mLines.size()-1]->line().p1().y(),
                                         endPos.x(),
                                         endPos.y());
    }

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
    mpTempLine = new QGraphicsLineItem(mLines[mLines.size()-1]->line().p2().x(),
                                      mLines[mLines.size()-1]->line().p2().y(),
                                      mLines[mLines.size()-1]->line().p2().x(),
                                      mLines[mLines.size()-1]->line().p2().y(),
                                      this);
    mpTempLine->setPen(QPen(mActiveColor,mWidth));
    mLines.push_back(mpTempLine);
    mLines[mLines.size()-2]->setPen(QPen(mPrimaryColor,mWidth));
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


void GUIConnector::setStraigth(bool var)
{
    mStraigth = var;
}


bool GUIConnector::isStraigth()
{
    return mStraigth;
}
