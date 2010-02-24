//$Id$

#include "GUIComponent.h"
#include <iostream>
#include "GUIPort.h"
#include "GUIConnector.h"
#include <ostream>
#include <assert.h>
#include <QDebug>

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include <vector>
#include <QGraphicsItem>

#include <QGraphicsSceneMoveEvent>
#include <QDebug>

#include <math.h>

GUIComponent::GUIComponent(const QString &fileName, QString componentName,QPoint position, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setPos(position);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges);

    //widget = new QWidget;

    mpParentView = parentView;

    this->setZValue(10);
    icon = new QGraphicsSvgItem(fileName,this);
//    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << "GUIcomponent: " << componentName.toStdString() << std::endl;

    //setWindowFlags(Qt::SplashScreen);//just to see the geometry
    setGeometry(0,0,icon->boundingRect().width(),icon->boundingRect().height());

    text = new GUIComponentTextItem(componentName,this);
    text->setPos(QPointF(icon->boundingRect().width()/2-text->boundingRect().width()/2, icon->boundingRect().height()));

    GUIPort *rectR = new GUIPort(icon->sceneBoundingRect().width()-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);

    GUIPort *rectL = new GUIPort(-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);
    //icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));

   // rectR->boundingRegion();

    connect(text, SIGNAL(textMoved(QGraphicsSceneMouseEvent *)), SLOT(fixTextPosition(QGraphicsSceneMouseEvent *)));
}


double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}
void GUIComponent::fixTextPosition(QGraphicsSceneMouseEvent * event)
{
    double x1 = icon->boundingRect().width()/2-text->boundingRect().width()/2;
    double y1 = icon->boundingRect().height();

    double x2 = icon->boundingRect().width()/2-text->boundingRect().width()/2;
    double y2 = -text->boundingRect().height();

    double x = text->mapToParent(event->pos()).x();
    double y = text->mapToParent(event->pos()).y();

    if (dist(x,y, x1,y1) > dist(x,y, x2,y2))
    {
        text->setPos(x2,y2);
    }
    else
    {
        text->setPos(x1,y1);
    }

    std::cout << "GUIComponent::fixTextPosition, x: " << x << " y: " << y << std::endl;

}


void GUIComponent::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    qDebug() << "GUIComponent: " << "mouseReleaseEvent";
    QGraphicsItem::mouseReleaseEvent(event);
}


GUIComponent::~GUIComponent()
{
    //delete widget;
}


QGraphicsView *GUIComponent::getParentView()
{
    return mpParentView;
}

void GUIComponent::addConnector(GUIConnector *item)
{
    mConnectors.push_back(item);
    connect(this,SIGNAL(componentMoved()),mConnectors.back(),SLOT(updatePos()));
    QColor color = QColor("black");
    mConnectors.back()->setPen(QPen(color, 2));
}


void GUIComponent::moveEvent(QGraphicsSceneMoveEvent *event)
{
    emit componentMoved();
}


GUIComponentTextItem::GUIComponentTextItem(const QString &text, QGraphicsItem *parent)
    :   QGraphicsTextItem(text, parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setTextInteractionFlags(Qt::TextEditorInteraction);
    //setTextInteractionFlags(Qt::TextEditable);
}


void GUIComponentTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "GUIComponentTextItem: " << "mouseReleaseEvent";
    emit textMoved(event);
    QGraphicsItem::mouseReleaseEvent(event);
}


void GUIComponentTextItem::keyReleaseEvent(QKeyEvent *event) //Vill inte...
{
    std::cout << "GUIComponentTextItem::keyPressEvent: " << event->key() << std::endl;
    //QGraphicsItem::keyPressEvent(event);
}

