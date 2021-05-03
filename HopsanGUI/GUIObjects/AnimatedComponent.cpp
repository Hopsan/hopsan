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
#include "MessageHandler.h"

#include "PlotHandler.h"
#include "PlotWindow.h"

namespace {

void updatePlotData(const QPointer<Port> &pPort, SharedVectorVariableT &pVariable) {
    if (!pPort.isNull()) {
        double data;
        pPort->getLastNodeData("Value", data);
        pVariable->append(data);
    }
};

void updatePlotData(const SharedVectorVariableT &pLogData, const double time, SharedVectorVariableT &pVariable) {
    if (!pLogData.isNull()) {
        auto pTimeVector = pLogData->getSharedTimeOrFrequencyVector();
        if (!pTimeVector.isNull()) {
            int index = pTimeVector->lower_bound(time, true);
            if (index >= 0) {
                const double data = pLogData->peekData(index);
                pVariable->append(data);
            }
        }
    }
};

}

//! @brief Constructor for the animated component class
AnimatedComponent::AnimatedComponent(ModelObject* unanimatedComponent, AnimationWidget *parent)
    : QObject(parent)
{
    //Set member pointer variables
    mpAnimationWidget = parent;
    mpModelObject = unanimatedComponent;
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
                //! @todo generation info should be some kind of "property" for all of the animation code so that if you change it, it should change everywhere, to make it possible to animate different generations
                int generation = mpAnimationWidget->mpContainer->getLogDataHandler()->getCurrentGenerationNumber();
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
                        mpData->last().append(mpAnimationWidget->getPlotDataPtr()->copyVariableDataVector(makeFullVariableName(mpModelObject->getParentSystemNameHieararchy(), tempComponentName, tempPortName, dataName),generation));

                        const auto& dataValues = mpData->last().last();
                        for(const auto& dataValue : dataValues)
                        {
                            if(!std::isfinite(dataValue))
                            {
                                mpAnimationWidget->disablePlayback();
                                gpMessageHandler->addErrorMessage("Model results contain Inf or NaN. Animation playback not available.");
                                return;
                            }
                        }
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
        textData *= mUnitScaling;
        QString text = QString::number(textData,'f', mPrecision);
        text = text.left(mPrecision);
        while(text.size() < mPrecision) {
            text.append("0");
        }
        text.prepend(mDescription);
        text.append(mUnit);
        mpText->setPlainText(text);
        mpText->setHtml(mHtml+ text + "</div>");
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
                            if(!std::isfinite(data.last()))
                            {
                                mpAnimationWidget->stop_reset();
                                gpMessageHandler->addErrorMessage("Encountered Inf or NaN value. Real-time animation aborted.");
                                return;
                            }
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
            for(const ModelObjectAnimationMovementData &movement : mpAnimationData->movables[m].movementData) {
                int idx = movement.dataIdx;
                x -= data[idx]*movement.x*movement.multiplierValue/movement.divisorValue;
                y -= data[idx]*movement.y*movement.multiplierValue/movement.divisorValue;
                rot -= data[idx]*movement.theta*movement.multiplierValue/movement.divisorValue;
            }

            //Apply parameter multipliers/divisors
//            if(mpAnimationData->movables[m].useMultipliers)    //! @todo Multipliers and divisors currently only work for first data
//            {
//                x *= mpAnimationData->movables[m].multiplierValue;
//                y *= mpAnimationData->movables[m].multiplierValue;
//                rot *= mpAnimationData->movables[m].multiplierValue;
//            }
//            if(mpAnimationData->movables[m].useDivisors)
//            {
//                x /= mpAnimationData->movables[m].divisorValue;
//                y /= mpAnimationData->movables[m].divisorValue;
//                rot /= mpAnimationData->movables[m].divisorValue;
//            }

            //Apply new position
            mpMovables[m]->setPos(x, y);
            mpMovables[m]->setRotation(rot);

            //Set scale
            if(!mpAnimationData->movables[m].resizeData.isEmpty())
            {
                double totalScaleX = 1;
                double totalScaleY = 1;

                bool xChanged = false;
                bool yChanged = false;
                for(const ModelObjectAnimationResizeData &resize : mpAnimationData->movables[m].resizeData) {
                    int idx1 = resize.dataIdx1;
                    int idx2 = resize.dataIdx2;
                    double scaleData;
                    if(idx1 != idx2 && idx2 > 0)
                    {
                        scaleData = data[idx1]+data[idx2];
                    }
                    else
                    {
                        scaleData = data[idx1];
                    }

                    if(resize.x != 0)
                    {
                        double scaleX = resize.x*scaleData;
                        totalScaleX *= scaleX*resize.multiplierValue/resize.divisorValue;
                        xChanged = true;
                    }

                    if(resize.y != 0)
                    {
                        double scaleY = resize.y*scaleData;
                        totalScaleY *= scaleY*resize.multiplierValue/resize.divisorValue;
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
            if(mpAnimationData->movables[m].colorData.r != 0.0 || mpAnimationData->movables[m].colorData.g != 0.0 || mpAnimationData->movables[m].colorData.b != 0.0 || mpAnimationData->movables[m].colorData.a != 0.0)
            {
                int idx = mpAnimationData->movables[m].colorData.dataIdx;
                double div = mpAnimationData->movables[m].colorData.divisorValue;
                double mul = mpAnimationData->movables[m].colorData.multiplierValue;

                int ir = mpAnimationData->movables[m].colorData.initR;
                int r=ir;
                if(mpAnimationData->movables[m].colorData.r != 0)
                {
                    r = std::max(0, std::min(255, ir-int(mpAnimationData->movables[m].colorData.r*data[idx]*mul/div)));
                }

                int ig = mpAnimationData->movables[m].colorData.initG;
                int g = ig;
                if(mpAnimationData->movables[m].colorData.g != 0)
                {
                    g = std::max(0, std::min(255, ig-int(mpAnimationData->movables[m].colorData.g*data[idx]*mul/div)));
                }

                int ib = mpAnimationData->movables[m].colorData.initB;
                int b = ib;
                if(mpAnimationData->movables[m].colorData.b != 0)
                {
                    b = std::max(0, std::min(255, ib-int(mpAnimationData->movables[m].colorData.b*data[idx]*mul/div)));
                }

                int ia = mpAnimationData->movables[m].colorData.initA;
                if(ia == 0)
                {
                    ia = 255;
                }
                int a = ia;
                if(mpAnimationData->movables[m].colorData.a != 0)
                {
                    a = std::max(0, std::min(255, ia-int(mpAnimationData->movables[m].colorData.a*data[idx]*mul/div)));
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
                //double mrot = mpMovables[m]->rotation();
                //pos.setX((portStartX+x)*cos(mrot) - (portStartY+y)*sin(mrot));
                //pos.setY((portStartY+y)*cos(mrot) + (portStartX+x)*sin(mrot));
                //pos.setX(portStartX);
                //pos.setY(portStartY);
                pos = mpMovables[m]->mapToItem(mpBase, portStartX, portStartY);
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
            mpModelObject->getParentSystemObject()->getCoreSystemAccessPtr()->writeNodeData(mpModelObject->getName(), "out", "Value", text.toDouble());
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

        //Figure out description text
        mDescription = mpModelObject->getParameterValue("description");
        if(!mDescription.isEmpty()) {
            mDescription.append(" ");
        }

        //Figure out unit and scaling
        mUnit = mpModelObject->getParameterValue("unit");
        mUnitScaling = mpModelObject->getParameterValue("unitscaling").toDouble();

        //Figure out precision
        mPrecision = mpModelObject->getParameterValue("precision").toInt();

        //Figure out colors
        mBackgroundColor = mpModelObject->getParameterValue("backgroundcolor");
        mTextColor = mpModelObject->getParameterValue("textcolor");

        //Generate HTML style string
        mHtml = "<div style='background:rgba("+mBackgroundColor+", 100%);font-family: monospace;color:rgba("+mTextColor+", 100%); '>";
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

    if(mpAnimationData->movables[m].isSwitchable && mpAnimationData->movables[m].hideIconOnSwitch)
    {
        mpMovables.at(m)->hide();
    }




    //Get parameter multipliers and divisors for movement data
    for(int i=0; i<mpAnimationData->movables[m].movementData.size(); ++i)
    {
        QString multStr = mpAnimationData->movables[m].movementData[i].multiplier;
        if(!multStr.isEmpty())
        {
            QString parValue = mpModelObject->getParameterValue(multStr);
            if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
            {
                parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
            }
            bool ok;
            double temp = parValue.toDouble(&ok);
            if(!ok)
            {
                temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
            }
            mpAnimationData->movables[m].movementData[i].multiplierValue = temp;
        }

        QString divStr = mpAnimationData->movables[m].movementData[i].divisor;
        if(!divStr.isEmpty())
        {
            QString parValue = mpModelObject->getParameterValue(divStr);
            if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
            {
                parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
            }
            bool ok;
            double temp = parValue.toDouble(&ok);
            if(!ok)
            {
                temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
            }
            mpAnimationData->movables[m].movementData[i].divisorValue = temp;
        }
    }

    //Get parameter multipliers and divisors for resize data
    for(int i=0; i<mpAnimationData->movables[m].resizeData.size(); ++i)
    {
        QString multStr = mpAnimationData->movables[m].resizeData[i].multiplier;
        if(!multStr.isEmpty())
        {
            QString parValue = mpModelObject->getParameterValue(multStr);
            if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
            {
                parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
            }
            bool ok;
            double temp = parValue.toDouble(&ok);
            if(!ok)
            {
                temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
            }
            mpAnimationData->movables[m].resizeData[i].multiplierValue = temp;
        }

        QString divStr = mpAnimationData->movables[m].resizeData[i].divisor;
        if(!divStr.isEmpty())
        {
            QString parValue = mpModelObject->getParameterValue(divStr);
            if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
            {
                parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
            }
            bool ok;
            double temp = parValue.toDouble(&ok);
            if(!ok)
            {
                temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
            }
            mpAnimationData->movables[m].resizeData[i].divisorValue = temp;
        }
    }

    //Get parameter multipliers and divisors for color data
    QString multStr = mpAnimationData->movables[m].colorData.multiplier;
    if(!multStr.isEmpty())
    {
        QString parValue = mpModelObject->getParameterValue(multStr);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
        }
        mpAnimationData->movables[m].colorData.multiplierValue = temp;
    }

    QString divStr = mpAnimationData->movables[m].colorData.divisor;
    if(!divStr.isEmpty())
    {
        QString parValue = mpModelObject->getParameterValue(divStr);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
        }
        mpAnimationData->movables[m].colorData.divisorValue = temp;
    }

    //Old code
    double multiplierValue = 1;
    for(int p=0; p<mpAnimationData->movables[m].multipliers.size(); ++p)
    {
        if(mpAnimationData->movables[m].multipliers[p].isEmpty())
            continue;

        QString parValue = mpModelObject->getParameterValue(mpAnimationData->movables[m].multipliers[p]);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
        }
        multiplierValue *= temp;
    }
    mpAnimationData->movables[m].multiplierValue = multiplierValue;
    mpAnimationData->movables[m].useMultipliers = !mpAnimationData->movables[m].multipliers.isEmpty();
    //End old code

    //Old code
    double divisorValue = 1;
    for(int p=0; p<mpAnimationData->movables[m].divisors.size(); ++p)
    {
        if(mpAnimationData->movables[m].divisors[p].isEmpty())
            continue;

        QString parValue = mpModelObject->getParameterValue(mpAnimationData->movables[m].divisors[p]);
        if(!parValue.isEmpty() && parValue[0].isLetter())   //Starts with letter, to it must be a system parameter
        {
            parValue = mpModelObject->getParentSystemObject()->getParameterValue(parValue);
        }
        bool ok;
        double temp = parValue.toDouble(&ok);
        if(!ok)
        {
            temp = mpModelObject->getParentSystemObject()->getParameterValue(parValue).toDouble(&ok);
        }
        divisorValue *= temp;
    }
    mpAnimationData->movables[m].divisorValue = divisorValue;
    mpAnimationData->movables[m].useDivisors = !mpAnimationData->movables[m].divisors.isEmpty();
    //End old code
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
//! @param [in] pParentSystem Pointer to container object animation is showing
//! @param [in] pParent Parent object (QGraphicsItem), used for the coordinate system
AnimatedIcon::AnimatedIcon(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, AnimatedComponent *pAnimatedComponent, SystemObject *pParentSystem, int idx, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, Deselected, pParentSystem, pParent)
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
    return AnimatedObjectType;
}

QString AnimatedIcon::getHmfTagName() const
{
    // These objects are not present in hmf files, so nothing
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
        pDialog->deleteLater();
    }

    // Open plot window if double-clicking on scope component
    auto pScope = qobject_cast<AnimatedScope*>(mpAnimatedComponent);
    if (pScope) {
        pScope->openPlotwindow();
    }

    QGraphicsWidget::mouseDoubleClickEvent(event);
}


void AnimatedIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    int idx = mpAnimatedComponent->indexOfMovable(this);
    if(idx >= 0 && mpAnimatedComponent->getAnimationDataPtr()->getMovablePtr(mIdx)->isAdjustable)
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


//! @brief Handles mouse press events on animated icons, used for switchable movables
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

        bool switchable = pData->getMovablePtr(idx)->isSwitchable;
        if(switchable)
        {
            //! @todo Don't do pointer lookup every time step!
            double *pNodeData = mpAnimatedComponent->mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mpAnimatedComponent->mpModelObject->getName(), pData->movables[mIdx].switchablePort, pData->movables[mIdx].switchableDataName);
            double onValue = pData->getMovablePtr(idx)->switchableOnValue;
            double offValue = pData->getMovablePtr(idx)->switchableOffValue;
            if((*pNodeData) == onValue/*mpAnimatedComponent->mpMovables[idx]->isVisible()*/)
            {
                if(pData->getMovablePtr(idx)->hideIconOnSwitch)
                {
                    mpAnimatedComponent->mpMovables[idx]->setVisible(false);
                }
                (*pNodeData) = offValue;
            }
            else
            {
                if(pData->getMovablePtr(idx)->hideIconOnSwitch)
                {
                    mpAnimatedComponent->mpMovables[idx]->setVisible(true);
                }
                (*pNodeData) = onValue;
            }
        }
    }

    QGraphicsWidget::mousePressEvent(event);
}


