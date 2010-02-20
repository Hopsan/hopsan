#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H

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
#include "GUIConnector.h"
#include "graphicsrectitem.h"
#include <vector>

class GraphicsRectItem;     //Forwarad declaration

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    ~GUIConnector();
    QPointF startPos;
    QPointF endPos;
    //ComponentGuiClass* getPort();
    void setStartPort(GraphicsRectItem *port);
    void setEndPort(GraphicsRectItem *port);
    GraphicsRectItem *getStartPort();
    GraphicsRectItem *getEndPort();
    void drawLine(QPointF startPos, QPointF endPos);
    void addLine();
    void removeLine(QPointF cursorPos);
    void setPen(QPen pen);
    int getNumberOfLines();

public slots:
    void updatePos();

protected:
    virtual void SetEndPos(qreal x2, qreal y2);

private:
    std::vector<QGraphicsLineItem*> mLines;
    GraphicsRectItem *mStartPort;
    GraphicsRectItem *mEndPort;
    QGraphicsScene *mScene;
    QGraphicsLineItem *mTempLine;
    QColor mColor;
    //QGraphicsLineItem *mLine1;
    //QGraphicsLineItem *mLine2;
};

#endif // GUICONNECTOR_H
