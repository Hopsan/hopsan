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
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->setAcceptHoverEvents(true);

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
    connect(this->mpParentView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));

    mpSelectionBox = new QGraphicsRectItem(0,0,icon->boundingRect().width(),icon->boundingRect().height(),this);
    mpSelectionBox->setPen(QPen(QColor("red"),2));
    mpSelectionBox->setVisible(false);
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
    connect(this,SIGNAL(componentMoved()),item,SLOT(updatePos()));
}

void GUIComponent::deleteComponent()
{
    qDebug() << "Debug123\n";
    if(this->isSelected())
    {
        this->scene()->removeItem(this);
        delete(this);
    }
}

void GUIComponent::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setPen(QPen(QColor("darkRed"),2));
        this->mpSelectionBox->setVisible(true);
    }
}

void GUIComponent::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setVisible(false);
    }
}

QVariant GUIComponent::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        qDebug() << "Component selected status = " << this->isSelected();
        this->mpSelectionBox->setPen(QPen(QColor("red"),2));
        this->mpSelectionBox->setVisible(!this->isSelected());
    }
    else if (change == QGraphicsItem::ItemPositionChange)
    {
        emit componentMoved();
    }
    return value;
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





