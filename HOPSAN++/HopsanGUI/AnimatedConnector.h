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
class Connector;
class AnimationWidget;
class ConnectorAppearance;

class AnimatedConnector : public QGraphicsWidget
{
    Q_OBJECT
    friend class AnimatedConnectorLine;
public:
    AnimatedConnector(Connector *pConnector, AnimationWidget *pAnimationWidget);
    ~AnimatedConnector();
    virtual void updateAnimation();

    AnimationWidget *mpAnimationWidget;
    Connector *mpConnector;

private:
    QVector<double> mvIntensityData;
    QVector<double> mvFlowData;
    int mDirectionCorrection;

    ConnectorAppearance *mpConnectorAppearance;

    QVector<AnimatedConnectorLine*> mpLines;
    QVector<connectorGeometry> mGeometries;
    QVector<QPointF> mPoints;

    QString mComponentName;
    QString mPortName;
};


class AnimatedConnectorLine : public QObject, public QGraphicsLineItem
{
    friend class AnimatedConnector;
    Q_OBJECT
public:
    AnimatedConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance *pConnApp, AnimatedConnector *pAnimatedConnector);
    ~AnimatedConnectorLine();

    void paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w);
    void addEndArrow();
    void addStartArrow();
    void setPen(const QPen &pen);

private:
    void clearArrows();

    AnimatedConnector *mpParentConnector;
    ConnectorAppearance *mpConnectorAppearance;

    bool mHasStartArrow;
    bool mHasEndArrow;

    QGraphicsLineItem *mArrowLine1;
    QGraphicsLineItem *mArrowLine2;
    qreal mArrowSize;
    qreal mArrowAngle;

    QPointF mStartPos;
    QPointF mEndPos;
};

#endif // ANIMATEDCONNECTOR_H
