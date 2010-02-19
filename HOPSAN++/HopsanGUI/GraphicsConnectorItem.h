#ifndef GRAPHICSCONNECTORITEM_H
#define GRAPHICSCONNECTORITEM_H

#include <QCursor>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <vector>
#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include "GraphicsConnectorItem.h"
#include "graphicsrectitem.h"

class GraphicsRectItem;     //Forwarad declaration

class GraphicsConnectorItem : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GraphicsConnectorItem(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~GraphicsConnectorItem();
    QPointF startPos;
    QPointF endPos;
    //ComponentGuiClass* getPort();
    void setStartPort(GraphicsRectItem *port);
    void setEndPort(GraphicsRectItem *port);
    GraphicsRectItem *getStartPort();
    GraphicsRectItem *getEndPort();
    void drawLine(QPointF startPos, QPointF endPos);

public slots:
    void updatePos();

protected:
    virtual void SetEndPos(qreal x2, qreal y2);

private:
    GraphicsRectItem *mStartPort;
    GraphicsRectItem *mEndPort;
    QGraphicsScene *mScene;
};

#endif // GRAPHICSCONNECTORITEM_H
