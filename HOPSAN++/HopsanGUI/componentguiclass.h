#ifndef COMPONENTGUICLASS_H
#define COMPONENTGUICLASS_H

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include "GraphicsConnectorItem.h"
#include <vector>
#include <QGraphicsItem>


class GraphicsConnectorItem;

class ComponentGuiClass : public QGraphicsWidget
{
    Q_OBJECT
public:
    ComponentGuiClass(const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~ComponentGuiClass();
    QGraphicsView *getParentView();
    void addConnector(GraphicsConnectorItem *item);

protected:
    //virtual void moveEvent(QGraphicsItem::GraphicsItemChange);
    virtual void moveEvent(QGraphicsItem::GraphicsItemChange *change);

signals:
    void componentMoved();

private:
    QGraphicsView *mParentView;
    std::vector<GraphicsConnectorItem*> mConnectors;        //Inteded to store connectors for each component
    //QWidget *widget;
};

#endif // COMPONENTGUICLASS_H
