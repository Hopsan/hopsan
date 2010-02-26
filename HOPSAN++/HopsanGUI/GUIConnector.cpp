//$Id$

#include "GUIConnector.h"
#include <QDebug>

#include <QCursor>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include "GUIPort.h"
#include <vector>
#include "GUIConnectorLine.h"


GUIConnector::GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QColor activecolor, QColor hovercolor, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    this->setPos(x1, y1);
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->mpParentView = parentView;
    this->mPrimaryColor = color;
    this->mActiveColor = activecolor;
    this->mHoverColor = hovercolor;
    this->mWidth = width;
    this->mIsActive = false;
    this->mEndPortConnected = false;
    mpTempLine = new GUIConnectorLine(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      QPen(this->mPrimaryColor, this->mWidth), QPen(this->mActiveColor, this->mWidth),
                                      QPen(this->mHoverColor, this->mWidth), 0, this);
    mLines.push_back(mpTempLine);
    connect(mLines[mLines.size()-1],SIGNAL(lineClicked()),this,SLOT(doSelect()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    this->setPen(QPen(mActiveColor, mWidth));
    this->mStraight = true;
    connect(this->mpParentView,SIGNAL(keyPressDelete()),this,SLOT(deleteMeIfIMeIsActive()));
    //connect(this->mpParentView,SIGNAL(viewClicked()),this,SLOT(setPassive()));

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
    connect(this->mpStartPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));
}

void GUIConnector::setEndPort(GUIPort *port)
{
    this->mpEndPort = port;
    this->mEndPortConnected = true;
    connect(this->mpEndPort->getComponent(),SIGNAL(componentMoved()),this,SLOT(updatePos()));

    qDebug() << this->boundingRect().x() << " " << this->boundingRect().y() << " ";
    connect(this->mpEndPort->getComponent(),SIGNAL(componentDeleted()),this,SLOT(deleteMe()));
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

void GUIConnector::doSelect()
{
    if(this->mEndPortConnected)     //Non-finished lines shall not be selectable
    {
        this->setSelected(true);
        qDebug() << "doSelect()";
    }
}

void GUIConnector::setActive()
{
    if(this->mEndPortConnected)
    {
        mIsActive = true;
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setActive();
        }
        qDebug() << "setActive()";
    }
}

void GUIConnector::setPassive()
{
    if(this->mEndPortConnected)
    {
        mIsActive = false;
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setPassive();
        }
    }
}

void GUIConnector::setUnHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setPassive();
        }
    }
}

void GUIConnector::setHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (std::size_t i=0; i!=mLines.size(); ++i )
        {
            mLines[i]->setHovered();
        }
    }
}

void GUIConnector::drawLine(QPointF startPos, QPointF endPos)
{
    startPos = this->mapFromScene(startPos);
    endPos = this->mapFromScene(endPos);

    mLines[0]->setLine(startPos.x(),
                       startPos.y(),
                       mLines[0]->line().p2().x(),
                       mLines[0]->line().p2().y());
    if (mStraight) //If straight lines are activated
    {
        if (mLines.size()==1) //Special case for the first line
        {
            //if (abs(mLines[mLines.size()-1]->line().p1().x()-endPos.x()) > abs(mLines[mLines.size()-1]->line().p1().y()-endPos.y()))
            //{
                mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                                 mLines[mLines.size()-1]->line().p1().y(),
                                                 endPos.x(),
                                                 mLines[mLines.size()-1]->line().p1().y());
        }
        else //After the first line
        {
            //If the previous line is horizontal:
            if (mLines[mLines.size()-2]->line().p1().y() == (mLines[mLines.size()-2]->line().p2().y()))
            {
                //qDebug() << "Previous Line Horizontal";
                mLines[mLines.size()-2]->setLine(mLines[mLines.size()-2]->line().p1().x(),
                                                 mLines[mLines.size()-2]->line().p1().y(),
                                                 endPos.x(),
                                                 mLines[mLines.size()-2]->line().p2().y());
            }

            //If the previous line is vertical:
            else if (mLines[mLines.size()-2]->line().p1().x() == (mLines[mLines.size()-2]->line().p2().x()))
            {
                mLines[mLines.size()-2]->setLine(mLines[mLines.size()-2]->line().p1().x(),
                                                 mLines[mLines.size()-2]->line().p1().y(),
                                                 mLines[mLines.size()-2]->line().p2().x(),
                                                 endPos.y());
            }
            //If the previous line was not "straight":
            else
            {
                if (abs(endPos.x()-mLines[mLines.size()-1]->line().p1().x())>abs(endPos.y()-mLines[mLines.size()-1]->line().p1().y()))
                {
                    mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                                     mLines[mLines.size()-1]->line().p1().y(),
                                                     endPos.x(),
                                                     mLines[mLines.size()-1]->line().p2().y());

                }
                else
                {
                    mLines[mLines.size()-1]->setLine(mLines[mLines.size()-1]->line().p1().x(),
                                                     mLines[mLines.size()-1]->line().p1().y(),
                                                     mLines[mLines.size()-1]->line().p2().x(),
                                                     endPos.y());
                }
            }

            //Current line
            mLines[mLines.size()-1]->setLine(mLines[mLines.size()-2]->line().p2().x(),
                                             mLines[mLines.size()-2]->line().p2().y(),
                                             endPos.x(),
                                             endPos.y());
        }
    }

    else //If straight lines are inactivated
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
    mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                      mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                      QPen(this->mPrimaryColor, this->mWidth), QPen(this->mActiveColor, this->mWidth),
                                      QPen(this->mHoverColor, this->mWidth), mLines.size(), this);
    mpTempLine->setActive();
    mLines.push_back(mpTempLine);
    mLines[mLines.size()-2]->setPassive();
    connect(mLines[mLines.size()-1],SIGNAL(lineClicked()),this,SLOT(doSelect()));
    connect(mLines[mLines.size()-1],SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
}

void GUIConnector::removeLine(QPointF cursorPos)
{
    if (mLines.size() > 1)
    {
        this->scene()->removeItem(mLines.back());
        mLines.pop_back();
        this->drawLine(this->mapToScene(mLines[0]->line().p1()), cursorPos);
    }
    else
    {
        this->scene()->removeItem(this);
        delete(this);
    }
}

int GUIConnector::getNumberOfLines()
{
    return mLines.size();
}


void GUIConnector::setStraight(bool var)
{
    mStraight = var;
}


bool GUIConnector::isStraight()
{
    return mStraight;
}

void GUIConnector::deleteMe()
{
    mLines.clear();
    this->scene()->removeItem(this);
    delete(this);
}


void GUIConnector::deleteMeIfMeIsActive()
{
    if(this->mIsActive && mLines.size() > 0)
    {
        mLines.clear();
        this->scene()->removeItem(this);
        delete(this);
    }
}

void GUIConnector::updateLine(int lineNumber)
{
    mLines[lineNumber-1]->setLine(mLines[lineNumber-1]->line().p1().x(),
                                  mLines[lineNumber-1]->line().p1().y(),
                                  mLines[lineNumber]->line().p1().x(),
                                  mLines[lineNumber]->line().p1().x());
    mLines[lineNumber+1]->setLine(mLines[lineNumber]->line().p2().x(),
                                  mLines[lineNumber]->line().p2().y(),
                                  mLines[lineNumber+1]->line().p2().x(),
                                  mLines[lineNumber+1]->line().p2().x());
}


QVariant GUIConnector::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        qDebug() << "Line selection status = " << this->isSelected();
        if(this->isSelected())
        {
            this->setPassive();
        }
        else
        {
            this->setActive();
        }
    }
    return value;
}
