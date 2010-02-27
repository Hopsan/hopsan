//$Id$

#include "HopsanCore.h"
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

GUIPort::GUIPort(Port *kernelPort, qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, GUIComponent *component, QGraphicsItem *parent)
        : QGraphicsRectItem(x, y, width, height,parent)
{
    //Core interaction
    mpKernelPort = kernelPort;
    //

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


void GUIPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit portClicked(this);
    std::cout << "GUIPort.cpp: " << "portClick emitted\n";
}
