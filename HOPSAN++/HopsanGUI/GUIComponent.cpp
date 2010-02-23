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


GUIComponent::GUIComponent(const QString &fileName, QString componentName,QPoint position, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setPos(position);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges);

    //widget = new QWidget;

    mpParentView = parentView;

    this->setZValue(10);
    QGraphicsSvgItem *icon = new QGraphicsSvgItem(fileName,this);
//    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << "GUIcomponent: " << componentName.toStdString() << std::endl;

    //setWindowFlags(Qt::SplashScreen);//just to see the geometry
    setGeometry(0,0,icon->boundingRect().width(),icon->boundingRect().height());

    QGraphicsTextItem *text = new QGraphicsTextItem(componentName,this);
    text->setTextInteractionFlags(Qt::TextEditorInteraction);
    text->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    text->setPos(QPointF(icon->boundingRect().width()/2-text->boundingRect().width()/2, icon->boundingRect().height()));
    //text->setTextInteractionFlags(Qt::TextEditable);

    GUIPort *rectR = new GUIPort(icon->sceneBoundingRect().width()-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);

    GUIPort *rectL = new GUIPort(-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);
    //icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));

   // rectR->boundingRegion();

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

