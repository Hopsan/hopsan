#ifndef GRAPHICSRECTITEM_H
#define GRAPHICSRECTITEM_H

#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>

class GraphicsRectItem : public QGraphicsRectItem
{
public:
    GraphicsRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = 0);
    ~GraphicsRectItem();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
};

#endif // GRAPHICSRECTITEM_H
