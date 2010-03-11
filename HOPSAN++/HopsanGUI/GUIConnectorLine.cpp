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


//! Constructor.
//! @param x1 is the x-coordinate of the start position of the line.
//! @param y1 is the y-coordinate of the start position of the line.
//! @param x2 is the x-coordinate of the end position of the line.
//! @param y2 is the y-coordinate of the end position of the line.
//! @param primaryPen defines the default color and width of the line.
//! @param activePen defines the color and width of the line when it is selected.
//! @param hoverPen defines the color and width of the line when it is hovered by the mouse cursor.
//! @param lineNumber is the number of the line in the connector.
//! @param *parent is a pointer to the parent object.
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, QPen hoverPen, int lineNumber, QGraphicsItem *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->mPrimaryPen = primaryPen;
    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    this->mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    this->setGeometry(GUIConnectorLine::HORIZONTAL);
    this->mHasStartArrow = false;
    this->mHasEndArrow = false;
    this->mArrowSize = 10.0;
    this->mArrowAngle = 0.6;
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


//! Changes the style of the line to active.
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mActivePen);
}


//! Changes the style of the line to default.
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
        this->setPen(mPrimaryPen);
}


//! Changes the style of the line to hovered.
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mHoverPen);
}


//! Defines what shall happen if the line is clicked.
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit lineClicked();
}


//! Devines what shall happen if the mouse cursor enters the line. Change cursor if the line is movable.
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


//! Defines what shall happen when mouse cursor leaves the line.
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}


//! Returns the type of geometry (vertical, horizontal or diagonal) of the line.
//! @see setGeometry(geometryType newgeometry)
GUIConnectorLine::geometryType GUIConnectorLine::getGeometry()
{
    return mGeometry;
}


//! Sets the type of geometry (vertical, horizontal or diagonal) of the line.
//! @see getGeometry()
void GUIConnectorLine::setGeometry(geometryType newgeometry)
{
    mGeometry=newgeometry;
}


//! Returns the number of the line in the connector.
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Defines what shall happen if the line is selected or moved.
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
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
void GUIConnectorLine::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
{
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    if(this->mHasEndArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addEndArrow();
    }
    else if(this->mHasStartArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(x1,y1,x2,y2);
}


//! Adds an arrow at the end of the line.
//! @see addStartArrow()
void GUIConnectorLine::addEndArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasEndArrow = true;
}


//! Adds an arrow at the start of the line.
//! @see addEndArrow()
void GUIConnectorLine::addStartArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    this->mHasStartArrow = true;
}


//! Reimplementation of inherited setPen function to include arrow pen too.
void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(this->mHasStartArrow | this->mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}
