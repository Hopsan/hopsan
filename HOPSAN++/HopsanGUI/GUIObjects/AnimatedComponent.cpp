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

#include <math.h>
#include <float.h>

#include "AnimatedComponent.h"
#include "GUIPort.h"
#include "Utilities/GUIUtilities.h"
#include "MainWindow.h"
#include "GUIModelObject.h"
#include "../common.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/ProjectTabWidget.h"
//#include "GraphicsView.h"


AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, QString basePath, QStringList movablePaths, QStringList dataPorts,
                                     QStringList dataNames, QStringList parameterMultipliers, QStringList parameterDivisors, QVector<double> movementX, QVector<double> movementY, QVector<double> movementTheta, QVector<double> startX,
                                     QVector<double> startY, QVector<double> startTheta, QVector<double> transformOriginX, QVector<double> transformOriginY, AnimationWidget *parent)
    : QObject(parent /*parent*/)
{

    //Something all Animated Components Should have. ie. Base, movable, etc.
    mpParent = parent;
    mpUnanimatedComponent = unanimatedComponent;
    //mpOriginal = new ModelObject(unanimatedComponent->getCenterPos().toPoint(),unanimatedComponent->rotation(),unanimatedComponent->getAppearanceData(),DESELECTED,USERGRAPHICS, unanimatedComponent->getParentContainerObject(),unanimatedComponent->parentItem());
   // gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->scene()->removeItem(mpOriginal);
    mpParent = parent;
    this->mnMovableParts = movablePaths.size();
    this->mpData = new QList<QVector<double> >();
    this->mpMins = new QVector<double>();
    this->mpMaxes = new QVector<double>();

    mStartX = startX;
    mStartY = startY;
    mStartTheta = startTheta;

    mvMovementX = movementX;
    mvMovementY = movementY;
    mvMovementTheta = movementTheta;

    mParameterMultipliers = parameterMultipliers;
    mParameterDivisors = parameterDivisors;

   // qDebug() << "mvMovementTheta = " << mvMovementTheta;

    setupAnimationBase(basePath);

    if(mnMovableParts > 0)
    {
        for(int i=0; i<movablePaths.size(); ++i)
        {
            setupAnimationMovable(0,movablePaths.at(i),startX.at(i),startY.at(i),transformOriginX.at(i),transformOriginY.at(i));
            if(unanimatedComponent->getPort(dataPorts.at(i))->isConnected())
            {
                mpData->insert(i,mpParent->getPlotDataPtr()->at(mpParent->getNumberOfPlotGenerations()-1).find(unanimatedComponent->getName()).value().find(dataPorts.at(i)).value().find(dataNames.at(i)).value().second);
            }
        }

        this->calculateMinsAndMaxes();
    }


    mScaleX = 1.0;
}


void AnimatedComponent::draw()
{
    this->mpParent->getScenePtr()->addItem(this->mpBase);

    if(mnMovableParts > 0)
    {
        for(int b=0; b<mnMovableParts; ++b)
        {
            this->mpParent->getScenePtr()->addItem(this->mpMovables.at(b));
        }
    }
}

void AnimatedComponent::update()
{
    for(int b=0; b<mnMovableParts; ++b)
    {
        double data;
        if(mpData->isEmpty())
        {
            return;//data = 0;
        }
        else
        {
            data = mpData->at(b).at(mpParent->getIndex());
        }
//        if(mParameterMultipliers[b] != QString())
//        {
//            data = data*mpUnanimatedComponent->getParameterValue(mParameterMultipliers[b]).toDouble();
//        }
//        if(mParameterDivisors[b] != QString())
//        {
//            data = data/mpUnanimatedComponent->getParameterValue(mParameterDivisors[b]).toDouble();
//        }


        if(mvMovementTheta[b] != 0.0)
        {
            double rot = mStartTheta[b] - data*mvMovementTheta[b];
            mpMovables[b]->setRotation(rot);
        }

        if(mvMovementX[b] != 0.0 || mvMovementY[b] != 0.0)
        {
            double x = mStartX[b] - data*mvMovementX[b];
            double y = mStartY[b] - data*mvMovementY[b];
            mpMovables[b]->setPos(x, y);
        }
    }
}

