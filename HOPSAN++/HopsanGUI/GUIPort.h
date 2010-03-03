//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsRectItem>
//#include <QCursor>
//#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
//#include "GUIConnector.h"
#include "GUIComponent.h"

class GUIComponent;        //Forwarad declaration
class Port;

class GUIPort : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    GUIPort(Port *corePort, qreal x, qreal y, qreal width, qreal height, QGraphicsView *parentView, GUIComponent *component, QGraphicsItem *parent = 0);
    ~GUIPort();
    QPointF rectPos;
    QGraphicsView *getParentView();
    GUIComponent *getComponent();

    Port *mpCorePort;

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    //protected slots:
    void plot(size_t plotFlowAction);

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

private:
    QColor myLineColor;
    qreal myLineWidth;
    QGraphicsItem *pRectParent;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    QGraphicsView *mpParentView;
    GUIComponent *mpComponent;
};

#endif // GUIPORT_H
