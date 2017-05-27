/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

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
class AnimatedComponent;

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
    AnimatedComponent *mpStartComponent;
    AnimatedComponent *mpEndComponent;
    QString mStartPortName;
    QString mEndPortName;

    ConnectorAppearance *mpConnectorAppearance;

    QVector<AnimatedConnectorLine*> mpLines;
    QVector<ConnectorGeometryEnumT> mGeometries;
    QVector<QPointF> mPoints;

    QString mComponentName;
    QString mPortName;

    double *mpDataPressure, *mpDataFlow;
};


class AnimatedConnectorLine : public QObject, public QGraphicsLineItem
{
    friend class AnimatedConnector;
    Q_OBJECT
public:
    AnimatedConnectorLine(double x1, double y1, double x2, double y2, ConnectorAppearance *pConnApp, AnimatedConnector *pAnimatedConnector);
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
    double mArrowSize;
    double mArrowAngle;

    QPointF mStartPos;
    QPointF mEndPos;
};

#endif // ANIMATEDCONNECTOR_H
