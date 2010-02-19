#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H

#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
//#include "GraphicsConnectorItem.h"
#include "componentguiclass.h"

class ComponentGuiClass;        //Forwarad declaration

class GraphicsRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, ComponentGuiClass *component, QGraphicsItem *parent = 0);
    ~GraphicsRectItem();
    QPointF rectPos;
    QGraphicsView *getParentView();
    ComponentGuiClass *getComponent();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

signals:
    void portClicked(GraphicsRectItem *item);
    void portMoved(GraphicsRectItem *item);

private:
    QColor myLineColor;
    qreal myLineWidth;
    QGraphicsItem *pRectParent;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    QGraphicsView *mParentView;
    ComponentGuiClass *mComponent;
};

#endif // GRAPHICSRECTITEM_H
