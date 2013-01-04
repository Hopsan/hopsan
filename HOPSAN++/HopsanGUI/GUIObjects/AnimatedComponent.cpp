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
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/ProjectTabWidget.h"



//! @brief Constructor for the animated component class
AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, AnimationWidget *parent)
    : QObject(parent /*parent*/)
{
    //Set member pointer variables
    mpAnimationWidget = parent;
    mpModelObject = unanimatedComponent;
    mpAnimationWidget = parent;
    mpData = new QList<QList<QVector<double> > >();
    mpNodeDataPtrs = new QList<QList<double *> >();

    //Store port positions
    for(int i=0; i<unanimatedComponent->getPortListPtrs().size(); ++i)
    {
        Port *pPort = unanimatedComponent->getPortListPtrs().at(i);
        mPortPositions.insert(pPort->getPortName(), pPort->getCenterPos());
    }

    //Set the animation data object
    mpAnimationData = unanimatedComponent->getAppearanceData()->getAnimationDataPtr();

    //Setup the base icon
    setupAnimationBase(mpAnimationData->baseIconPath);

    //Setup the movable icons
    if(mpAnimationData->movableIconPaths.size() > 0)
    {
        for(int i=0; i<mpAnimationData->movableIconPaths.size(); ++i)
        {
            setupAnimationMovable(i);
            if(!mpAnimationData->dataPorts.at(i).isEmpty() && unanimatedComponent->getPort(mpAnimationData->dataPorts.at(i).first())->isConnected())
            {
                mpData->append(QList<QVector<double> >());
                mpNodeDataPtrs->append(QList<double*>());
                //! @todo generation info should be some kind of "propertie" for alla of the animation code sp that if you change it it should change everywhere, to make it possible to animate different generations
                int generations = mpAnimationWidget->mpContainer->getLogDataHandler()->getLatestGeneration();
                QString componentName = unanimatedComponent->getName();
                for(int j=0; j<mpAnimationData->dataPorts.at(i).size(); ++j)
                {
                    QString portName = mpAnimationData->dataPorts.at(i).at(j);
                    QString dataName = mpAnimationData->dataNames.at(i).at(j);

                    if(!mpAnimationWidget->getPlotDataPtr()->isEmpty())
                    {
                        mpData->last().append(mpAnimationWidget->getPlotDataPtr()->getPlotDataValues(generations, componentName, portName, dataName));
                    }
                    mpNodeDataPtrs->last().append(mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(componentName, portName, dataName));
                    //qDebug() << "mpData = " << *mpData;
                }
            }
        }
    }

    if(mpModelObject->getSubTypeName() == "XmasSky")
    {
        mpBase->setZValue(mpBase->zValue()-1);
    }
    if(mpModelObject->getSubTypeName() == "XmasSnow")
    {
        mpBase->setZValue(mpBase->zValue()+2);
    }
    if(mpModelObject->getSubTypeName() == "XmasSnowFlake")
    {
        mpBase->setZValue(mpBase->zValue()+1);
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
void AnimatedComponent::updateAnimation()
{
    int a=0;    //Adjustables use a different indexing, because all movables are not adjustable

    if(mpModelObject->getTypeName() == "SignalDisplay")
    {
        double textData;
        mpModelObject->getPort("in")->getLastNodeData("Value", textData);
        mpText->setPlainText(QString::number(textData,'g', 4));
    }

    //Loop through all movable icons
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationWidget->isRealTimeAnimation() && mpAnimationData->isAdjustable.at(m) && mpAnimationData->adjustableGainX.size() > a)   //Adjustable icon, write node data depending on position
        {
            double value = mpMovables.at(m)->x()*mpAnimationData->adjustableGainX.at(a) + mpMovables.at(m)->y()*mpAnimationData->adjustableGainY.at(a);
            mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->writeNodeData(mpModelObject->getName(), mpAnimationData->adjustablePort.at(a), mpAnimationData->adjustableDataName.at(a), value);
            ++a;
        }
        else        //Not adjustable, so let's move it
        {
            QList<double> data;

            if(mpAnimationWidget->isRealTimeAnimation() && !mpNodeDataPtrs->isEmpty())    //Real-time simulation, read from node vector directly
            {
                if(mpModelObject->getPort(mpAnimationData->dataPorts.at(m).first())->isConnected())
                {
                    for(int i=0; i<mpNodeDataPtrs->at(m).size(); ++i)
                    {
                        data.append(*mpNodeDataPtrs->at(m).at(i));
                    }
                    data.append(0);
                    //mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(mpModelObject->getName(), mpAnimationData->dataPorts.at(m), mpAnimationData->dataNames.at(m), data);
                }
                else
                {
                    data.append(0);
                }
            }
            else if(!mpData->isEmpty())                                //Not real-time, so read from predefined data member object
            {
                for(int i=0; i<mpData->at(m).size(); ++i)
                {
                    data.append(mpData->at(m).at(i).at(mpAnimationWidget->getIndex()));
                }
            }

            if(data.isEmpty())
            {
                continue;
            }

            //Apply parameter multipliers/divisors
            if(mpAnimationData->multipliers[m] != QString())    //! @todo Multipliers and divisors currently only work for first data
            {
                data[0] = data[0]*mpModelObject->getParameterValue(mpAnimationData->multipliers[m]).toDouble();
            }
            if(mpAnimationData->divisors[m] != QString())
            {
                data[0] = data[0]/mpModelObject->getParameterValue(mpAnimationData->divisors[m]).toDouble();
            }

            //Set rotation
            if(mpAnimationData->speedTheta[m] != 0.0)
            {
                double rot = mpAnimationData->startTheta[m] - data[0]*mpAnimationData->speedTheta[m];
                mpMovables[m]->setRotation(rot);
            }

            //Set position
            if(mpAnimationData->speedX[m] != 0.0 || mpAnimationData->speedY[m] != 0.0)
            {
                double x = mpAnimationData->startX[m] - data[0]*mpAnimationData->speedX[m];
                double y = mpAnimationData->startY[m] - data[0]*mpAnimationData->speedY[m];
                mpMovables[m]->setPos(x, y);
            }

            //Set scale
            if(mpAnimationData->resizeX[m] != 0.0 || mpAnimationData->resizeY[m] != 0.0)
            {
                int idx1 = mpAnimationData->scaleDataIdx1[m];
                int idx2 = mpAnimationData->scaleDataIdx2[m];
                double scaleData;
                if(idx1 != idx2)
                {
                    scaleData = data[idx1]-data[idx2];
                }
                else
                {
                    scaleData = data[idx1];
                }
                double scaleX = mpAnimationData->resizeX[m]*scaleData;
                double scaleY = mpAnimationData->resizeY[m]*scaleData;
                double initX = mpAnimationData->initScaleX[m];
                double initY = mpAnimationData->initScaleY[m];
                mpMovables[m]->resetTransform();
                mpMovables[m]->scale(initX-scaleX, initY-scaleY);
            }

            //Set color
            if(mpAnimationData->colorR[m] != 0.0 || mpAnimationData->colorG[m] != 0.0 || mpAnimationData->colorB[m] != 0.0 || mpAnimationData->colorA[m] != 0.0)
            {
                int idx = mpAnimationData->colorDataIdx[m];

                int ir = mpAnimationData->initColorR[m];
                int r=ir;
                if(mpAnimationData->colorR[m] != 0)
                {
                    r = std::max(0, std::min(255, ir-int(mpAnimationData->colorR[m]*data[idx])));
                }

                int ig = mpAnimationData->initColorG[m];
                int g = ig;
                if(mpAnimationData->colorG[m] != 0)
                {
                    g = std::max(0, std::min(255, ig-int(mpAnimationData->colorG[m]*data[idx])));
                }

                int ib = mpAnimationData->initColorB[m];
                int b = ib;
                if(mpAnimationData->colorB[m] != 0)
                {
                    b = std::max(0, std::min(255, ib-int(mpAnimationData->colorB[m]*data[idx])));
                }

                int ia = mpAnimationData->initColorA[m];
                if(ia == 0)
                {
                    ia = 255;
                }
                int a = ia;
                if(mpAnimationData->colorA[m] != 0)
                {
                    a = std::max(0, std::min(255, ia-int(mpAnimationData->colorA[m]*data[idx])));
                }


                QGraphicsColorizeEffect *pEffect = new QGraphicsColorizeEffect();
                QColor color(r,g,b,a);
             //   qDebug() << "Color: " << r << " " << g << " " << b << " " << a;
                pEffect->setColor(color);
                mpMovables[m]->setGraphicsEffect(pEffect);
                mpMovables[m]->setOpacity(double(a)/255.0);
            }


            mpMovables[m]->update();

            //Update "port" positions, so that connectors will follow component
            for(int p=0; p<mpAnimationData->movablePortNames[m].size(); ++p)
            {
                QString portName = mpAnimationData->movablePortNames[m][p];
                double portStartX = mpAnimationData->movablePortStartX[m][p];
                double portStartY = mpAnimationData->movablePortStartY[m][p];

                QPointF pos;
                pos.setX(portStartX-data[0]*mpAnimationData->speedX[m]);
                pos.setY(portStartY-data[0]*mpAnimationData->speedY[m]);
                mPortPositions.insert(portName, pos);
            }
        }
    }
}


