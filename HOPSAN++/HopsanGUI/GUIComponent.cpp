#include "GUIComponent.h"
#include <iostream>
#include "GUIPort.h"
#include "GUIConnector.h"
#include <ostream>

GUIComponent::GUIComponent(const QString &fileName, QString componentName,QPoint position, QGraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setPos(position);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    //widget = new QWidget;

    mParentView = parentView;

    QGraphicsSvgItem *icon = new QGraphicsSvgItem(fileName,this);
    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
    std::cout << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << componentName.toStdString() << std::endl;

    QGraphicsTextItem *text = new QGraphicsTextItem(componentName,this);
    text->setTextInteractionFlags(Qt::TextEditorInteraction);
    text->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    text->setPos(QPointF(-text->boundingRect().width()/2, icon->boundingRect().height()/2));
    text->setTextInteractionFlags(Qt::TextEditable);

    GUIPort *rectR = new GUIPort(icon->sceneBoundingRect().width()-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);

    GUIPort *rectL = new GUIPort(-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon);

    //icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));

   // rectR->boundingRegion();
}

GUIComponent::~GUIComponent()
{
    //delete widget;
}


QGraphicsView *GUIComponent::getParentView()
{
    return mParentView;
}

void GUIComponent::addConnector(GUIConnector *item)
{
    mConnectors.push_back(item);
    connect(this,SIGNAL(componentMoved()),mConnectors.back(),SLOT(updatePos()));
    QColor color = QColor("black");
    mConnectors.back()->setPen(QPen(color, 2));
}

//void GUIComponent::moveEvent(QMoveEvent *event)
//{
//    emit componentMoved();
//    if (!mConnectors.empty())
//    {
//        QColor color = QColor("blue");
//        mConnectors.back()->setPen(QPen(color, 2));
//    }
//}


void GUIComponent::moveEvent(QGraphicsItem::GraphicsItemChange *change)
{
    if (*change == QGraphicsItem::ItemPositionChange)
    {
        emit componentMoved();
        if (!mConnectors.empty())
        {
            QColor color = QColor("blue");
            mConnectors.back()->setPen(QPen(color, 2));
        }
    }
}
