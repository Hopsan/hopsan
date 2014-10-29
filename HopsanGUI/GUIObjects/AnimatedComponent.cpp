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

#include <qmath.h>
#include <float.h>
#include <QGraphicsColorizeEffect>

#include "common.h"
#include "global.h"
#include "AnimatedComponent.h"
#include "CoreAccess.h"
#include "GraphicsView.h"
#include "GUIModelObject.h"
#include "GUIPort.h"
#include "../HopsanCore/include/Port.h"
#include "Dialogs/AnimatedIconPropertiesDialog.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/ModelWidget.h"



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
        mPortPositions.insert(pPort->getName(), pPort->getCenterPos());
    }

    //Set the animation data object
    mpAnimationData = unanimatedComponent->getAppearanceData()->getAnimationDataPtr();

    //Setup the base icon
    setupAnimationBase(mpAnimationData->baseIconPath);

    //Setup the movable icons
    if(!mpAnimationData->movables.isEmpty())
    {
        for(int i=0; i<mpAnimationData->movables.size(); ++i)
        {
            setupAnimationMovable(i);
            //if(!mpAnimationData->movables[i].dataPorts.isEmpty()/* && unanimatedComponent->getPort(mpAnimationData->dataPorts.at(i).first())->isConnected()*/)
            //{
                mpData->append(QList<QVector<double> >());
                mpNodeDataPtrs->append(QList<double*>());
                //! @todo generation info should be some kind of "propertie" for alla of the animation code sp that if you change it it should change everywhere, to make it possible to animate different generations
                int generation = mpAnimationWidget->mpContainer->getLogDataHandler()->getCurrentGeneration();
                QString componentName = unanimatedComponent->getName();
                for(int j=0; j<mpAnimationData->movables[i].dataPorts.size(); ++j)
                {
                    QString portName = mpAnimationData->movables[i].dataPorts.at(j);
                    QString dataName = mpAnimationData->movables[i].dataNames.at(j);

                    if(!mpAnimationWidget->getPlotDataPtr()->isEmpty())
                    {
                        QString tempComponentName = componentName;
                        QString tempPortName = portName;
                        if(mpModelObject->getPort(portName) && mpModelObject->getPort(portName)->getPortType().contains("Multiport"))
                        {
                            tempComponentName = mpModelObject->getPort(portName)->getConnectedPorts().first()->getParentModelObjectName();
                            tempPortName = mpModelObject->getPort(portName)->getConnectedPorts().first()->getName();
                        }
                        mpData->last().append(mpAnimationWidget->getPlotDataPtr()->copyVariableDataVector(makeConcatName(tempComponentName, tempPortName, dataName),generation));
                    }
                    mpNodeDataPtrs->last().append(mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(componentName, portName, dataName));
                    if(!mpNodeDataPtrs->last().last())
                    {
                        mpNodeDataPtrs->last().last() = new double(0);
                    }
                    //qDebug() << "mpData = " << *mpData;
                }
            //}
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

    if(mIsDisplay)
    {
        double textData;
        mpModelObject->getPort("in")->getLastNodeData("Value", textData);
        QString text = QString::number(textData,'g', 4);
        text = text.left(6);
        mpText->setPlainText(text);
    }

    //Loop through all movable icons
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationWidget->isRealTimeAnimation() && mpAnimationData->movables[m].isAdjustable)
        {
            double value = mpMovables.at(m)->x()*mpAnimationData->movables[m].adjustableGainX + mpMovables.at(m)->y()*mpAnimationData->movables[m].adjustableGainY;
            (*mpMovables.at(m)->mpAdjustableNodeDataPtr) = value;
            ++a;
        }
        else        //Not adjustable, so let's move it
        {
            QList<double> data;

            if(mpAnimationWidget->isRealTimeAnimation() && !mpNodeDataPtrs->isEmpty())    //Real-time simulation, read from node vector directly
            {
                if(true/*mpModelObject->getPort(mpAnimationData->dataPorts.at(m).first())->isConnected()*/)
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
                    if(!mpData->at(m).at(i).isEmpty())
                    {
                        data.append(mpData->at(m).at(i).at(mpAnimationWidget->getIndex()));
                    }
                    else
                    {
                        data.append(0);
                    }
                }
            }

            if(data.isEmpty())
            {
                continue;
            }

            //Set position and rotation
            double x = mpAnimationData->movables[m].startX;
            double y = mpAnimationData->movables[m].startY;
            double rot = mpAnimationData->movables[m].startTheta;
            for(int j=0; j<mpAnimationData->movables[m].movementX.size(); ++j)
            {
                int idx = mpAnimationData->movables[m].movementDataIdx[j];
                x -= data[idx]*mpAnimationData->movables[m].movementX[j];
                y -= data[idx]*mpAnimationData->movables[m].movementY[j];
                rot -= data[idx]*mpAnimationData->movables[m].movementTheta[j];
            }

            //Apply parameter multipliers/divisors
            if(mpAnimationData->movables[m].useMultipliers)    //! @todo Multipliers and divisors currently only work for first data
            {
                x *= mpAnimationData->movables[m].multiplierValue;
                y *= mpAnimationData->movables[m].multiplierValue;
                rot *= mpAnimationData->movables[m].multiplierValue;
            }
            if(mpAnimationData->movables[m].useDivisors)
            {
                x /= mpAnimationData->movables[m].divisorValue;
                y /= mpAnimationData->movables[m].divisorValue;
                rot /= mpAnimationData->movables[m].divisorValue;
            }

            //Apply new position
            mpMovables[m]->setPos(x, y);
            mpMovables[m]->setRotation(rot);

            //Set scale
            if(!mpAnimationData->movables[m].resizeX.isEmpty())
            {
                double totalScaleX = 1;
                double totalScaleY = 1;

                bool xChanged = false;
                bool yChanged = false;
                for(int r=0; r<mpAnimationData->movables[m].resizeX.size(); ++r)
                {
                    int idx1 = mpAnimationData->movables[m].scaleDataIdx1[r];
                    int idx2 = mpAnimationData->movables[m].scaleDataIdx2[r];
                    double scaleData;
                    if(idx1 != idx2 && idx2 > 0)
                    {
                        scaleData = data[idx1]-data[idx2];
                    }
                    else
                    {
                        scaleData = data[idx1];
                    }

                    if(mpAnimationData->movables[m].resizeX[r] != 0)
                    {
                        double scaleX = mpAnimationData->movables[m].resizeX[r]*scaleData;
                        totalScaleX *= scaleX;
                        xChanged = true;
                    }

                    if(mpAnimationData->movables[m].resizeY[r] != 0)
                    {
                        double scaleY = mpAnimationData->movables[m].resizeY[r]*scaleData;
                        totalScaleY *= scaleY;
                        yChanged = true;
                    }
                }
                if(!xChanged)
                    totalScaleX = 0;
                if(!yChanged)
                    totalScaleY = 0;
                double initX = mpAnimationData->movables[m].initScaleX;
                double initY = mpAnimationData->movables[m].initScaleY;
                totalScaleX = initX - totalScaleX;
                totalScaleY = initY - totalScaleY;

                mpMovables[m]->resetTransform();
                //mpMovables[m]->scale(initX-scaleX, initY-scaleY);
                mpMovables[m]->setTransform(QTransform::fromScale(totalScaleX, totalScaleY), true);
            }

            //Set color
            if(mpAnimationData->movables[m].colorR != 0.0 || mpAnimationData->movables[m].colorG != 0.0 || mpAnimationData->movables[m].colorB != 0.0 || mpAnimationData->movables[m].colorA != 0.0)
            {
                int idx = mpAnimationData->movables[m].colorDataIdx;

                int ir = mpAnimationData->movables[m].initColorR;
                int r=ir;
                if(mpAnimationData->movables[m].colorR != 0)
                {
                    r = std::max(0, std::min(255, ir-int(mpAnimationData->movables[m].colorR*data[idx])));
                }

                int ig = mpAnimationData->movables[m].initColorG;
                int g = ig;
                if(mpAnimationData->movables[m].colorG != 0)
                {
                    g = std::max(0, std::min(255, ig-int(mpAnimationData->movables[m].colorG*data[idx])));
                }

                int ib = mpAnimationData->movables[m].initColorB;
                int b = ib;
                if(mpAnimationData->movables[m].colorB != 0)
                {
                    b = std::max(0, std::min(255, ib-int(mpAnimationData->movables[m].colorB*data[idx])));
                }

                int ia = mpAnimationData->movables[m].initColorA;
                if(ia == 0)
                {
                    ia = 255;
                }
                int a = ia;
                if(mpAnimationData->movables[m].colorA != 0)
                {
                    a = std::max(0, std::min(255, ia-int(mpAnimationData->movables[m].colorA*data[idx])));
                }



                QColor color(r,g,b,a);
             //   qDebug() << "Color: " << r << " " << g << " " << b << " " << a;
                mpMovables[m]->mpEffect->setColor(color);
                mpMovables[m]->setGraphicsEffect(mpMovables[m]->mpEffect);
                mpMovables[m]->setOpacity(double(a)/255.0);
            }

            //Indicators
            if(mpAnimationData->movables[m].isIndicator)
            {
                double data = (*mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mpModelObject->getName(), mpAnimationData->movables[m].indicatorPort, mpAnimationData->movables[m].indicatorDataName));
                mpMovables[m]->setVisible(data > 0.5);
            }


           // mpMovables[m]->update();

            //Update "port" positions, so that connectors will follow component
            for(int p=0; p<mpAnimationData->movables[m].movablePortNames.size(); ++p)
            {
                QString portName = mpAnimationData->movables[m].movablePortNames[p];
                double portStartX = mpAnimationData->movables[m].movablePortStartX[p];
                double portStartY = mpAnimationData->movables[m].movablePortStartY[p];

                QPointF pos;
                pos.setX(portStartX+x);
                pos.setY(portStartY+y);
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

void AnimatedComponent::textEdited()
{
    disconnect(mpText->document(), SIGNAL(contentsChanged()), this, SLOT(textEdited()));

    QTextCursor cursor = mpText->textCursor();

    QString text = mpText->toPlainText();
    text.remove("\n");

    if(text.size() > 10)  //Limit value to box size
    {
        text.chop(1);
        mpText->setPlainText(text);
//        QTextCursor cursor = mpText->textCursor();
//        cursor.movePosition(QTextCursor::EndOfLine);
//        mpText->setTextCursor(cursor);
    }

    mpText->setPlainText(text);
    if(text.toDouble())
    {
        qDebug() << "Double value: " << text.toDouble();
        if(mpModelObject->getPort("out"))
        {
            mpModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->writeNodeData(mpModelObject->getName(), "out", "Value", text.toDouble());
        }
    }
    else
    {
        qDebug() << "Illegal value: " << text;
    }

    //cursor.movePosition(QTextCursor::EndOfLine);
    mpText->setTextCursor(cursor);

    connect(mpText->document(), SIGNAL(contentsChanged()), this, SLOT(textEdited()));
}


//! @brief Creates the animation base icon
//! @param [in] basePath Path to the icon file
void AnimatedComponent::setupAnimationBase(QString basePath)
{
    ModelObjectAppearance *baseAppearance = new ModelObjectAppearance();
    if(mpAnimationData->baseIconPath.isEmpty())
    {
        mpAnimationData->baseIconPath = mpModelObject->getAppearanceData()->getIconPath(UserGraphics, Absolute);
        if(mpAnimationData->baseIconPath.isEmpty())
        {
            mpAnimationData->baseIconPath = mpModelObject->getAppearanceData()->getIconPath(ISOGraphics, Absolute);
        }
        if(mpAnimationData->baseIconPath.isEmpty())
        {
            mpAnimationData->baseIconPath = mpModelObject->getAppearanceData()->getDefaultMissingIconPath();
        }
        baseAppearance->setIconPath(mpAnimationData->baseIconPath, UserGraphics, Absolute);
    }
    else
    {
        baseAppearance->setIconPath(basePath, UserGraphics, Relative);
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

    mpText = new QGraphicsTextItem(mpBase);
    mpText->setPlainText("0");
    mpText->setFont(QFont("Arial", 16));
    mpText->setPos(7,0);
    mpText->hide();

    mIsDisplay = (mpModelObject->getTypeName() == "SignalDisplay");
    mIsNumericalInput = (mpModelObject->getTypeName() == "SignalNumericalInput");
    if(mIsNumericalInput)
    {
        mpText->setTextInteractionFlags(Qt::TextEditorInteraction);
        mpText->show();
        connect(mpText->document(), SIGNAL(contentsChanged()), this, SLOT(textEdited()));
    }
    if(mIsDisplay)
    {
        mpText->setDefaultTextColor(Qt::green);
        mpText->show();
    }
}


//! @brief Creates a movable icon
//! @param [in] m Index of icon to create
void AnimatedComponent::setupAnimationMovable(int m)
{
    ModelObjectAppearance* pAppearance = new ModelObjectAppearance();
    pAppearance->setIconPath(mpAnimationData->movables[m].iconPath,UserGraphics, Relative);
    int idx = mpAnimationData->movables[m].idx;
    QGraphicsItem *pBase = mpBase;
    if(mpAnimationData->movables[m].movableRelative > -1)
    {
        for(int r=0; r<mpMovables.size(); ++r)
        {
            if(mpMovables[r]->mIdx == mpAnimationData->movables[m].movableRelative)
            {
                pBase = mpMovables[r];
            }
        }
    }
    this->mpMovables.append(new AnimatedIcon(QPoint(0,0),0, pAppearance,this, 0, idx, pBase));
    this->mpMovables.at(m)->setTransformOriginPoint(mpAnimationData->movables[m].transformOriginX,mpAnimationData->movables[m].transformOriginY);

    mpMovables.at(m)->setRotation(mpAnimationData->movables[m].startTheta);
    mpMovables.at(m)->setPos(mpAnimationData->movables[m].startX, mpAnimationData->movables[m].startY);

    //Set adjustables to non-adjustable if the port is connected
    if(mpAnimationData->movables[m].isAdjustable)
    {
        QString port = mpAnimationData->movables[m].adjustablePort;
        if(mpModelObject->getPort(port)->getPortType() != "WritePortType" && mpModelObject->getPort(port)->isConnected())
        {
            mpAnimationData->movables[m].isAdjustable = false;
        }
    }

    //Set icon to be movable by mouse if it shall be adjustable
    mpMovables.at(m)->setFlag(QGraphicsItem::ItemIsMovable, mpAnimationData->movables[m].isAdjustable);

    if(mpAnimationData->movables[m].isSwitchable)
    {
        mpMovables.at(m)->hide();
    }




    //Get parameter multiplier (will be 1 if not found)
    double multiplierValue = 1;
    for(int p=0; p<mpAnimationData->movables[m].multipliers.size(); ++p)
    {
        if(mpAnimationData->movables[m].multipliers[p].isEmpty())
            continue;

        QString parValue = mpModelObject->getParameterValue(mpAnimationData->movables[m].multipliers[p]);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentContainerObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentContainerObject()->getParameterValue(parValue).toDouble(&ok);
        }
        multiplierValue *= temp;
    }
    mpAnimationData->movables[m].multiplierValue = multiplierValue;
    mpAnimationData->movables[m].useMultipliers = !mpAnimationData->movables[m].multipliers.isEmpty();


    //Get parmeter divisor (will be 1 if not found)
    double divisorValue = 1;
    for(int p=0; p<mpAnimationData->movables[m].divisors.size(); ++p)
    {
        if(mpAnimationData->movables[m].divisors[p].isEmpty())
            continue;

        QString parValue = mpModelObject->getParameterValue(mpAnimationData->movables[m].divisors[p]);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentContainerObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentContainerObject()->getParameterValue(parValue).toDouble(&ok);
        }
        divisorValue *= temp;
    }
    mpAnimationData->movables[m].divisorValue = divisorValue;
    mpAnimationData->movables[m].useDivisors = !mpAnimationData->movables[m].divisors.isEmpty();
}


