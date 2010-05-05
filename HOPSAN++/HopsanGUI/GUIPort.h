//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include "GUIObject.h"
#include "HopsanCore.h"

//Forward declaration
class GUIObject;
class GUIComponent;

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    enum portDirectionType {VERTICAL, HORIZONTAL};
    GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, Port::PORTTYPE type, GUIPort::portDirectionType portDirection, GUIObject *parent = 0);
    ~GUIPort();
    void updatePosition();
    QGraphicsView *getParentView();
    GUIObject *getGuiObject();
    void magnify(bool blowup);
    portDirectionType getPortDirection();
    void setPortDirection(GUIPort::portDirectionType direction);
    QString getName();

    QPointF rectPos;
    int getPortNumber();
    Port::PORTTYPE getPortType();
    Port *mpCorePort;
    bool isConnected;

public slots:
    void hideIfNotConnected(bool justDoIt);

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
    GUIObject *mpParentGuiObject;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;
    //GUIPort::portType mType;
    portDirectionType mPortDirection;
    qreal mX;
    qreal mY;

};

#endif // GUIPORT_H
