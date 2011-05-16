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
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

#include "common.h"
#include "GUIPortAppearance.h"

//Forward declarations
class GUIModelObject;
class GUISystem;
class GUIContainerObject;

enum portDirection {TOPBOTTOM, LEFTRIGHT};

class GUIPort :public QGraphicsSvgItem
{
    Q_OBJECT
public:
    enum PortTypeIndicationT {ACTUALPORTTYPE, INTERNALPORTTYPE};

    GUIPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent = 0);
    ~GUIPort();
    virtual void refreshParentContainerSigSlotConnections();

    GUIContainerObject *getParentContainerObjectPtr();
    GUIModelObject *getGuiModelObject();
    QString getName();
    void setDisplayName(const QString name);
    QString getGuiModelObjectName();
    QPointF getCenterPos();
    portDirection getPortDirection();
    qreal getPortHeading();
    bool getLastNodeData(QString dataName, double& rData);

    void updatePosition(qreal x, qreal y);
    void updatePositionByFraction(qreal x, qreal y);
    void magnify(bool blowup);
    void show();
    void hide();

    virtual QString getPortType(const PortTypeIndicationT ind=ACTUALPORTTYPE);
    virtual QString getNodeType();

    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits);
    void getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits);
//    void setStartValueDataByNames(QVector<QString> names, QVector<double> values);
    bool setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt);

    void addConnection();
    void removeConnection();
    bool isConnected();

    GUIModelObject *mpParentGuiModelObject; //!< @todo should be private or protected
    QPointF rectPos;

public slots:
    void hideIfNotConnected(bool togglePortsActionTriggered);
    void setVisible(bool value);
    bool plot(QString dataName, QString dataUnit=QString());
    void refreshPortOverlayPosition();
    void refreshPortGraphics();
    void refreshPortGraphics(QString cqsType, QString portType, QString nodeType);

signals:
    void portClicked(GUIPort *item);
    void portMoved(GUIPort *item);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void addPortGraphicsOverlay(QStringList filepaths);
    void openRightClickMenu(QPoint screenPos);

protected slots:
    void setPortOverlayScale(qreal scale);

private:
    void setPortOverlayIconScale();

    QColor myLineColor;
    qreal myLineWidth;
    QGraphicsLineItem *lineH;
    QGraphicsLineItem *lineV;
    qreal mMag;
    qreal mOverlaySetScale;
    bool mIsMagnified;
    size_t mnConnections;
    GUIPortAppearance *mpPortAppearance;
    QString mName;
    QGraphicsTextItem *mpPortLabel;
    QVector<QGraphicsSvgItem*> mvPortGraphicsOverlayPtrs;
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
