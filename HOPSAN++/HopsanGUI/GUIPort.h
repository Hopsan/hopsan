//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include "GUIObject.h"
#include "HopsanCore.h"
#include "AppearanceData.h"

//Forward declaration
class GUIObject;
class GUIComponent;
class GraphicsView;

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    //GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, Port::PORTTYPE type, GUIPort::portDirectionType portDirection, GUIObject *parent = 0);
    GUIPort(QString name, qreal xpos, qreal ypos, PortAppearance* pPortAppearance, GUIObject *pParent = 0);
    ~GUIPort();
    void updatePosition();
    GraphicsView *getParentView();
    GUIObject *getGuiObject();
    void magnify(bool blowup);
    PortAppearance::portDirectionType getPortDirection();
    void setPortDirection(PortAppearance::portDirectionType direction);
    QString getName();
    QString getGUIComponentName();

    QPointF rectPos;
    int getPortNumber();

    QString getPortType();
    QString getNodeType();
    //*****Core Interaction*****
    Port *mpCorePort;
    //**************************

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
    GraphicsView *mpParentGraphicsView;
    GUIObject *mpParentGuiObject;
    QGraphicsTextItem *mpPortLabel;
    qreal mMag;
    bool mIsMag;
    //GUIPort::portType mType;

    PortAppearance *mpPortAppearance;
    qreal mXpos;
    qreal mYpos;

};

#endif // GUIPORT_H