//! @brief Returns a pointer to the animation data object
ModelObjectAnimationData *AnimatedComponent::getAnimationDataPtr()
{
    return mpAnimationData;
}


//! @brief Returns the index of a movable icon
//! @param [in] pMovable Pointer to movable icon
int AnimatedComponent::indexOfMovable(AnimatedIcon *pMovable)
{
    return mpMovables.indexOf(pMovable);
}


QPointF AnimatedComponent::getPortPos(QString portName)
{
    return mPortPositions.find(portName).value();
}


//! @brief Creates the animation base icon
//! @param [in] basePath Path to the icon file
void AnimatedComponent::setupAnimationBase(QString basePath)
{
    ModelObjectAppearance *baseAppearance = new ModelObjectAppearance();
    if(mpAnimationData->baseIconPath.isEmpty())
    {
        mpAnimationData->baseIconPath = mpModelObject->getAppearanceData()->getIconPath(USERGRAPHICS, ABSOLUTE);
        if(mpAnimationData->baseIconPath.isEmpty())
        {
            mpAnimationData->baseIconPath = mpModelObject->getAppearanceData()->getIconPath(ISOGRAPHICS, ABSOLUTE);
        }
        baseAppearance->setIconPath(mpAnimationData->baseIconPath, USERGRAPHICS, ABSOLUTE);
    }
    else
    {
        baseAppearance->setIconPath(basePath, USERGRAPHICS, RELATIVE);
    }
    mpBase = new AnimatedIcon(mpModelObject->pos(),0,baseAppearance,this,0,0);
    mpAnimationWidget->getGraphicsScene()->addItem(mpBase);
    if(mpModelObject->isFlipped())
    {
        mpBase->flipHorizontal();
    }
    mpBase->setRotation(mpModelObject->rotation());
    mpBase->setCenterPos(mpModelObject->getCenterPos());

    //Base icon shall never be movable
    mpBase->setFlag(QGraphicsItem::ItemIsMovable, false);

   // mpBase->refreshIconPosition();

    mpText = new QGraphicsTextItem(mpBase, 0);
    mpText->setPlainText("0");
    mpText->setFont(QFont("Arial", 16));
    mpText->setPos(7,0);
    if(mpModelObject->getTypeName() != "SignalDisplay")
    {
        mpText->hide();
    }
}


