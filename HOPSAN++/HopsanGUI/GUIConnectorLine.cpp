//$Id$

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
#include <iostream>
#include <QDebug>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QDebug>

#include "GUIConnectorLine.h"


GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    //this->mParentConnector = parentConnector;
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    this->mParentConnectorEndPortConnected = false;
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

void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::VERTICAL)
    {
        this->setCursor(Qt::SizeHorCursor);
    }
    else if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::HORIZONTAL)
    {
           this->setCursor(Qt::SizeVerCursor);
    }
    emit lineHoverEnter();
}

void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}

GUIConnectorLine::geometryType GUIConnectorLine::getGeometry()
{
    return mGeometry;
}

void GUIConnectorLine::setGeometry(geometryType newgeometry)
{
    mGeometry=newgeometry;
}

int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}

QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        qDebug() << "Line selection status changed\n";
        if(this->isSelected())
        {
            this->setSelected(false);
            emit lineSelected();
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        qDebug() << "Line has moved\n";
        emit lineMoved(this->mLineNumber);
    }
    return value;
}

void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}