//! @brief Handles mouse release events on animated icons, used for momentary switchable movables
void AnimatedIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpAnimatedComponent->mpMovables.isEmpty())
    {
        int idx = mIdx;//mpAnimatedComponent->indexOfMovable(this);
        if(idx < 0)
        {
            idx = 0;    //Not good, we assume there is only one movable and that it is the switch
        }

        ModelObjectAnimationData *pData = mpAnimatedComponent->getAnimationDataPtr();
        bool momentary = pData->getMovablePtr(idx)->isMomentary;
        if(momentary)
        {
            double *pNodeData = mpAnimatedComponent->mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mpAnimatedComponent->mpModelObject->getName(), pData->getMovablePtr(mIdx)->switchablePort, pData->movables[mIdx].switchableDataName);
            double offValue = pData->getMovablePtr(idx)->switchableOffValue;
            if(pData->getMovablePtr(idx)->hideIconOnSwitch)
            {
                mpAnimatedComponent->mpMovables[idx]->setVisible(false);
            }
            (*pNodeData) = offValue;
        }
    }

    QGraphicsWidget::mouseReleaseEvent(event);
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
    this->setTransform(transf,true); // transformation origin point seems to have no effect here for some reason
    //Reset to center pos (as transform origin point was ignored)
    this->setCenterPos(cpos);

    // If the icon is (not rotating) its position will be refreshed
    //refreshIconPosition();

    // Toggle isFlipped bool
    if(mIsFlipped)
    {
        mIsFlipped = false;
    }
    else
    {
        mIsFlipped = true;
    }
}

