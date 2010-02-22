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

GUIPort::GUIPort(qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, GUIComponent *component, QGraphicsItem *parent)
        : QGraphicsRectItem(x, y, width, height,parent)
{
    mParentView = parentView;
    mComponent = component;
    rectPos.setX(x);
    rectPos.setY(y);
    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    QBrush brush(Qt::green);
    this->setBrush(brush);

    QObject::connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
}

GUIPort::~GUIPort()
{
}

void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    this->setBrush(brush);
    std::cout << "GUIPort.cpp: " << "hovering over port\n";
}

void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QBrush brush(Qt::green);
    this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
}


QGraphicsView *GUIPort::getParentView()
{
    return mParentView;
}

GUIComponent *GUIPort::getComponent()
{
    return mComponent;
}

//void GUIPort::portClicked(GUIPort *item)
//{
//}


void GUIPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    //if (event->button() != Qt::LeftButton)
    //    return;
    emit portClicked(this);
    std::cout << "GUIPort.cpp: " << "portClick emitted\n";

//    QPointF newPos(5.0, 5.0);
//    QPointF oldPos = this->mapToScene(this->boundingRect().center());
//    myLineWidth = 2.0;
//    myLineColor = QColor("black");
//    GUIConnector *lineH = new GUIConnector(oldPos.x(), oldPos.y(), oldPos.x(), 0.0, myLineWidth, myLineColor, this);
//    GUIConnector *lineV = new GUIConnector(oldPos.x(), 0.0, 0.0, 0.0, myLineWidth, myLineColor, this);

    //this->scene()->addItem(&*lineH);
    //this->scene()->addItem(&*lineV);

}

