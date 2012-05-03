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

#include "common.h"
#include "AnimatedComponent.h"
#include "GraphicsView.h"
#include "GUIModelObject.h"
#include "GUIPort.h"
#include "MainWindow.h"
#include "../HopsanCore/include/Port.h"
#include "Dialogs/AnimatedIconPropertiesDialog.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUIContainerObject.h"


//! @brief Constructor for the animated component class
AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, AnimationWidget *parent)
    : QObject(parent /*parent*/)
{
    //Set member pointer variables
    mpAnimationWidget = parent;
    mpModelObject = unanimatedComponent;
    mpAnimationWidget = parent;
    mpData = new QList<QVector<double> >();

    //Set the animation data object
    mAnimationData = unanimatedComponent->getAppearanceData()->getAnimationData();

    //Setup the base icon
    setupAnimationBase(mAnimationData.baseIconPath);

    //Setup the movable icons
    if(mAnimationData.movableIconPaths.size() > 0)
    {
        for(int i=0; i<mAnimationData.movableIconPaths.size(); ++i)
        {
            setupAnimationMovable(i);
            if(unanimatedComponent->getPort(mAnimationData.dataPorts.at(i))->isConnected())
            {
                mpData->insert(i,mpAnimationWidget->getPlotDataPtr()->at(mpAnimationWidget->getNumberOfPlotGenerations()-1).find(unanimatedComponent->getName()).value().find(mAnimationData.dataPorts.at(i)).value().find(mAnimationData.dataNames.at(i)).value().second);
            }
        }
    }

    //Draw itself to the scene
    draw();
}


//! @brief Draws the animated component to the scene of the parent animation widget
void AnimatedComponent::draw()
{
    //Add the base icon to the scene
    mpAnimationWidget->getGraphicsScene()->addItem(mpBase);

    //Add the movable icons to the scene
    if(mpMovables.size() > 0)
    {
        for(int m=0; m<mpMovables.size(); ++m)
        {
            mpAnimationWidget->getGraphicsScene()->addItem(mpMovables.at(m));
        }
    }
}


//! @brief Updates the animation of the component
void AnimatedComponent::update()
{
    int a=0;    //Adjustables use a different indexing, because all movables are not adjustable

    //Loop through all movable icons
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationWidget->isRealTimeAnimation() && mAnimationData.isAdjustable.at(m))   //Adjustable icon, write node data depending on position
        {
            double value = mpMovables.at(m)->x()*mAnimationData.adjustableGainX.at(a) + mpMovables.at(m)->y()*mAnimationData.adjustableGainY.at(a);
            mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->writeNodeData(mpModelObject->getName(), mAnimationData.adjustablePort.at(a), mAnimationData.adjustableDataName.at(a), value);
            ++a;
        }
        else        //Not adjustable, so let's move it
        {
            double data;
            if(mpData->isEmpty())       //No data (port is not connected)
            {
                return;
            }
            else
            {
                if(mpAnimationWidget->isRealTimeAnimation())    //Real-time simulation, read from node vector directly
                {
                    if(mpModelObject->getPort(mAnimationData.dataPorts.at(m))->isConnected())
                        mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(mpModelObject->getName(), mAnimationData.dataPorts.at(m), mAnimationData.dataNames.at(m), data);
                    else
                        data=0;
                }
                else                                //Not real-time, so read from predefined data member object
                {
                    data = mpData->at(m).at(mpAnimationWidget->getIndex());
                }
            }

            //Apply parameter multipliers/divisors
            if(mAnimationData.multipliers[m] != QString())
            {
                data = data*mpModelObject->getParameterValue(mAnimationData.multipliers[m]).toDouble();
            }
            if(mAnimationData.divisors[m] != QString())
            {
                data = data/mpModelObject->getParameterValue(mAnimationData.divisors[m]).toDouble();
            }

            //Set rotation
            if(mAnimationData.speedTheta[m] != 0.0)
            {
                double rot = mAnimationData.startTheta[m] - data*mAnimationData.speedTheta[m];
                mpMovables[m]->setRotation(rot);
            }

            //Set position
            if(mAnimationData.speedX[m] != 0.0 || mAnimationData.speedY[m] != 0.0)
            {
                double x = mAnimationData.startX[m] - data*mAnimationData.speedX[m];
                double y = mAnimationData.startY[m] - data*mAnimationData.speedY[m];
                mpMovables[m]->setPos(x, y);
            }
        }
    }
}


ModelObjectAnimationData *AnimatedComponent::getAnimationDataPtr()
{
    return &mAnimationData;
}


int AnimatedComponent::indexOfMovable(AnimatedIcon *pMovable)
{
    return mpMovables.indexOf(pMovable);
}


//! @brief Creates the animation base icon
//! @param [in] basePath Path to the icon file
void AnimatedComponent::setupAnimationBase(QString basePath)
{
    ModelObjectAppearance *baseAppearance = new ModelObjectAppearance();
    if(mAnimationData.baseIconPath.isEmpty())
    {
        mAnimationData.baseIconPath = mpModelObject->getAppearanceData()->getIconPath(USERGRAPHICS, ABSOLUTE);
        if(mAnimationData.baseIconPath.isEmpty())
        {
            mAnimationData.baseIconPath = mpModelObject->getAppearanceData()->getIconPath(ISOGRAPHICS, ABSOLUTE);
        }
        baseAppearance->setIconPath(mAnimationData.baseIconPath, USERGRAPHICS, ABSOLUTE);
    }
    else
    {
        baseAppearance->setIconPath(basePath, USERGRAPHICS, RELATIVE);
    }
    mpBase = new AnimatedIcon(mpModelObject->pos(),mpModelObject->rotation(),baseAppearance,this,0,0);
    mpAnimationWidget->getGraphicsScene()->addItem(mpBase);
    if(mpModelObject->isFlipped())
    {
        mpBase->flipHorizontal();
    }
    mpBase->setCenterPos(mpModelObject->getCenterPos());
}


