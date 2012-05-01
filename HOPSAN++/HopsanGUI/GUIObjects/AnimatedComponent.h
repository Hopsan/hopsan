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
#include <QGraphicsSvgItem>

#include "GUIObject.h"
#include "GUIModelObjectAppearance.h"

class AnimationWidget;
class AnimatedObject;
class ModelObject;
class ModelObjectAppearance;


class AnimatedComponent : public QObject
{
    Q_OBJECT

    friend class AnimatedObject;
public:
    //Make room for some additional params passed form children
    AnimatedComponent(ModelObject* unanimatedComponent, QString basePath, QStringList movablePaths, QStringList dataPorts,
                      QStringList dataNames, QStringList parameterMultipliers, QStringList parameterDivisors,
                      QVector<double> movementX, QVector<double> movementY, QVector<double> movementTheta,
                      QVector<double> startX, QVector<double> startY, QVector<double> startTheta,
                      QVector<double> transformOriginX, QVector<double> transformOriginY,
                      QVector<bool> isAdjustable, QVector<double> adjustableMinX, QVector<double> adjustableMaxX, QVector<double> adjustableMinY,
                      QVector<double> adjustableMaxY, QStringList adjustablePort, QStringList adjustableDataName,
                      QVector<double> adjustableGainX, QVector<double> adjustableGainY, AnimationWidget *parent);

    void draw();
    void update();
   // ModelObject* getOriginal();

private:
    void setupAnimationBase(QString basePath);
    void setupAnimationMovable(int n, QString movablePath, double transformOriginX, double transformOriginY);
    void limitMovables();

    ModelObject *mpModelObject;
    AnimationWidget *mpAnimationWidget;
    AnimatedObject *mpBase;
    QList<AnimatedObject *> mpMovables;
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

    QStringList mDataPorts;
    QStringList mDataNames;

    QVector<bool> mIsAdjustable;
    QVector<double> mAdjustableMinX;
    QVector<double> mAdjustableMaxX;
    QVector<double> mAdjustableMinY;
    QVector<double> mAdjustableMaxY;
    QStringList mAdjustablePort;
    QStringList mAdjustableDataName;
    QVector<double> mAdjustableGainX;
    QVector<double> mAdjustableGainY;

    double mScaleX;
};



//////////////////////////////////////////////////////////////////////////////////////////////


class AnimatedObject : public WorkspaceObject
{
    Q_OBJECT

public:
    AnimatedObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, selectionStatus startSelected = DESELECTED,  AnimatedComponent *pAnimatedComponent=0, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);
    virtual ~AnimatedObject();

    AnimatedComponent *mpAnimatedComponent;

    virtual void setParentContainerObject(ContainerObject *pParentContainer);

    //Appearance methods
    virtual ModelObjectAppearance* getAppearanceData();
    QGraphicsSvgItem *getIcon();

    enum { Type = ANIMATEDOBJECT };
    int type() const;

public slots:
    virtual void refreshAppearance();
    void rotate(qreal angle);
    void flipVertical();
    void flipHorizontal();
    void setIcon();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    //Protected members
    ModelObjectAppearance mModelObjectAppearance;

    graphicsType mIconType;
    bool mIconRotation;
    QGraphicsSvgItem *mpIcon;
    QString mLastIconPath;
    qreal mLastIconScale;

protected slots:
    void setIconZoom(const qreal zoom);

private:
    void refreshIconPosition();
};


#endif // ANIMATEDCOMPONENT_H
