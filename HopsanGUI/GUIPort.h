/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   GUIPort.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPort class
//!
//$Id$

#ifndef PORT_H
#define PORT_H

#include <QGraphicsSvgItem>
//#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsWidget>
#include <QSharedPointer>

#include "common.h"
#include "GUIPortAppearance.h"
#include "CoreAccess.h"

//Forward declarations
class ModelObject;
class SystemContainer;
class ContainerObject;
class Connector;
class PlotWindow;

enum PortDirectionT {TopBottomDirectionType, LeftRightDirectionType};

class Port :public QGraphicsWidget
{
    Q_OBJECT
public:
    Port(QString name, double xpos, double ypos, PortAppearance* pPortAppearance, ModelObject *pParent = 0);
    ~Port();

    ContainerObject *getParentContainerObject();
    ModelObject *getParentModelObject();
    const ModelObject *getParentModelObject() const;
    QString getParentModelObjectName() const;

    QString getName() const;
    void setDisplayName(const QString name);

    QPointF getCenterPos();
    double getPortRotation();
    PortDirectionT getPortDirection();
    void setCenterPos(double x, double y);
    void setCenterPosByFraction(double x, double y);
    void setRotation(double angle);

    void magnify(bool blowup);
    void show();
    void hide();

    void setEnable(bool enable);
    void setModified(bool modified);

    virtual QString getPortType(const CoreSystemAccess::PortTypeIndicatorT ind=CoreSystemAccess::ActualPortType);
    virtual QString getNodeType();
    QString getPortDescription();

    QStringList getVariableNames();
    QStringList getFullVariableNames();

    bool getLastNodeData(QString dataName, double& rData) const;

    void disconnectAndRemoveAllConnectedConnectors();
    virtual void rememberConnection(Connector *pConnector);
    virtual void forgetConnection(Connector *pConnector);
    QVector<Connector*> getAttachedConnectorPtrs() const; //!< @todo should this be virtual also
    virtual QVector<Port *> getConnectedPorts();
    virtual Port* getRealPort();

    bool isConnected();
    bool isAutoPlaced();

    const PortAppearance* getPortAppearance() const;

public slots:
    void showIfNotConnected(bool doShow=true);
    void setVisible(bool value);
    PlotWindow* plot(QString dataName, QString dataUnit=QString(), QColor desiredCurveColor=QColor());
    void refreshPortOverlayPosition();
    void refreshPortGraphics();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void openRightClickMenu(QPoint screenPos);
    void moveEvent(QGraphicsSceneMoveEvent *event);

    QVector<Connector*> mConnectedConnectors;

protected slots:
    void refreshPortOverlayScale(double scale);

private:
    void refreshPortMainGraphics();
    void refreshPortOverlayGraphics();
    void refreshPortLabelText();

    void openDefineAliasDialog(const QString &rVarName, const QString &rCurrentAlias="");

//    QColor myLineColor;
//    double myLineWidth;
//    QGraphicsLineItem *lineH;
//    QGraphicsLineItem *lineV;

    ModelObject *mpParentModelObject;

    PortAppearance *mpPortAppearance;
    PortAppearance mPortAppearanceAfterLastRefresh;
    QString mPortDisplayName;

    double mMag;
    double mOverlaySetScale;
    bool mIsMagnified;

    QGraphicsTextItem *mpPortLabel;
    //QVector<QGraphicsSvgItem*> mvPortGraphicsOverlayPtrs;
    QGraphicsSvgItem *mpCQSIconOverlay;
    QGraphicsSvgItem *mpMultiPortIconOverlay;
    QGraphicsSvgItem *mpMainIcon;
};

QPointF getOffsetPointfromPort(Port *pStartPortGUIPort, Port *pEndPort);

#endif // GUIPORT_H
