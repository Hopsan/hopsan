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
#include "GraphicsView.h"
#include "GUIPort.h"
#include "Utilities/GUIUtilities.h"
#include "MainWindow.h"
#include "GUIModelObject.h"
#include "../common.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "../HopsanCore/include/Port.h"
//#include "GraphicsView.h"


AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, QString basePath, QStringList movablePaths, QStringList dataPorts,
                                     QStringList dataNames, QStringList parameterMultipliers, QStringList parameterDivisors, QVector<double> movementX, QVector<double> movementY, QVector<double> movementTheta, QVector<double> startX,
                                     QVector<double> startY, QVector<double> startTheta, QVector<double> transformOriginX, QVector<double> transformOriginY, QVector<bool> isAdjustable, QVector<double> adjustableMinX, QVector<double> adjustableMaxX, QVector<double> adjustableMinY, QVector<double> adjustableMaxY, QStringList adjustablePort, QStringList adjustableDataName, QVector<double> adjustableGainX, QVector<double> adjustableGainY, AnimationWidget *parent)
    : QObject(parent /*parent*/)
{
    //Something all Animated Components Should have. ie. Base, movable, etc.
    mpAnimationWidget = parent;
    mpModelObject = unanimatedComponent;
    //mpOriginal = new ModelObject(unanimatedComponent->getCenterPos().toPoint(),unanimatedComponent->rotation(),unanimatedComponent->getAppearanceData(),DESELECTED,USERGRAPHICS, unanimatedComponent->getParentContainerObject(),unanimatedComponent->parentItem());
   // gpMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView->scene()->removeItem(mpOriginal);
    mpAnimationWidget = parent;
    this->mpData = new QList<QVector<double> >();
    this->mpMins = new QVector<double>();
    this->mpMaxes = new QVector<double>();

    mStartX = startX;
    mStartY = startY;
    mStartTheta = startTheta;

    mvMovementX = movementX;
    mvMovementY = movementY;
    mvMovementTheta = movementTheta;

    mDataPorts = dataPorts;
    mDataNames = dataNames;

    mParameterMultipliers = parameterMultipliers;
    mParameterDivisors = parameterDivisors;

    mIsAdjustable = isAdjustable;
    mAdjustableMinX = adjustableMinX;
    mAdjustableMaxX = adjustableMaxX;
    mAdjustableMinY = adjustableMinY;
    mAdjustableMaxY = adjustableMaxY;
    mAdjustablePort = adjustablePort;
    mAdjustableDataName = adjustableDataName;
    mAdjustableGainX = adjustableGainX;
    mAdjustableGainY = adjustableGainY;

   // qDebug() << "mvMovementTheta = " << mvMovementTheta;

    setupAnimationBase(basePath);

    if(movablePaths.size() > 0)
    {
        for(int i=0; i<movablePaths.size(); ++i)
        {
            setupAnimationMovable(i,movablePaths.at(i),transformOriginX.at(i),transformOriginY.at(i));
            if(unanimatedComponent->getPort(dataPorts.at(i))->isConnected())
            {
                mpData->insert(i,mpAnimationWidget->getPlotDataPtr()->at(mpAnimationWidget->getNumberOfPlotGenerations()-1).find(unanimatedComponent->getName()).value().find(dataPorts.at(i)).value().find(dataNames.at(i)).value().second);
            }
        }

        this->calculateMinsAndMaxes();
    }

    mScaleX = 1.0;
}


void AnimatedComponent::draw()
{
    this->mpAnimationWidget->getScenePtr()->addItem(this->mpBase);

    if(mpMovables.size() > 0)
    {
        for(int m=0; m<mpMovables.size(); ++m)
        {
            this->mpAnimationWidget->getScenePtr()->addItem(this->mpMovables.at(m));
        }
    }
}