//! @brief Creates a movable icon
//! @param [in] m Index of icon to create
void AnimatedComponent::setupAnimationMovable(int m)
{
    ModelObjectAppearance* pAppearance = new ModelObjectAppearance();
    pAppearance->setIconPath(mpAnimationData->movableIconPaths[m],USERGRAPHICS, RELATIVE);
    int idx = mpAnimationData->movableIdx[m];
    QGraphicsItem *pBase = mpBase;
    if(mpAnimationData->movableRelatives[m] > -1)
    {
        for(int r=0; r<mpMovables.size(); ++r)
        {
            if(mpMovables[r]->mIdx == mpAnimationData->movableRelatives[m])
            {
                pBase = mpMovables[r];
            }
        }
    }
    this->mpMovables.append(new AnimatedIcon(QPoint(0,0),0, pAppearance,this, 0, idx, pBase));
    this->mpMovables.at(m)->setTransformOriginPoint(mpAnimationData->transformOriginX[m],mpAnimationData->transformOriginY[m]);

    mpMovables.at(m)->setRotation(mpAnimationData->startTheta.at(m));
    mpMovables.at(m)->setPos(mpAnimationData->startX.at(m), mpAnimationData->startY.at(m));

    //Set icon to be movable by mouse if it shall be adjustable
    mpMovables.at(m)->setFlag(QGraphicsItem::ItemIsMovable, mpAnimationData->isAdjustable.at(m));
}


