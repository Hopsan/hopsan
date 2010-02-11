#ifndef COMPONENTGUICLASS_H
#define COMPONENTGUICLASS_H

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>

#include "graphicsrectitem.h"

class ComponentGuiClass : public QGraphicsWidget
{
    Q_OBJECT
public:
    ComponentGuiClass(const QString &fileName, QString componentName, QPoint position, QGraphicsItem *parent = 0);
    ~ComponentGuiClass();

    //QWidget *widget;
};

#endif // COMPONENTGUICLASS_H