void AnimatedComponent::update()
{
    int a=0;    //a = "Adjustable index"
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationWidget->mRealTime && mIsAdjustable.at(m))
        {
            double value = mpMovables.at(m)->x()*mAdjustableGainX.at(a) +
                           mpMovables.at(m)->y()*mAdjustableGainY.at(a);
                     //qDebug() << "Setting " << mvAdjustablePorts.at(a) << ", " << mvAdjustableVariables.at(a) << " to " << value << "!";
            mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->writeNodeData(mpModelObject->getName(), mAdjustablePort.at(a), mAdjustableDataName.at(a), value);

            ++a;
        }
        else
        {
            double data;
            if(mpData->isEmpty())
            {
                return;//data = 0;
            }
            else
            {
                if(mpAnimationWidget->mRealTime)
                {
                    if(mpModelObject->getPort(mDataPorts.at(m))->isConnected())
                        mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(mpModelObject->getName(), mDataPorts.at(m), mDataNames.at(m), data);
                    else
                        data=0;
                }
                else
                {
                    data = mpData->at(m).at(mpAnimationWidget->getIndex());
                }
            }
    //        if(mParameterMultipliers[b] != QString())
    //        {
    //            data = data*mpUnanimatedComponent->getParameterValue(mParameterMultipliers[b]).toDouble();
    //        }
    //        if(mParameterDivisors[b] != QString())
    //        {
    //            data = data/mpUnanimatedComponent->getParameterValue(mParameterDivisors[b]).toDouble();
    //        }


            if(mvMovementTheta[m] != 0.0)
            {
                double rot = mStartTheta[m] - data*mvMovementTheta[m];
                mpMovables[m]->setRotation(rot);
            }

            if(mvMovementX[m] != 0.0 || mvMovementY[m] != 0.0)
            {
                double x = mStartX[m] - data*mvMovementX[m];
                double y = mStartY[m] - data*mvMovementY[m];
                mpMovables[m]->setPos(x, y);
            }
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
        basePath = mpModelObject->getAppearanceData()->getIconPath(USERGRAPHICS, ABSOLUTE);
        if(basePath.isEmpty())
        {
            basePath = mpModelObject->getAppearanceData()->getIconPath(ISOGRAPHICS, ABSOLUTE);
        }
        baseAppearance->setIconPath(basePath, USERGRAPHICS, ABSOLUTE);
        qDebug() << "Base path = " << basePath;
    }
    else
    {
        baseAppearance->setIconPath(basePath, USERGRAPHICS, RELATIVE);
    }
    mpBase = new AnimatedObject(mpModelObject->pos(),mpModelObject->rotation(),baseAppearance,DESELECTED,this,0,0);
    mpAnimationWidget->mpGraphicsScene->addItem(mpBase);
    if(mpModelObject->isFlipped())
    {
        mpBase->flipHorizontal();
    }
    mpBase->setCenterPos(mpModelObject->getCenterPos());
}


void AnimatedComponent::setupAnimationMovable(int n, QString movablePath, double transformOriginX, double transformOriginY)
{
    ModelObjectAppearance* pAppearance = new ModelObjectAppearance();
    pAppearance->setIconPath(movablePath,USERGRAPHICS, RELATIVE);
    this->mpMovables.append(new AnimatedObject(QPoint(0,0),0, pAppearance,DESELECTED,this, 0,mpBase));
    this->mpMovables.at(n)->setTransformOriginPoint(transformOriginX,transformOriginY);

    double rot = mpModelObject->rotation() - mStartTheta.at(n);
    //double x = mpUnanimatedComponent->getCenterPos().x() - mpBase->size().width()/2*cos(deg2rad(rot)) + mpBase->size().height()/2*sin(deg2rad(rot)) + mStartX.at(n);
    //double y = mpUnanimatedComponent->getCenterPos().y() - mpBase->size().height()/2*cos(deg2rad(rot)) - mpBase->size().width()/2*sin(deg2rad(rot)) + mStartY.at(n);

    mpMovables.at(n)->setRotation(mStartTheta.at(n));
    mpMovables.at(n)->setPos(mStartX.at(n), mStartY.at(n));

    mpMovables.at(n)->setFlag(QGraphicsItem::ItemIsMovable, false);
    //mpMovables.at(n)->setPos(QPointF(mpUnanimatedComponent->pos().x()+startX, mpUnanimatedComponent->pos().y()+startY));
    //qDebug() << "Setting pos to " << mpMovables.at(n)->getCenterPos();
    //mpMovables.at(n)->setRotation(mpOriginal->rotation());

    if(mIsAdjustable.at(n))
    {
        mpMovables.at(n)->setFlag(QGraphicsItem::ItemIsMovable);
    }
}


void AnimatedComponent::limitMovables()
{
    int a=0;
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(!mAdjustableMaxX.isEmpty() && mIsAdjustable.at(m))
        {
            if(mpMovables.at(m)->x() > mAdjustableMaxX.at(a))
            {
                mpMovables.at(m)->setX(mAdjustableMaxX.at(a));
            }
            else if(mpMovables.at(m)->x() < mAdjustableMinX.at(a))
            {
                mpMovables.at(m)->setX(mAdjustableMinX.at(a));
            }
            else if(mpMovables.at(m)->y() > mAdjustableMaxY.at(a))
            {
                mpMovables.at(m)->setY(mAdjustableMaxY.at(a));
            }
            else if(mpMovables.at(m)->y() < mAdjustableMinY.at(a))
            {
                mpMovables.at(m)->setY(mAdjustableMinY.at(a));
            }
        }

        ++a;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////




AnimatedObject::AnimatedObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, selectionStatus startSelected, AnimatedComponent *pAnimatedComponent, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, startSelected, pParentContainer, pParent)
{
    mOldPos = position;

        //Initialize variables
    mpAnimatedComponent = pAnimatedComponent;
    mpIcon = 0;

        //Set the hmf save tag name
    mHmfTagName = HMF_OBJECTTAG; //!< @todo change this

        //Make a local copy of the appearance data (that can safely be modified if needed)
    if (pAppearanceData != 0)
    {
        mModelObjectAppearance = *pAppearanceData;
    }

        //Setup appearance
    this->setIcon();
    this->refreshAppearance();
    this->setCenterPos(position);
    this->setZValue(MODELOBJECT_Z);
    this->setSelected(startSelected);
}


