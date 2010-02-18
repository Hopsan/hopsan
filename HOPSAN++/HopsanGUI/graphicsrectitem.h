#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H

#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include "GraphicsConnectorItem.h"
#include "componentguiclass.h"

class GraphicsRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GraphicsRectItem();
    QPointF rectPos;
    QGraphicsView *getParentView();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void portClicked(GraphicsRectItem *item);

private:
    QColor myLineColor;
    qreal myLineWidth;
    QGraphicsItem *pRectParent;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    QGraphicsView *mParentView;
};

#endif // GRAPHICSRECTITEM_H
