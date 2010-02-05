#ifndef COMPONENTGUICLASS_H
#define COMPONENTGUICLASS_H

#include <QGraphicsSvgItem>
#include <QWidget>

#include "graphicsrectitem.h"

class ComponentGuiClass : public QGraphicsSvgItem
{
    Q_OBJECT
public:
    ComponentGuiClass(const QString &fileName, QPoint position, QGraphicsItem *parent = 0);
    ~ComponentGuiClass();

    QWidget *widget;
    GraphicsRectItem *rect;
};

#endif // COMPONENTGUICLASS_H
