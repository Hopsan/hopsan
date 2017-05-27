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

#include <QDebug>
#include <QColor>
#include <QStyleOptionGraphicsItem>

#include "GraphicsView.h"
#include "AnimatedConnector.h"
#include "GUIObjects/AnimatedComponent.h"
#include "Widgets/ModelWidget.h"
#include "GUIPort.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIConnectorAppearance.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/AnimationWidget.h"

class UndoStack;

//! @brief Constructor for animated connector
//! @param [in] pConnector Pointer to the real connector the animation represents
//! @param [in] pAnimationWidget Pointer to animation widget where the connector is shown
AnimatedConnector::AnimatedConnector(Connector *pConnector, AnimationWidget *pAnimationWidget)
        : QGraphicsWidget()
{
    mpAnimationWidget = pAnimationWidget;
    mpConnector = pConnector;

    mpStartComponent = mpAnimationWidget->getAnimatedComponent(pConnector->getStartComponentName());
    mpEndComponent = mpAnimationWidget->getAnimatedComponent(pConnector->getEndComponentName());
    mStartPortName = pConnector->getStartPortName();
    mEndPortName = pConnector->getEndPortName();

    mpDataPressure = 0;
    mpDataFlow = 0;

    if(pConnector->getStartPort()->getNodeType() == "NodeHydraulic" && pConnector->getStartPort()->getPortType() != "ReadPortType" && pConnector->getEndPort()->getPortType() != "ReadPortType")
    {
        if(pConnector->getStartPort()->getPortType() == "PowerMultiportType")
        {
            int g = pConnector->getParentContainer()->getLogDataHandler()->getCurrentGenerationNumber();
            QString componentName = pConnector->getEndPort()->getParentModelObject()->getName();
            QString portName = pConnector->getEndPort()->getName();

            if(!pConnector->getParentContainer()->getLogDataHandler()->isEmpty())
            {
                //! @todo it should be smarter to generate the fullnames once before animation to avoid the fullname creation every time, especially since adding the system name hierarchy thing
                mvIntensityData = pConnector->getParentContainer()->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(pConnector->getParentContainer()->getSystemNameHieararchy(), componentName, portName, "Pressure"), g);
                mvFlowData = pConnector->getParentContainer()->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(pConnector->getParentContainer()->getSystemNameHieararchy(), componentName, portName, "Flow"), g);
            }
        }
        else
        {
            int g = pConnector->getParentContainer()->getLogDataHandler()->getCurrentGenerationNumber();
            QString componentName = pConnector->getStartPort()->getParentModelObject()->getName();
            QString portName = pConnector->getStartPort()->getName();

            if(!pConnector->getParentContainer()->getLogDataHandler()->isEmpty())
            {
                mvIntensityData = pConnector->getParentContainer()->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(pConnector->getParentContainer()->getSystemNameHieararchy(), componentName, portName, "Pressure"),g);
                mvFlowData = pConnector->getParentContainer()->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(pConnector->getParentContainer()->getSystemNameHieararchy(), componentName, portName, "Flow"),g);
            }
        }

        if(mpConnector->getStartPort()->getPortType() == "PowerMultiportType")
        {
            mComponentName = mpConnector->getEndPort()->getParentModelObject()->getName();
            mPortName = mpConnector->getEndPort()->getName();
        }
        else
        {
            mComponentName = mpConnector->getStartPort()->getParentModelObject()->getName();
            mPortName = mpConnector->getStartPort()->getName();
        }

    }

    // Determine appearance
    mpConnectorAppearance = pConnector->mpConnectorAppearance;

    mPoints = pConnector->mPoints;

    // Create the lines
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        AnimatedConnectorLine *pLine = new AnimatedConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                                 mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                                 mpConnectorAppearance, this);

        QPen tempPen = pConnector->mpLines.at(i)->pen();
        mpLines.push_back(pLine);
        if(pConnector->mpLines.at(i)->mHasStartArrow)
        {
            pLine->addStartArrow();
        }
        if(pConnector->mpLines.at(i)->mHasEndArrow)
        {
            pLine->addEndArrow();
        }
        pLine->setPen(tempPen);
    }

    if(pConnector->getStartPort()->getParentModelObject()->getTypeCQS() == "C")
    {
        mDirectionCorrection = 1;
    }
    else
    {
        mDirectionCorrection = -1;
    }

    if(mpConnector->getStartPort()->getNodeType() == "NodeHydraulic" || mpConnector->getEndPort()->getNodeType() == "NodeHydraulic")
    {
        mpDataPressure = mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mComponentName, mPortName, "Pressure");
        mpDataFlow = mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getNodeDataPtr(mComponentName, mPortName, "Flow");
    }
}


//! @brief Destructor for animated connector class
AnimatedConnector::~AnimatedConnector()
{
    delete mpConnectorAppearance;
}


