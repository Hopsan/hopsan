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
//! @file   AnimatedConnector.cpp
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-04-25
//!
//! @brief Contains a class for animated connectors
//!
//$Id$

#ifndef ANIMATEDCONNECTOR_H
#define ANIMATEDCONNECTOR_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPen>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMoveEvent>
#include <QTextStream>
#include <QtXml>

#include "common.h"


class AnimatedConnectorLine;
class GraphicsView;
class WorkspaceObject;
class Port;
class SystemContainer;
class ContainerObject;
class Connector;
class AnimationWidget;
class ConnectorAppearance;

class AnimatedConnector : public QGraphicsWidget
{
    Q_OBJECT
    friend class AnimatedConnectorLine;
public:
    AnimatedConnector(Connector *pConnector, AnimationWidget *parent);
    ~AnimatedConnector();
    virtual void update();

    AnimationWidget *mpAnimationWidget;
    Connector *mpConnector;

private:
    bool mIsActive;
    bool mIsDashed;
    QVector<double> mvIntensityData;
    QVector<double> mvFlowData;
    double mMaxIntensity;
    double mMinIntensity;
    int mDirectionCorrection;

    ConnectorAppearance *mpConnectorAppearance;

    QVector<AnimatedConnectorLine*> mpLines;
    QVector<connectorGeometry> mGeometries;
    QVector<QPointF> mPoints;
};


class AnimatedConnectorLine : public QObject, public QGraphicsLineItem
{
    friend class AnimatedConnector;
    Q_OBJECT
public:
    AnimatedConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance *pConnApp, int lineNumber, AnimatedConnector *parent = 0);
    ~AnimatedConnectorLine();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    void addEndArrow();
    void addStartArrow();
    void setActive();
    void setPassive();
    void setHovered();
    void setGeometry(connectorGeometry geometry);
    void setLine(QPointF pos1, QPointF pos2);
    int getLineNumber();
    void setPen(const QPen &pen);

public slots:

    void setConnected();

signals:
    void lineClicked();
    void lineMoved(int);
    void lineHoverEnter();
    void lineHoverLeave();
    void lineSelected(bool isSelected, int lineNumber);

private:
    void clearArrows();

    AnimatedConnector *mpParentConnector;
    ConnectorAppearance *mpConnectorAppearance;

    bool mIsActive;
    bool mParentConnectorEndPortConnected;
    bool mHasStartArrow;
    bool mHasEndArrow;
    int mLineNumber;

    connectorGeometry mGeometry;
    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    qreal mArrowSize;
    qreal mArrowAngle;

    QPointF mStartPos;
    QPointF mEndPos;
    QPointF mOldPos;
};

#endif // ANIMATEDCONNECTOR_H
