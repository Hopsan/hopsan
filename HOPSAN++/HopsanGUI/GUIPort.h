//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include "GUIComponent.h"
#include "HopsanCore.h"

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, GUIComponent *parent = 0);
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
    //QGraphicsItem *pRectParent;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    QGraphicsView *mpParentView;
    GUIComponent *mpParentComponent;
    qreal mMag;
    bool mIsMag;
};

#endif // GUIPORT_H