//! @brief Destructor for GUI Objects
AnimatedObject::~AnimatedObject()
{
    emit objectDeleted();
}

void AnimatedObject::setParentContainerObject(ContainerObject *pParentContainer)
{
    WorkspaceObject::setParentContainerObject(pParentContainer);
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int AnimatedObject::type() const
{
    return Type;
}


//! @brief Updates the icon of the object to user or iso style
//! @param gfxType Graphics type that shall be used
void AnimatedObject::setIcon()
{
    QString iconPath;
    qreal iconScale;
    iconPath = mModelObjectAppearance.getFullAvailableIconPath(USERGRAPHICS);
    iconScale = mModelObjectAppearance.getIconScale(USERGRAPHICS);
    mIconType = USERGRAPHICS;

    //Avoid swappping icon if same as before, we swap also if scale changes
    if  ( (mLastIconPath != iconPath) || !fuzzyEqual(mLastIconScale, iconScale, 0.001) )
    {
        if (mpIcon != 0)
        {
            mpIcon->deleteLater(); //Shedule previous icon for deletion
            disconnect(mpAnimatedComponent->mpAnimationWidget->mpGraphicsView, SIGNAL(zoomChange(qreal)), this, SLOT(setIconZoom(qreal)));
        }

        mpIcon = new QGraphicsSvgItem(iconPath, this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mpIcon->setScale(iconScale);

        this->prepareGeometryChange();
        this->resize(mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale);  //Resize modelobject
        mpSelectionBox->setSize(0.0, 0.0, mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale); //Resize selection box

        this->setTransformOriginPoint(this->boundingRect().center());

        if(mModelObjectAppearance.getIconRotationBehaviour(mIconType) == "ON")
        {
            mIconRotation = true;
            mpIcon->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
        }
        else
        {
            mIconRotation = false;
            mpIcon->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            if (this->getParentContainerObject() != 0)
            {
                mpIcon->setScale(this->getParentContainerObject()->mpParentProjectTab->getGraphicsView()->getZoomFactor()*iconScale);
                connect(this->getParentContainerObject()->mpParentProjectTab->getGraphicsView(), SIGNAL(zoomChange(qreal)), this, SLOT(setIconZoom(qreal)), Qt::UniqueConnection);
            }
            //! @todo we need to disconnect this also at some point, when swapping between systems or groups
        }

        mLastIconPath = iconPath;
        mLastIconScale = iconScale;
    }
}


void AnimatedObject::refreshIconPosition()
{
    //Only move when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setPos( this->mapFromScene(this->getCenterPos() - mpIcon->boundingRect().center() ));
    }
}


void AnimatedObject::setIconZoom(const qreal zoom)
{
    //Only scale when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setScale(mModelObjectAppearance.getIconScale(mIconType)*zoom);
    }
}



void AnimatedObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Moving an animated object!";

    QGraphicsItem::mouseMoveEvent(event);

//    this->setX(mapToScene(mOldPos).x());
}



//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant AnimatedObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    //WorkspaceObject::itemChange(change, value);

    // Move component only horizontal, vertical or snap to original position if Ctrl is pressed
    if (change == QGraphicsItem::ItemPositionHasChanged && this->scene() != 0)
    {
        mpAnimatedComponent->limitMovables();
    }

    return value;
}




//! @todo try to reuse the code in rotate guiobject,
void AnimatedObject::rotate(qreal angle)
{
    if(mIsFlipped)
    {
        angle *= -1;
    }
    this->setRotation(normDeg360(this->rotation()+angle));

    refreshIconPosition();
}


//! @brief Slot that flips the object vertically
//! @see flipHorizontal()
void AnimatedObject::flipVertical()
{
    this->flipHorizontal();
    this->rotate(180);
}


//! @brief Slot that flips the object horizontally
//! @see flipVertical()
void AnimatedObject::flipHorizontal()
{
    QTransform transf;
    transf.scale(-1.0, 1.0);

    //Remember center pos
    QPointF cpos = this->getCenterPos();
    //Transform
    this->setTransform(transf,true); // transformationorigin point seems to have no effect here for some reason
    //Reset to center pos (as transform origin point was ignored)
    this->setCenterPos(cpos);

    // If the icon is (not rotating) its position will be refreshed
    refreshIconPosition();

    // Toggel isFlipped bool
    if(mIsFlipped)
    {
        mIsFlipped = false;
    }
    else
    {
        mIsFlipped = true;
    }
}


//! @brief Returns a pointer to the appearance data object
ModelObjectAppearance* AnimatedObject::getAppearanceData()
{
    return &mModelObjectAppearance;
}


//! @brief Refreshes the appearance of the object
void AnimatedObject::refreshAppearance()
{
    //! @todo should make sure we only do this if we really need to resize (after icon change)
    QPointF centerPos =  this->getCenterPos(); //Remember center pos for resize
    this->setIcon();
    this->setCenterPos(centerPos); //Re-set center pos after resize
}


QGraphicsSvgItem *AnimatedObject::getIcon()
{
    return mpIcon;
}
