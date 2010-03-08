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
#include <vector>

GUIConnector::GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, QPen passivePen, QPen activePen, QPen hoverPen, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    this->setPos(x1, y1);
    this->startPos.setX(x1);
    this->startPos.setY(y1);
    this->endPos.setX(x2);
    this->endPos.setY(y2);
    this->mpParentView = parentView;
    this->mPassivePen = passivePen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mIsActive = false;
    this->mEndPortConnected = false;
    mpTempLine = new GUIConnectorLine(this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      this->mapFromScene(startPos).x(), this->mapFromScene(startPos).y(),
                                      mPassivePen, mActivePen, mHoverPen, 0, this);
    mLines.push_back(mpTempLine);
    connect(mLines[mLines.size()-1],SIGNAL(lineSelected(bool)),this,SLOT(doSelect(bool)));
    connect(mLines[mLines.size()-1],SIGNAL(lineClicked()),this,SLOT(doSelect()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    this->setActive();
    connect(this->mpParentView,SIGNAL(keyPressDelete()),this,SLOT(deleteMeIfMeIsActive()));
    connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
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
    for(std::size_t i=1; i!=mLines.size()-1; ++i)
    {
        mLines[i]->setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemUsesExtendedStyleOption | QGraphicsItem::ItemIsSelectable);
    }
    mLines[0]->setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption | QGraphicsItem::ItemIsSelectable);
    mLines[mLines.size()-1]->setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption | QGraphicsItem::ItemIsSelectable);
    emit endPortConnected();
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

void GUIConnector::doSelect(bool lineSelected)
{
    if(this->mEndPortConnected)     //Non-finished lines shall not be selectable
    {
        this->setSelected(lineSelected);
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
    //startPos = this->mapFromScene(startPos);
    //endPos = this->mapFromScene(endPos);

    //////////////Only used when moving components:///////////////
    getLine(0)->setLine(getLine(0)->mapFromScene(startPos).x(),
                        getLine(0)->mapFromScene(startPos).y(),
                        getLine(0)->mapFromParent(getLine(1)->mapToParent(getLine(1)->line().p1())).x(),
                        getLine(0)->mapFromScene(startPos).y());
    getLine(1)->setLine(getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).x(),
                        getLine(1)->mapFromParent(getLine(0)->mapToParent(getLine(0)->line().p2())).y(),
                        getLine(1)->line().x2(),
                        getLine(1)->line().y2());
    //////////////////////////////////////////////////////////////

    //First line of the connector:
    if (getNumberOfLines()<3 and getThisLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getLastLine()->setLine(getLastLine()->mapFromScene(startPos).x(),
                               getLastLine()->mapFromScene(startPos).y(),
                               getLastLine()->mapFromScene(endPos).x(),
                               getLastLine()->mapFromScene(startPos).y());
        getLastLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
        getThisLine()->setGeometry(GUIConnectorLine::VERTICAL);
    }

    //If last line was vertical:
    else if (getLastLine()->getGeometry()== GUIConnectorLine::VERTICAL and getThisLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getLastLine()->setLine(getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).x(),
                               getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).y(),
                               getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).x(),
                               getLastLine()->mapFromScene(endPos).y());
        getThisLine()->setGeometry(GUIConnectorLine::HORIZONTAL);
    }
    //If last line was horizontal:
    else if (getLastLine()->getGeometry()==GUIConnectorLine::HORIZONTAL and getThisLine()->getGeometry()!=GUIConnectorLine::DIAGONAL)
    {
        getLastLine()->setLine(getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).x(),
                               getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).y(),
                               getLastLine()->mapFromScene(endPos).x(),
                               getLastLine()->mapFromParent(getOldLine()->mapToParent(getOldLine()->line().p2())).y());
        getThisLine()->setGeometry(GUIConnectorLine::VERTICAL);
    }

    //If the line is diagonal:
//    if (getThisLine()->getGeometry()==GUIConnectorLine::DIAGONAL)
//    {
//        getThisLine()->setLine(getLastLine()->line().x2(),
//                               getLastLine()->line().y2(),
//                               endPos.x(),
//                               endPos.y());
//    }

    //This Line:
    getThisLine()->setLine(getThisLine()->mapFromParent(getLastLine()->mapToParent(getLastLine()->line().p2())).x(),
                           getThisLine()->mapFromParent(getLastLine()->mapToParent(getLastLine()->line().p2())).y(),
                           getThisLine()->mapFromScene(endPos).x(),
                           getThisLine()->mapFromScene(endPos).y());
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
                                      mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
    mpTempLine->setActive();
    mLines.push_back(mpTempLine);
    mLines[mLines.size()-2]->setPassive();
    connect(mLines[mLines.size()-1],SIGNAL(lineSelected(bool)),this,SLOT(doSelect(bool)));
    //connect(mLines[mLines.size()-1],SIGNAL(lineClicked()),this,SLOT(doSelect()));
    connect(mLines[mLines.size()-1],SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
}

