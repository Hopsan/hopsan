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
    enum portType {POWER, READ, WRITE};
    GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, GUIPort::portType type, GUIComponent *parent = 0);
    ~GUIPort();
    QPointF rectPos;
    QGraphicsView *getParentView();
    GUIComponent *getComponent();
    void magnify(bool blowup);
    int getPortNumber();
    portType getPortType();
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
    GUIPort::portType mType;
};

#endif // GUIPORT_H
