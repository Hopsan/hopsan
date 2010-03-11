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
#include <QStyleOptionGraphicsItem>
#include "GUIConnectorLine.h"
#include <QPointF>
#include <math.h>


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
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    this->setGeometry(GUIConnectorLine::HORIZONTAL);
    this->mHasArrow = false;
}

GUIConnectorLine::~GUIConnectorLine()
{
}


//! Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! Changes the style of the line to active
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mActivePen);
}


//! Changes the style of the line to default
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
        this->setPen(mPrimaryPen);
}


//! Changes the style of the line to hovered
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mHoverPen);
}



//! Emits line clicked signal
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit lineClicked();
}


//! Emits hover enter signal and changes cursor if line is modifyable
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::VERTICAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
        else if(this->mParentConnectorEndPortConnected && this->getGeometry()==GUIConnectorLine::HORIZONTAL)
        {
               this->setCursor(Qt::SizeVerCursor);
        }
    }
    emit lineHoverEnter();
}


//! Emits hover leave event
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}


//! Returns the type of geometry (vertical, horizontal or diagonal) of the line
//! @see setGeometry(geometryType newgeometry)
GUIConnectorLine::geometryType GUIConnectorLine::getGeometry()
{
    return mGeometry;
}


//! Sets the type of geometry (vertical, horizontal or diagonal) of the line
//! @see getGeometry()
void GUIConnectorLine::setGeometry(geometryType newgeometry)
{
    mGeometry=newgeometry;
}


//! Returns the number of the line in the connector
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Emits selected and moved signals
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        qDebug() << "Line selection status = " << this->isSelected();
        emit lineSelected(this->isSelected());
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        qDebug() << "Line has moved\n";
        emit lineMoved(this->mLineNumber);
    }
    return value;
}


//! Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! Reimplementation of setLine; stores the start and end positions before changing them
void GUIConnectorLine::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    QGraphicsLineItem::setLine(x1,y1,x2,y2);
}


void GUIConnectorLine::addEndArrow()
{
    qreal arrowSize = 15.0;
    qreal arrowAngle = 0.5;
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    qDebug() << "Angle = " << angle;
    qDebug() << this->endPos.y() << this->startPos.y() << this->endPos.x() << this->startPos.x();
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-arrowSize*cos(angle+arrowAngle),
                                        this->endPos.y()-arrowSize*sin(angle+arrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-arrowSize*cos(angle-arrowAngle),
                                        this->endPos.y()-arrowSize*sin(angle-arrowAngle), this);
    mArrowLine1->setPen(this->pen());
    mArrowLine2->setPen(this->pen());
    mArrowLine1->show();
    mArrowLine2->show();
    this->mHasArrow = true;
}


void GUIConnectorLine::addStartArrow()
{
    qreal arrowSize = 15.0;
    qreal arrowAngle = 0.5;
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    qDebug() << "Angle = " << angle;
    qDebug() << this->endPos.y() << this->startPos.y() << this->endPos.x() << this->startPos.x();
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+arrowSize*cos(angle+arrowAngle),
                                        this->startPos.y()+arrowSize*sin(angle+arrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+arrowSize*cos(angle-arrowAngle),
                                        this->startPos.y()+arrowSize*sin(angle-arrowAngle), this);
    mArrowLine1->setPen(this->pen());
    mArrowLine2->setPen(this->pen());
    mArrowLine1->show();
    mArrowLine2->show();
    this->mHasArrow = true;
}

void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(this->mHasArrow)
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}

