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

#include "MainWindow.h"
#include "GraphicsView.h"
#include "AnimatedConnector.h"
#include "GUIObjects/AnimatedComponent.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIPort.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIConnectorAppearance.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIContainerObject.h"

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

    if(pConnector->getStartPort()->getNodeType() == "NodeHydraulic" && pConnector->getStartPort()->getPortType() != "READPORT" && pConnector->getEndPort()->getPortType() != "READPORT")
    {
        if(pConnector->getStartPort()->getPortType() == "POWERMULTIPORT")
        {
            int g = pConnector->getParentContainer()->getLogDataHandler()->getLatestGeneration();
            QString componentName = pConnector->getEndPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getEndPort()->getPortName();

            if(!pConnector->getParentContainer()->getLogDataHandler()->isEmpty())
            {
                mvIntensityData = pConnector->getParentContainer()->getLogDataHandler()->getPlotDataValues(g, componentName, portName, "Pressure");
                mvFlowData = pConnector->getParentContainer()->getLogDataHandler()->getPlotDataValues(g, componentName, portName, "Flow");
            }
        }
        else
        {
            int g = pConnector->getParentContainer()->getLogDataHandler()->getLatestGeneration();
            QString componentName = pConnector->getStartPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getStartPort()->getPortName();

            if(!pConnector->getParentContainer()->getLogDataHandler()->isEmpty())
            {
                mvIntensityData = pConnector->getParentContainer()->getLogDataHandler()->getPlotDataValues(g, componentName, portName, "Pressure");
                mvFlowData = pConnector->getParentContainer()->getLogDataHandler()->getPlotDataValues(g, componentName, portName, "Flow");
            }
        }

        if(mpConnector->getStartPort()->getPortType() == "POWERMULTIPORT")
        {
            mComponentName = mpConnector->getEndPort()->getGuiModelObject()->getName();
            mPortName = mpConnector->getEndPort()->getPortName();
        }
        else
        {
            mComponentName = mpConnector->getStartPort()->getGuiModelObject()->getName();
            mPortName = mpConnector->getStartPort()->getPortName();
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

    if(pConnector->getStartPort()->getGuiModelObject()->getTypeCQS() == "C")
    {
        mDirectionCorrection = 1;
    }
    else
    {
        mDirectionCorrection = -1;
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
    if(mpConnector->getStartPort()->getNodeType() != "NodeHydraulic")
    {
        return;
    }

    double max = mpAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value();
    double min = mpAnimationWidget->mIntensityMinMap.find("NodeHydraulic").value();

    QPen tempPen = mpLines.first()->pen();
    tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);

    double data, flowData;
    if(mpAnimationWidget->isRealTimeAnimation())    //Real-time animation
    {
        mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(mComponentName, mPortName, "Pressure", data);
        mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(mComponentName, mPortName, "Flow", flowData);
    }
    else if(!mvIntensityData.isEmpty())             //Replay animation
    {
        int index = mpAnimationWidget->getIndex();
        data = mvIntensityData[index];
        flowData = mvFlowData[index];
    }

    int red = std::min(255.0, 255*(data-min)/(0.8*max-min));
    int blue = 255-red;
    tempPen.setColor(QColor(red,0,blue));
    tempPen.setDashOffset(tempPen.dashOffset()+mDirectionCorrection*mpAnimationWidget->mFlowSpeedMap.find("NodeHydraulic").value()*flowData/* *fmod(mpAnimationWidget->getLastAnimationTime(),10.0)/10.0*/);

    for(int i=0; i<mpLines.size(); ++i)
    {
        mpLines[i]->setPen(tempPen);
    }

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
}


//------------------------------------------------------------------------------------------------------------------------//


//! @brief Constructor for animated connector lines
//! @param x1 X-coordinate of the start position of the line.
//! @param y1 Y-coordinate of the start position of the line.
//! @param x2 X-coordinate of the end position of the line.
//! @param y2 Y-coordinate of the end position of the line.
//! @param pConnApp Pointer to the connector appearance data, containing pens
//! @param *parent Pointer to the parent object (the animated connector)
AnimatedConnectorLine::AnimatedConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance* pConnApp, AnimatedConnector *parent)
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

    qreal angle = atan2((this->mEndPos.y()-this->mStartPos.y()), (this->mEndPos.x()-this->mStartPos.x()));
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

    qreal angle = atan2((this->mEndPos.y()-this->mStartPos.y()), (this->mEndPos.x()-this->mStartPos.x()));
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