void AnimatedScope::openPlotwindow()
{
    if (!mpPlotWindow.isNull()) {
        mpPlotWindow->show();
        return;
    }

    LogDataHandler2* pHandler = mpModelObject->getParentSystemObject()->getLogDataHandler().data();
    const QString scopeName = mpModelObject->getName();

    mpPlotWindow = gpPlotHandler->createNewUniquePlotWindow(scopeName);
    mTimeData.clear();
    mBottomData.clear();
    mLeftDatas.clear();
    mRightDatas.clear();

    const QString timeName = scopeName+"_time";
    mTimeData.animatedPlotData = pHandler->createOrphanVariable(timeName, VariableTypeT::VectorType);
    mTimeData.animatedPlotData->assignFrom(QVector<double>() << mpAnimationWidget->getLastAnimationTime());

    for(Port* pPort : mpModelObject->getPort("in")->getConnectedPorts()) {
        QString fullName = makeFullVariableName(pPort->getParentModelObject()->getParentSystemNameHieararchy(), pPort->getParentModelObjectName(),
                                                pPort->getName(),"Value");
        auto pAnimPlotVariable = pHandler->createOrphanVariable(fullName, VariableTypeT::VectorType);
        pAnimPlotVariable->assignFrom(QVector<double>() << 0);

        gpPlotHandler->plotDataToWindow(mpPlotWindow, pAnimPlotVariable, QwtPlot::yLeft);

        AnimatedPlotData animPlotData;
        animPlotData.pModelObjectPort = pPort;
        animPlotData.animatedPlotData = pAnimPlotVariable;
        animPlotData.logData = pHandler->getVectorVariable(fullName, -1);
        mLeftDatas.append(animPlotData);
    }
    for(Port* pPort : mpModelObject->getPort("in_right")->getConnectedPorts()) {
        QString fullName = makeFullVariableName(pPort->getParentModelObject()->getParentSystemNameHieararchy(), pPort->getParentModelObjectName(),
                                                pPort->getName(),"Value");
        auto pAnimPlotVariable = pHandler->createOrphanVariable(fullName, VariableTypeT::VectorType);
        pAnimPlotVariable->assignFrom(QVector<double>() << 0);

        gpPlotHandler->plotDataToWindow(mpPlotWindow, pAnimPlotVariable, QwtPlot::yRight);

        AnimatedPlotData animPlotData;
        animPlotData.pModelObjectPort = pPort;
        animPlotData.animatedPlotData = pAnimPlotVariable;
        animPlotData.logData = pHandler->getVectorVariable(fullName, -1);
        mRightDatas.append(animPlotData);
    }

    Port *pBottomPort = mpModelObject->getPort("in_bottom");
    if(pBottomPort->isConnected()) {
        QString fullName = makeFullVariableName(pBottomPort->getParentModelObject()->getParentSystemNameHieararchy(), pBottomPort->getParentModelObjectName(),
                                                pBottomPort->getName(),"Value");
        auto pAnimPlotVariable = pHandler->createOrphanVariable(fullName, VariableTypeT::VectorType);
        pAnimPlotVariable->assignFrom(QVector<double>() << 0);

        mpPlotWindow->setXData(pAnimPlotVariable, true);

        AnimatedPlotData animPlotData;
        animPlotData.pModelObjectPort = pBottomPort;
        animPlotData.animatedPlotData = pAnimPlotVariable;
        animPlotData.logData = pHandler->getVectorVariable(fullName, -1);
        mBottomData = animPlotData;
    }
    else {
        mpPlotWindow->setXData(mTimeData.animatedPlotData, true);
    }

    mpPlotWindow->showNormal();
}

