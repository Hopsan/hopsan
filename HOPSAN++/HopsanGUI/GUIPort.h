//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include "GUIObject.h"
#include "HopsanCore.h"

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    //enum portType {POWER, READ, WRITE};
    enum portDirectionType {VERTICAL, HORIZONTAL};
    GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, Port::PORTTYPE type, GUIPort::portDirectionType portDirection, GUIComponent *parent = 0);
    ~GUIPort();
    QPointF rectPos;
    QGraphicsView *getParentView();
    GUIComponent *getComponent();
    void magnify(bool blowup);
    int getPortNumber();
    Port::PORTTYPE getPortType();
    Port *mpCorePort;
    portDirectionType getPortDirection();
    void setPortDirection(GUIPort::portDirectionType direction);

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
    //GUIPort::portType mType;
    portDirectionType mPortDirection;

};

#endif // GUIPORT_H
