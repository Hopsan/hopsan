#include "graphicsrectitem.h"
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
#include "componentguiclass.h"
#include <iostream>

GraphicsRectItem::GraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, ComponentGuiClass *component, QGraphicsItem *parent)
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

    QObject::connect(this,SIGNAL(portClicked(GraphicsRectItem*)),this->getParentView(),SLOT(addConnector(GraphicsRectItem*)));
}

GraphicsRectItem::~GraphicsRectItem()
{
}

void GraphicsRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //this->setCursor(Qt::CrossCursor);
    
    QBrush brush(Qt::blue);
    this->setBrush(brush);
    std::cout << "hovering over port\n";
}

void GraphicsRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{

    QBrush brush(Qt::green);
    this->setBrush(brush);
}


QGraphicsView *GraphicsRectItem::getParentView()
{
    return mParentView;
}

ComponentGuiClass *GraphicsRectItem::getComponent()
{
    return mComponent;
}

//void GraphicsRectItem::portClicked(GraphicsRectItem *item)
//{
//}


void GraphicsRectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

    //if (event->button() != Qt::LeftButton)
    //    return;
    emit portClicked(this);
    std::cout << "portClick emitted\n";

//    QPointF newPos(5.0, 5.0);
//    QPointF oldPos = this->mapToScene(this->boundingRect().center());
//    myLineWidth = 2.0;
//    myLineColor = QColor("black");
//    GraphicsConnectorItem *lineH = new GraphicsConnectorItem(oldPos.x(), oldPos.y(), oldPos.x(), 0.0, myLineWidth, myLineColor, this);
//    GraphicsConnectorItem *lineV = new GraphicsConnectorItem(oldPos.x(), 0.0, 0.0, 0.0, myLineWidth, myLineColor, this);

    //this->scene()->addItem(&*lineH);
    //this->scene()->addItem(&*lineV);

}