//! @brief Limits the position of movables that are adjustable (can be moved by mouse)
void AnimatedComponent::limitMovables()
{
    for(int m=0; m<mpMovables.size(); ++m)
    {
        if(mpAnimationData->movables[m].isAdjustable)
        {
            if(mpMovables.at(m)->x() > mpAnimationData->movables[m].adjustableMaxX)
            {
                mpMovables.at(m)->setX(mpAnimationData->movables[m].adjustableMaxX);
            }
            else if(mpMovables.at(m)->x() < mpAnimationData->movables[m].adjustableMinX)
            {
                mpMovables.at(m)->setX(mpAnimationData->movables[m].adjustableMinX);
            }
            else if(mpMovables.at(m)->y() > mpAnimationData->movables[m].adjustableMaxY)
            {
                mpMovables.at(m)->setY(mpAnimationData->movables[m].adjustableMaxY);
            }
            else if(mpMovables.at(m)->y() < mpAnimationData->movables[m].adjustableMinY)
            {
                mpMovables.at(m)->setY(mpAnimationData->movables[m].adjustableMinY);
            }
        }
    }
}



//! @brief Creator for the animated icon class
//! @param [in] position Initial position of icon
//! @param [in] rotation Initial rotation of icon
//! @param [in] pAppearanceData Pointer to appearance data object
//! @param [in] pAnimatedComponent Pointer to animated component icon belongs to
//! @param [in] pParentContainer Pointer to container object animation is showing
//! @param [in] pParent Parent object (QGraphicsItem), used for the coordinate system
AnimatedIcon::AnimatedIcon(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent, ContainerObject *pParentContainer, int idx, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, Deselected, pParentContainer, pParent)
{
    //Store original position
    mPreviousPos = position;

    //Initialize member pointer variables
    mpAnimatedComponent = pAnimatedComponent;

    //Make a local copy of the appearance data (that can safely be modified if needed)
    mModelObjectAppearance = *pAppearanceData;

    //Setup appearance
    QString iconPath = mModelObjectAppearance.getFullAvailableIconPath(mIconType);
    double iconScale = mModelObjectAppearance.getIconScale(mIconType);
    mIconType = UserGraphics;
    mpIcon = new QGraphicsSvgItem(iconPath, this);
    mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
    mpIcon->setScale(iconScale);

    this->prepareGeometryChange();
    this->resize(mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale);  //Resize modelobject
    mpSelectionBox->setSize(0.0, 0.0, mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale); //Resize selection box
    this->setCenterPos(position);

    this->setZValue(ModelobjectZValue);

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

    this->setVisible(mpAnimatedComponent->mpModelObject->isVisible());


    ModelObjectAnimationData *pData = mpAnimatedComponent->getAnimationDataPtr();
    if(pParent != 0 && pData->movables[0].isAdjustable)
    {
        QString comp = mpAnimatedComponent->mpModelObject->getName();
        QString port = pData->movables[0].adjustablePort;
        QString dataName = pData->movables[0].adjustableDataName;
        mpAdjustableNodeDataPtr = mpAnimatedComponent->mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(comp, port, dataName);
    }

    mpEffect = new QGraphicsColorizeEffect();
}

