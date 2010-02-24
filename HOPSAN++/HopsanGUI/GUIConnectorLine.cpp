//$Id$

#include "GUIPort.h"
#include <QObject>
#include <QGraphicsObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QTabWidget>
#include <QStringList>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QWidget>
#include <QGraphicsItem>
#include "GUIComponent.h"
#include <iostream>
#include <QDebug>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include "GUIConnectorLine.h"
#include <QDebug>


GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    setFlags(QGraphicsItem::ItemStacksBehindParent | QGraphicsItem::ItemSendsGeometryChanges);
    //this->mParentConnector = parentConnector;
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
}

GUIConnectorLine::~GUIConnectorLine()
{
}

void GUIConnectorLine::setActive()
{
        this->setPen(mActivePen);
}

void GUIConnectorLine::setPassive()
{
        this->setPen(mPrimaryPen);
}

void GUIConnectorLine::setHovered()
{
        this->setPen(mHoverPen);
}

void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit lineClicked();
}

void GUIConnectorLine::moveEvent(QGraphicsSceneMoveEvent *event)
{
    qDebug() << "Moving line " << this->mLineNumber;
    emit lineMoved(this->mLineNumber);
}

void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverEnter();
}

void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}