void GUIConnector::addFixedLine(int length, int heigth, GUIConnectorLine::geometryType geometry)
{
    if(geometry == GUIConnectorLine::HORIZONTAL)
    {
        qDebug() << "HORIZONTAL from" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y();
        mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                          mLines[mLines.size()-1]->line().p2().x()+length, mLines[mLines.size()-1]->line().p2().y(),
                                          mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
    }
    else if(geometry == GUIConnectorLine::VERTICAL)
    {
        qDebug() << "VERTICAL from" << mLines[mLines.size()-1]->line().p2().x() << mLines[mLines.size()-1]->line().p2().y();
        mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                          mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y()+heigth,
                                          mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
    }
    else if(geometry == GUIConnectorLine::DIAGONAL)
    {
        mpTempLine = new GUIConnectorLine(mLines[mLines.size()-1]->line().p2().x(), mLines[mLines.size()-1]->line().p2().y(),
                                          mLines[mLines.size()-1]->line().p2().x()+length, mLines[mLines.size()-1]->line().p2().y()+heigth,
                                          mPassivePen, mActivePen, mHoverPen, mLines.size(), this);
    }
    mpTempLine->setGeometry(geometry);
    mpTempLine->setActive();
    mLines.push_back(mpTempLine);
    mLines[mLines.size()-2]->setPassive();
    connect(mLines[mLines.size()-1],SIGNAL(lineClicked(bool)),this,SLOT(doSelect(bool)));
    connect(mLines[mLines.size()-1],SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
    connect(mLines[mLines.size()-1],SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
    connect(this,SIGNAL(endPortConnected()),mLines[mLines.size()-1],SLOT(setConnected()));
}

void GUIConnector::removeLine(QPointF cursorPos)
{
    if (getNumberOfLines() > 2)
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
        this->deleteMe();
    }
}

void GUIConnector::updateLine(int lineNumber)
{
    qDebug() << "Updating line: x = " << getLine(lineNumber)->line().x2();
    if (this->mEndPortConnected && lineNumber != 0 && lineNumber != mLines.size())
    {
        if(getLine(lineNumber)->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
//            getLine(0)->setLine(startPos.x(),
//                                startPos.y(),
//                                getLine(1)->line().x1(),
//                                startPos.y());

            getLine(lineNumber-1)->setLine(getLine(lineNumber-1)->line().x1(),
                                           getLine(lineNumber-1)->line().y1(),
                                           getLine(lineNumber-1)->line().x2(),
                                           getLine(lineNumber-1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p1())).y());
            getLine(lineNumber+1)->setLine(getLine(lineNumber+1)->line().x1(),
                                           getLine(lineNumber+1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p2())).y(),
                                           getLine(lineNumber+1)->line().x2(),
                                           getLine(lineNumber+1)->line().y2());
            getLine(lineNumber)->setLine(getLine(lineNumber)->mapFromParent(getLine(lineNumber-1)->mapToParent(getLine(lineNumber-1)->line().p2())).x(),
                                         getLine(lineNumber)->line().y1(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber+1)->mapToParent(getLine(lineNumber+1)->line().p1())).x(),
                                         getLine(lineNumber)->line().y2());
        }
        else if(getLine(lineNumber)->getGeometry()==GUIConnectorLine::VERTICAL)
        {
            getLine(lineNumber-1)->setLine(getLine(lineNumber-1)->line().x1(),
                                           getLine(lineNumber-1)->line().y1(),
                                           getLine(lineNumber-1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p1())).x(),
                                           getLine(lineNumber-1)->line().y2());
            getLine(lineNumber+1)->setLine(getLine(lineNumber+1)->mapFromParent(getLine(lineNumber)->mapToParent(getLine(lineNumber)->line().p2())).x(),
                                           getLine(lineNumber+1)->line().y1(),
                                           getLine(lineNumber+1)->line().x2(),
                                           getLine(lineNumber+1)->line().y2());
            getLine(lineNumber)->setLine(getLine(lineNumber)->line().x1(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber-1)->mapToParent(getLine(lineNumber-1)->line().p2())).y(),
                                         getLine(lineNumber)->line().x2(),
                                         getLine(lineNumber)->mapFromParent(getLine(lineNumber+1)->mapToParent(getLine(lineNumber+1)->line().p1())).y());
        }
    }
    this->updatePos();
}


GUIConnectorLine *GUIConnector::getOldLine()
{
    return mLines[mLines.size()-3];
}

GUIConnectorLine *GUIConnector::getLastLine()
{
    return mLines[mLines.size()-2];
}


GUIConnectorLine *GUIConnector::getThisLine()
{
    return mLines[mLines.size()-1];
}

GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mLines[line];
}


QVariant GUIConnector::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        qDebug() << "Connector selection status = " << this->isSelected();
        if(!this->isSelected())
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
