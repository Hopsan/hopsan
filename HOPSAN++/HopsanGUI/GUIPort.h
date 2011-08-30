/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   GUIPort.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPort class
//!
//$Id$

#ifndef GUIPORT_H
#define GUIPORT_H

#include <QGraphicsSvgItem>
//#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsWidget>

#include "common.h"
#include "GUIPortAppearance.h"

//Forward declarations
class GUIModelObject;
class GUISystem;
class GUIContainerObject;
class GUIConnector;
class PlotWindow;

enum PortDirectionT {TOPBOTTOM, LEFTRIGHT};

class GUIPort :public QGraphicsWidget
{
    Q_OBJECT
public:
    enum PortTypeIndicationT {ACTUALPORTTYPE, INTERNALPORTTYPE};

    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0);
    ~GUIPort();

    GUIContainerObject *getParentContainerObjectPtr();
    GUIModelObject *getGuiModelObject();
    QString getGuiModelObjectName();

    QString getPortName();
    void setDisplayName(const QString name);

    QPointF getCenterPos();
    qreal getPortRotation();
    PortDirectionT getPortDirection();
    void setCenterPos(qreal x, qreal y);
    void setCenterPosByFraction(qreal x, qreal y);
    void setRotation(qreal angle);

    void magnify(bool blowup);
    void show();
    void hide();

    virtual QString getPortType(const PortTypeIndicationT ind=ACTUALPORTTYPE);
    virtual QString getNodeType();

    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits);
    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits);
//    void setStartValueDataByNames(QVector<QString> names, QVector<double> values);
    bool setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt);


    bool getLastNodeData(QString dataName, double& rData);

    void addConnection(GUIConnector *pConnector);
    void removeConnection(GUIConnector *pConnector);
    bool isConnected();
    QVector<GUIPort *> getConnectedPorts();

    GUIModelObject *mpParentGuiModelObject; //!< @todo make private

public slots:
    void hideIfNotConnected(bool togglePortsActionTriggered);
    void setVisible(bool value);
    PlotWindow* plot(QString dataName, QString dataUnit=QString());
    void plotToPlotWindow(PlotWindow *pPlotWindow, QString dataName, QString dataUnit=QString());
    void refreshPortOverlayPosition();
    void refreshPortGraphics();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openRightClickMenu(QPoint screenPos);

protected slots:
    void refreshPortOverlayScale(qreal scale);

private:
    void refreshPortMainGraphics();
    void refreshPortOverlayGraphics();

//    QColor myLineColor;
//    qreal myLineWidth;
//    QGraphicsLineItem *lineH;
//    QGraphicsLineItem *lineV;

    QVector<GUIConnector*> mConnectedConnectors;
    GUIPortAppearance *mpPortAppearance;
    GUIPortAppearance mPortAppearanceAfterLastRefresh;
    QString mPortDisplayName;

    qreal mMag;
    qreal mOverlaySetScale;
    bool mIsMagnified;

    QGraphicsTextItem *mpPortLabel;
    //QVector<QGraphicsSvgItem*> mvPortGraphicsOverlayPtrs;
    QGraphicsSvgItem *mpCQSIconOverlay;
    QGraphicsSvgItem *mpMultiPortIconOverlay;
    QGraphicsSvgItem *mpMainIcon;
};


class GroupPort : public GUIPort
{
public:
    GroupPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0);
    QString getPortType(const PortTypeIndicationT ind=ACTUALPORTTYPE);
    QString getNodeType();
};

QPointF getOffsetPointfromPort(GUIPort *pStartPortGUIPort, GUIPort *pEndPort);

#endif // GUIPORT_H