void AnimatedIcon::loadFromDomElement(QDomElement domElement)
{
    Q_UNUSED(domElement)
    // Nothing
}

void AnimatedIcon::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    Q_UNUSED(rDomElement)
    Q_UNUSED(contents)
    // Nothing
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int AnimatedIcon::type() const
{
    return Type;
}

QString AnimatedIcon::getHmfTagName() const
{
    // These objeects are not present in hmf files, so nothing
    return QString();
}

void AnimatedIcon::deleteMe(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    // Does nothing
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
        AnimatedIconPropertiesDialog *pDialog = new AnimatedIconPropertiesDialog(mpAnimatedComponent, mpAnimatedComponent->indexOfMovable(this), gpMainWindowWidget);
        pDialog->exec();
        delete(pDialog);
    }

    QGraphicsWidget::mouseDoubleClickEvent(event);
}


void AnimatedIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    int idx = mpAnimatedComponent->indexOfMovable(this);
    if(idx >= 0 && mpAnimatedComponent->getAnimationDataPtr()->movables[idx].isAdjustable)
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
    if(!mpAnimatedComponent->mpMovables.isEmpty())
    {
        int idx = mIdx;//mpAnimatedComponent->indexOfMovable(this);
        if(idx < 0)
        {
            idx = 0;    //Not good, we assume there is only one movable and that it is the switch
        }

        ModelObjectAnimationData *pData = mpAnimatedComponent->getAnimationDataPtr();
        bool switchable = pData->movables[idx].isSwitchable;
        if(switchable)
        {
            //! @todo Don't do pointer lookup every time step!
            double *pNodeData = mpAnimatedComponent->mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mpAnimatedComponent->mpModelObject->getName(), pData->movables[mIdx].switchablePort, pData->movables[mIdx].switchableDataName);
            if(mpAnimatedComponent->mpMovables[idx]->isVisible())
            {
                mpAnimatedComponent->mpMovables[idx]->setVisible(false);
                (*pNodeData) = 0;
            }
            else
            {
                mpAnimatedComponent->mpMovables[idx]->setVisible(true);
                (*pNodeData) = 1;
            }
        }
    }

    QGraphicsWidget::mousePressEvent(event);
}


//! @brief Slot that rotates the icon
//! @param [in] angle Angle to rotate (degrees)
void AnimatedIcon::rotate(double angle, UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    if(mIsFlipped)
    {
        angle *= -1;
    }
    this->setRotation(normDeg360(this->rotation()+angle));

    refreshIconPosition();
}


//! @brief Slot that flips the object vertically
//! @see flipHorizontal()
void AnimatedIcon::flipVertical(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    this->flipHorizontal();
    this->rotate(180);
}


//! @brief Slot that flips the object horizontally
//! @see flipVertical()
void AnimatedIcon::flipHorizontal(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)

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