//! @brief Limits the position of movables that are adjustable (can be moved by mouse)
void AnimatedComponent::limitMovables()
{
    int a=0;
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationData->isAdjustable.at(m) && mpAnimationData->adjustableGainX.size() > a)
        {
            if(mpMovables.at(m)->x() > mpAnimationData->adjustableMaxX.at(a))
            {
                mpMovables.at(m)->setX(mpAnimationData->adjustableMaxX.at(a));
            }
            else if(mpMovables.at(m)->x() < mpAnimationData->adjustableMinX.at(a))
            {
                mpMovables.at(m)->setX(mpAnimationData->adjustableMinX.at(a));
            }
            else if(mpMovables.at(m)->y() > mpAnimationData->adjustableMaxY.at(a))
            {
                mpMovables.at(m)->setY(mpAnimationData->adjustableMaxY.at(a));
            }
            else if(mpMovables.at(m)->y() < mpAnimationData->adjustableMinY.at(a))
            {
                mpMovables.at(m)->setY(mpAnimationData->adjustableMinY.at(a));
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
AnimatedIcon::AnimatedIcon(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent, ContainerObject *pParentContainer, int idx, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, DESELECTED, pParentContainer, pParent)
{

    //Store original position
    mOldPos = position;

    //Initialize member pointer variables
    mpAnimatedComponent = pAnimatedComponent;

    //Make a local copy of the appearance data (that can safely be modified if needed)
    mModelObjectAppearance = *pAppearanceData;

    //Setup appearance
    QString iconPath = mModelObjectAppearance.getFullAvailableIconPath(mIconType);
    double iconScale = mModelObjectAppearance.getIconScale(mIconType);
    mIconType = USERGRAPHICS;
    mpIcon = new QGraphicsSvgItem(iconPath, this);
    mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
    mpIcon->setScale(iconScale);

    this->prepareGeometryChange();
    this->resize(mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale);  //Resize modelobject
    mpSelectionBox->setSize(0.0, 0.0, mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale); //Resize selection box
    this->setCenterPos(position);

    this->setZValue(MODELOBJECT_Z);

    if(mpAnimatedComponent->mpModelObject->getSubTypeName() == "XmasSky")
    {
        this->setZValue(this->zValue()-1);
    }
    if(mpAnimatedComponent->mpModelObject->getSubTypeName() == "XmasSnow")
    {
        this->setZValue(this->zValue()+2);
    }
    if(mpAnimatedComponent->mpModelObject->getSubTypeName() == "XmasSnowFlake")
    {
        this->setZValue(this->zValue()+1);
    }

    mIdx = idx;
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int AnimatedIcon::type() const
{
    return Type;
}


//! @brief Refresh icon position after flipping or rotating
void AnimatedIcon::refreshIconPosition()
{
    mpIcon->setPos( this->mapFromScene(this->getCenterPos() - mpIcon->boundingRect().center() ));
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


//! @brief Defines what happens when icon is double clicked (open settings dialog)
//! @param event Contains information about the event
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


void AnimatedIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    int idx = mpAnimatedComponent->indexOfMovable(this);
    if(idx >= 0 && mpAnimatedComponent->getAnimationDataPtr()->isAdjustable.at(idx))
    {
        mpSelectionBox->setHovered();
    }

    QGraphicsWidget::hoverEnterEvent(event);
}


void AnimatedIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mpSelectionBox->setPassive();

    QGraphicsWidget::hoverLeaveEvent(event);
}


void AnimatedIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mousePressEvent(event);
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
    //refreshIconPosition();

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