//! @brief Updates the animated connector
void AnimatedConnector::updateAnimation()
{
    QPointF startPos = mapFromItem(mpStartComponent->mpModelObject, mpStartComponent->getPortPos(mStartPortName));
    double x1 = startPos.x();
    double y1 = startPos.y();
    double x2 = mpLines.first()->line().x2();
    double y2 = mpLines.first()->line().y2();
    mpLines.first()->setLine(x1, y1, x2, y2);

    QPointF endPos = mapFromItem(mpEndComponent->mpModelObject, mpEndComponent->getPortPos(mEndPortName));
    x1 = mpLines.last()->line().x1();
    y1 = mpLines.last()->line().y1();
    x2 = endPos.x();
    y2 = endPos.y();
    mpLines.last()->setLine(x1, y1, x2, y2);

    if(!mpDataPressure || !mpDataFlow/*mpConnector->getStartPort()->getNodeType() != "NodeHydraulic"*/)   //!< @todo Bad to compare with string
    {
        return;
    }

    double min = mpAnimationWidget->mHydraulicIntensityMin;
    double max = mpAnimationWidget->mHydraulicIntensityMax;

    QPen tempPen = mpLines.first()->pen();
    tempPen.setDashPattern(QVector<double>() << 1.5 << 3.5);

    double pressureData, flowData;
    if(mpAnimationWidget->isRealTimeAnimation())    //Real-time animation
    {
        pressureData = *mpDataPressure;
        flowData = *mpDataFlow;
    }
    else if(!mvIntensityData.isEmpty())             //Replay animation
    {
        int index = mpAnimationWidget->getIndex();
        pressureData = mvIntensityData[index];
        flowData = mvFlowData[index];
    }
    else
    {
        pressureData = 0;
        flowData = 0;
    }

    int red = std::min(255.0, 255*(pressureData-min)/(0.8*max-min));
    int blue = 255-red;
    tempPen.setColor(QColor(red,0,blue));
    tempPen.setDashOffset(tempPen.dashOffset()+mDirectionCorrection*mpAnimationWidget->mFlowSpeedMap.find("NodeHydraulic").value()*flowData);

    for(int i=0; i<mpLines.size(); ++i)
    {
        mpLines[i]->setPen(tempPen);
    }
}


//------------------------------------------------------------------------------------------------------------------------//


//! @brief Constructor for animated connector lines
//! @param x1 X-coordinate of the start position of the line.
//! @param y1 Y-coordinate of the start position of the line.
//! @param x2 X-coordinate of the end position of the line.
//! @param y2 Y-coordinate of the end position of the line.
//! @param pConnApp Pointer to the connector appearance data, containing pens
//! @param *parent Pointer to the parent object (the animated connector)
AnimatedConnectorLine::AnimatedConnectorLine(double x1, double y1, double x2, double y2, ConnectorAppearance* pConnApp, AnimatedConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mpConnectorAppearance = pConnApp;
    this->setAcceptHoverEvents(true);
    this->mStartPos = QPointF(x1,y1);
    this->mEndPos = QPointF(x2,y2);
    mHasStartArrow = false;
    mHasEndArrow = false;
    mArrowSize = 8.0;
    mArrowAngle = 0.5;

    this->setVisible(mpParentConnector->mpConnector->getLastLine()->isVisible());
}


//! @brief Destructor for animated connector lines
AnimatedConnectorLine::~AnimatedConnectorLine()
{
    clearArrows();
}


//! @brief Reimplementation of paint function. Removes the ugly dotted selection box.
void AnimatedConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! @brief Adds an arrow at the end of the line
//! @see addStartArrow()
void AnimatedConnectorLine::addEndArrow()
{
    clearArrows();

    double angle = atan2((this->mEndPos.y()-this->mStartPos.y()), (this->mEndPos.x()-this->mStartPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->mEndPos.x(),
                                        this->mEndPos.y(),
                                        this->mEndPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->mEndPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->mEndPos.x(),
                                        this->mEndPos.y(),
                                        this->mEndPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->mEndPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    mArrowLine1->setPen(this->pen());
    mArrowLine2->setPen(this->pen());
    mHasEndArrow = true;
}


//! @brief Adds an arrow at the start of the line
//! @see addEndArrow()
void AnimatedConnectorLine::addStartArrow()
{
    clearArrows();

    double angle = atan2((this->mEndPos.y()-this->mStartPos.y()), (this->mEndPos.x()-this->mStartPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->mStartPos.x(),
                                        this->mStartPos.y(),
                                        this->mStartPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->mStartPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->mStartPos.x(),
                                        this->mStartPos.y(),
                                        this->mStartPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->mStartPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    mArrowLine1->setPen(this->pen());
    mArrowLine2->setPen(this->pen());
    mHasStartArrow = true;
}

//! @brief Clears all arrows
void AnimatedConnectorLine::clearArrows()
{
    if (mHasStartArrow || mHasEndArrow)
    {
        delete mArrowLine1;
        delete mArrowLine2;
        mHasStartArrow = false;
        mHasEndArrow = false;
    }
}


//! @brief Reimplementation of setPen function, used to set the pen style for the arrow lines too
void AnimatedConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(mHasStartArrow | mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
    }
}
