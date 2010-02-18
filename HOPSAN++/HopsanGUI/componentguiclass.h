#ifndef COMPONENTGUICLASS_H
#define COMPONENTGUICLASS_H

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
//#include "graphicsrectitem.h"

class ComponentGuiClass : public QGraphicsWidget
{
    Q_OBJECT
public:
    ComponentGuiClass(const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~ComponentGuiClass();
    QGraphicsView *getParentView();

private:
    QGraphicsView *mParentView;
    //QWidget *widget;
};

#endif // COMPONENTGUICLASS_H
