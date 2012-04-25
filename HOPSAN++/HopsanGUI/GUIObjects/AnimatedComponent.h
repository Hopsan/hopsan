//!
//! @file   AnimatedComponent.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2011-07-07
//!
//! @brief Contains the abstract class for Animated Components
//!

#ifndef ANIMATEDCOMPONENT_H
#define ANIMATEDCOMPONENT_H


#include "GUIModelObject.h"
#include <QTransform>
#include <QList>
#include <QObject>
#include <QPen>
#include <cassert>
#include "../common.h"
#include "Widgets/AnimationWidget.h"


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
    ModelObject* getOriginal();

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
