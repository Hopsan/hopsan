#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsRectItem>
#include <QCursor>
#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
//#include "GUIConnector.h"
#include "componentguiclass.h"

class ComponentGuiClass;        //Forwarad declaration

class GUIPort : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GUIPort(qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, ComponentGuiClass *component, QGraphicsItem *parent = 0);
    ~GUIPort();
    QPointF rectPos;
    QGraphicsView *getParentView();
    ComponentGuiClass *getComponent();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

private:
    QColor myLineColor;
    qreal myLineWidth;
    QGraphicsItem *pRectParent;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    QGraphicsView *mParentView;
    ComponentGuiClass *mComponent;
};

#endif // GUIPORT_H
