#include "componentguiclass.h"
#include <iostream>

ComponentGuiClass::ComponentGuiClass(const QString &fileName, QString componentName,QPoint position, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    //widget = new QWidget;

    QGraphicsSvgItem *icon = new QGraphicsSvgItem(fileName,this);
    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
    std::cout << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << componentName.toStdString() << std::endl;

    QGraphicsTextItem *text = new QGraphicsTextItem(componentName,this);
    text->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    text->setPos(QPointF(-text->boundingRect().width()/2, icon->boundingRect().height()/2));
    text->setTextInteractionFlags(Qt::TextEditable);

    GraphicsRectItem *rectR = new GraphicsRectItem(icon->sceneBoundingRect().width()-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,icon);

    GraphicsRectItem *rectL = new GraphicsRectItem(-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,icon);

    //icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
}

ComponentGuiClass::~ComponentGuiClass()
{
    //delete widget;
}
