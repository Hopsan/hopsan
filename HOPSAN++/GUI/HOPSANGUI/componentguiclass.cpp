#include "componentguiclass.h"
#include <iostream>

ComponentGuiClass::ComponentGuiClass(const QString &fileName,QPoint position, QGraphicsItem *parent)
        : QGraphicsSvgItem(fileName,parent)
{
    this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    widget = new QWidget;

    this->setPos(position);
    std::cout << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;

    rect = new GraphicsRectItem(this->sceneBoundingRect().width(),this->sceneBoundingRect().height(),10.0,10.0,this);
}

ComponentGuiClass::~ComponentGuiClass()
{
    delete widget;
}