void AnimatedComponent::calculateMinsAndMaxes()
{
    for(int h=0; h<mpData->size(); ++h)
    {
        double max = -DBL_MAX;
        for(int i=0; i<mpData->at(h).size(); ++i)
        {
            if(mpData->at(h).at(i) > max)
            {
                max = mpData->at(h).at(i);
            }
        }
        mpMaxes->insert(h,max);
    }

    for(int j=0;j<mpData->size();j++)
    {
        double min = DBL_MAX;
        for(int k=0;k<mpData->at(j).size();k++)
        {
            if(mpData->at(j).at(k)<min)
            {
                min = mpData->at(j).at(k);
            }
        }
        mpMins->insert(j,min);
    }
}


void AnimatedComponent::setupAnimationBase(QString basePath)
{
    ModelObjectAppearance *baseAppearance = new ModelObjectAppearance();
    //ModelObjectAppearance *baseAppearance = mpUnanimatedComponent->getAppearanceData();
    if(basePath.isEmpty())
    {
        basePath = mpUnanimatedComponent->getAppearanceData()->getIconPath(USERGRAPHICS, ABSOLUTE);
        if(basePath.isEmpty())
        {
            basePath = mpUnanimatedComponent->getAppearanceData()->getIconPath(ISOGRAPHICS, ABSOLUTE);
        }
        baseAppearance->setIconPath(basePath, USERGRAPHICS, ABSOLUTE);
        qDebug() << "Base path = " << basePath;
    }
    else
    {
        baseAppearance->setIconPath(basePath, USERGRAPHICS, RELATIVE);
    }
    mpBase = new ModelObject(mpUnanimatedComponent->pos().toPoint(),mpUnanimatedComponent->rotation(),baseAppearance,DESELECTED,USERGRAPHICS,0,0);
    mpParent->mpGraphicsScene->addItem(mpBase);
    if(mpUnanimatedComponent->isFlipped())
    {
        mpBase->flipHorizontal(NOUNDO);
    }
    mpBase->setCenterPos(mpUnanimatedComponent->getCenterPos());
}


void AnimatedComponent::setupAnimationMovable(int n, QString movablePath, int startX, int startY, double transformOriginX, double transformOriginY)
{
    ModelObjectAppearance* tempNUMBER = new ModelObjectAppearance();
    tempNUMBER->setIconPath(movablePath,USERGRAPHICS, RELATIVE);
    this->mpMovables.append(new ModelObject(QPoint(0,0),0, tempNUMBER ,DESELECTED,USERGRAPHICS,0,mpBase));
    this->mpMovables.at(n)->setTransformOriginPoint(transformOriginX,transformOriginY);

    double rot = mpUnanimatedComponent->rotation() - mStartTheta.at(n);
    double x = mpUnanimatedComponent->getCenterPos().x() - mpBase->size().width()/2*cos(deg2rad(rot)) + mpBase->size().height()/2*sin(deg2rad(rot)) + mStartX.at(n);
    double y = mpUnanimatedComponent->getCenterPos().y() - mpBase->size().height()/2*cos(deg2rad(rot)) - mpBase->size().width()/2*sin(deg2rad(rot)) + mStartY.at(n);

    mpMovables.at(n)->setRotation(mStartTheta.at(n));
    mpMovables.at(n)->setPos(mStartX.at(n), mStartY.at(n));

    //mpMovables.at(n)->setPos(QPointF(mpUnanimatedComponent->pos().x()+startX, mpUnanimatedComponent->pos().y()+startY));
    //qDebug() << "Setting pos to " << mpMovables.at(n)->getCenterPos();
    //mpMovables.at(n)->setRotation(mpOriginal->rotation());
}


