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
//! @file   AnimatedComponent.cpp
//! @author Pratik Deschpande <pratik661@gmail.com>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-04-25
//!
//! @brief Contains a class for animated components
//!
//$Id$

#ifndef ANIMATEDCOMPONENT_H
#define ANIMATEDCOMPONENT_H

#include <QTransform>
#include <QList>
#include <QObject>
#include <QPen>
#include <cassert>

class AnimationWidget;
class ModelObject;
class ModelObjectAppearance;


class AnimatedComponent : public QObject
{
    Q_OBJECT
public:
    //Make room for some additional params passed form children
    AnimatedComponent(ModelObject* unanimatedComponent, QString basePath, QStringList movablePaths, QStringList dataPorts,
                      QStringList dataNames, QStringList parameterMultipliers, QStringList parameterDivisors,
                      QVector<double> movementX, QVector<double> movementY, QVector<double> movementTheta,
                      QVector<double> startX, QVector<double> startY, QVector<double> startTheta,
                      QVector<double> transformOriginX, QVector<double> transformOriginY, AnimationWidget *parent);

    virtual void draw();
    virtual void update();
   // ModelObject* getOriginal();

protected:
    void setupAnimationBase(QString basePath);
    void setupAnimationMovable(int n, QString movablePath, int movableXOffset, int movableYOffset, double transformOriginX, double transformOriginY);

    ModelObject *mpOriginal;
    ModelObject *mpUnanimatedComponent;
    AnimationWidget *mpParent;
    ModelObject *mpBase;
    QList<ModelObject *> mpMovables;
    QList<QVector<double> > *mpData;
    QVector<double> *mpMins;
    QVector<double> *mpMaxes;
    const QVector<double> *mpDataValues; // This used to be const
    void calculateMinsAndMaxes();

    QVector<double> mvMovementX;
    QVector<double> mvMovementY;
    QVector<double> mvMovementTheta;
    QVector<double> mStartX;
    QVector<double> mStartY;
    QVector<double> mStartTheta;
    QStringList mParameterMultipliers;
    QStringList mParameterDivisors;

    int mnMovableParts;
};

#endif // ANIMATEDCOMPONENT_H