//! @brief Creates a movable icon
//! @param [in] m Index of icon to create
void AnimatedComponent::setupAnimationMovable(int m)
{
    ModelObjectAppearance* pAppearance = new ModelObjectAppearance();
    pAppearance->setIconPath(mAnimationData.movableIconPaths[m],USERGRAPHICS, RELATIVE);
    this->mpMovables.append(new AnimatedIcon(QPoint(0,0),0, pAppearance,this, 0, mpBase));
    this->mpMovables.at(m)->setTransformOriginPoint(mAnimationData.transformOriginX[m],mAnimationData.transformOriginY[m]);

    mpMovables.at(m)->setRotation(mAnimationData.startTheta.at(m));
    mpMovables.at(m)->setPos(mAnimationData.startX.at(m), mAnimationData.startY.at(m));

    //Set icon to be movable by mouse if it shall be adjustable
    mpMovables.at(m)->setFlag(QGraphicsItem::ItemIsMovable, mAnimationData.isAdjustable.at(m));
}


//! @brief Limits the position of movables that are adjustable (can be moved by mouse)
void AnimatedComponent::limitMovables()
{
    int a=0;
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mAnimationData.isAdjustable.at(m))
        {
            if(mpMovables.at(m)->x() > mAnimationData.adjustableMaxX.at(a))
            {
                mpMovables.at(m)->setX(mAnimationData.adjustableMaxX.at(a));
            }
            else if(mpMovables.at(m)->x() < mAnimationData.adjustableMinX.at(a))
            {
                mpMovables.at(m)->setX(mAnimationData.adjustableMinX.at(a));
            }
            else if(mpMovables.at(m)->y() > mAnimationData.adjustableMaxY.at(a))
            {
                mpMovables.at(m)->setY(mAnimationData.adjustableMaxY.at(a));
            }
            else if(mpMovables.at(m)->y() < mAnimationData.adjustableMinY.at(a))
            {
                mpMovables.at(m)->setY(mAnimationData.adjustableMinY.at(a));
            }
        }

        ++a;
    }
}



//! @brief Creator for the animated icon class
//! @param [in] position Initial position of icon
//! @param [in] rotation Initial rotation of icon
//! @param [in] pAppearanceData Pointer to appearance data object
//! @param [in] pAnimatedComponent Pointer to animated component icon belongs to
//! @param [in] pParentContainer Pointer to container object animation is showing
//! @param [in] pParent Parent object (QGraphicsItem), used for the coordinate system
AnimatedIcon::AnimatedIcon(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, DESELECTED, pParentContainer, pParent)
{

    //Store original position
    mOldPos = position;

    //Initialize member pointer variables
    mpAnimatedComponent = pAnimatedComponent;
    mpIcon = 0;

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
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int AnimatedIcon::type() const
{
    return Type;
}


//! @brief Updates the icon of the object to user or iso style
//! @param gfxType Graphics type that shall be used
//! @todo This code can probably be simplified a lot for animated icons
void AnimatedIcon::setIcon()
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


//! @brief Refresh icon position after flipping or rotating
void AnimatedIcon::refreshIconPosition()
{
    //Only move when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setPos( this->mapFromScene(this->getCenterPos() - mpIcon->boundingRect().center() ));
    }
}


//! @brief Set icon zoom factor
//! @todo Can maybe be removed from animated icon class
void AnimatedIcon::setIconZoom(const qreal zoom)
{
    //Only scale when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setScale(mModelObjectAppearance.getIconScale(mIconType)*zoom);
    }
}



//! @brief Defines what happens when object position has changed (limits the position to maximum values)
//! @param change Tells what it is that has changed
QVariant AnimatedIcon::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged && this->scene() != 0)
    {
        mpAnimatedComponent->limitMovables();
    }

    return value;
}


void AnimatedIcon::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mpAnimatedComponent->indexOfMovable(this) > -1)  //Otherwise this is the base icon, which does not have parameters
    {
        AnimatedIconPropertiesDialog *pDialog = new AnimatedIconPropertiesDialog(mpAnimatedComponent, mpAnimatedComponent->indexOfMovable(this), gpMainWindow);
        pDialog->exec();
        delete(pDialog);
    }

    QGraphicsWidget::mouseDoubleClickEvent(event);
}


//! @brief Slot that rotates the icon
//! @param [in] angle Angle to rotate (degrees)
void AnimatedIcon::rotate(qreal angle)
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
void AnimatedIcon::flipVertical()
{
    this->flipHorizontal();
    this->rotate(180);
}


//! @brief Slot that flips the object horizontally
//! @see flipVertical()
void AnimatedIcon::flipHorizontal()
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
ModelObjectAppearance* AnimatedIcon::getAppearanceData()
{
    return &mModelObjectAppearance;
}


//! @brief Refreshes the appearance of the icon
void AnimatedIcon::refreshAppearance()
{
    //! @todo should make sure we only do this if we really need to resize (after icon change)
    QPointF centerPos =  this->getCenterPos(); //Remember center pos for resize
    this->setIcon();
    this->setCenterPos(centerPos); //Re-set center pos after resize
}


//! @brief Returns a pointer to the icon object
QGraphicsSvgItem *AnimatedIcon::getIcon()
{
    return mpIcon;
}
