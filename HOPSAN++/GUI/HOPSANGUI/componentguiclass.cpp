#include "componentguiclass.h"

ComponentGuiClass::ComponentGuiClass(const QString &fileName, QGraphicsItem *parent)
        : QGraphicsSvgItem(fileName,parent)
{
    this->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);

    widget = new QWidget;

    this->setPos(0,0);
}

ComponentGuiClass::~ComponentGuiClass()
{
    delete widget;
}