void AnimatedScope::updatePlotwindow()
{
    if (mpPlotWindow) {

        double currentTime = mpAnimationWidget->getLastAnimationTime();
        if(!mTimeData.animatedPlotData.isNull()) {
            mTimeData.animatedPlotData->append(currentTime);
        }

        // Handle real-time case
        if (mpAnimationWidget->isRealTimeAnimation()) {
            for(auto& animData : mLeftDatas) {
                updatePlotData(animData.pModelObjectPort, animData.animatedPlotData);
            }

            for(auto& animData : mRightDatas) {
                updatePlotData(animData.pModelObjectPort, animData.animatedPlotData);
            }

            if(!mBottomData.animatedPlotData.isNull()) {
                updatePlotData(mBottomData.pModelObjectPort, mBottomData.animatedPlotData);
            }
        }
        // Handle play-back case
        else {
            for(auto& animData : mLeftDatas) {
                updatePlotData(animData.logData, currentTime, animData.animatedPlotData);
            }

            for(auto& animData : mRightDatas) {
                updatePlotData(animData.logData, currentTime, animData.animatedPlotData);
            }

            if(!mBottomData.animatedPlotData.isNull()) {
                updatePlotData(mBottomData.logData, currentTime, mBottomData.animatedPlotData);
            }
        }
    }
}

void AnimatedScope::updateAnimation()
{
    AnimatedComponent::updateAnimation();
    updatePlotwindow();
}

void AnimatedPlotData::clear()
{
#if QT_VERSION >= 0x050000
    pModelObjectPort.clear();
#else
    pModelObjectPort = QPointer<Port>();
#endif
    animatedPlotData.clear();
    logData.clear();
}
