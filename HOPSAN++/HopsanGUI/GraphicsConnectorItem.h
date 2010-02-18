#ifndef GRAPHICSCONNECTORITEM_H
#define GRAPHICSCONNECTORITEM_H

#include <QCursor>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <vector>
#include "graphicsrectitem.h"

class GraphicsConnectorItem : public QGraphicsLineItem
{
public:
    GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent = 0);
    ~GraphicsConnectorItem();
    QPointF startPos;
    QPointF endPos;

protected:
    virtual void SetEndPos(qreal x2, qreal y2);

private:
    //std::vector<QGraphicsItem*> mPortVector;
};

#endif // GRAPHICSCONNECTORITEM_H
