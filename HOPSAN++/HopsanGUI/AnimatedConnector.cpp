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

    if(pConnector->getStartPort()->getNodeType() == "NodeHydraulic" && pConnector->getStartPort()->getPortType() != "READPORT" && pConnector->getEndPort()->getPortType() != "READPORT")
    {
        if(pConnector->getStartPort()->getPortType() == "POWERMULTIPORT")
        {
            int g = pConnector->getParentContainer()->getNumberOfPlotGenerations()-1;
            QString componentName = pConnector->getEndPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getEndPort()->getPortName();
            mvIntensityData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Pressure");
            mvFlowData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Flow");
        }
        else
        {
            int g = pConnector->getParentContainer()->getNumberOfPlotGenerations()-1;
            QString componentName = pConnector->getStartPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getStartPort()->getPortName();
            mvIntensityData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Pressure");
            mvFlowData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Flow");

        }

        if(!mpAnimationWidget->mIntensityMinMap.contains("NodeHydraulic"))
        {
            mpAnimationWidget->mIntensityMinMap.insert("NodeHydraulic", 0);
        }
        if(!mpAnimationWidget->mIntensityMaxMap.contains("NodeHydraulic"))
        {
            mpAnimationWidget->mIntensityMaxMap.insert("NodeHydraulic", 0);
        }
        for(int i=0; i<mvIntensityData.size(); ++i)
        {
            if(mvIntensityData.at(i) > mpAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value())
            {
                mpAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value() = mvIntensityData.at(i);
            }
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
void AnimatedConnector::update()
{
    if(!mvIntensityData.isEmpty())      //Should consider flow as well, somehow...
    {
        //double max = mpParentAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value();
        //double min = mpParentAnimationWidget->mIntensityMinMap.find("NodeHydraulic").value();

        //! @todo User should be able to choose this setting
        double max = 2e7;       //HARD CODED!!!
        double min = 0;

        QPen tempPen = mpLines.first()->pen();
        tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);

        int index = mpAnimationWidget->getIndex();
        //int lastIndex = mpAnimationWidget->getLastIndex();
        double data, flowData;
        if(mpAnimationWidget->isRealTimeAnimation())
        {
            QString componentName, portName;
            if(mpConnector->getStartPort()->getPortType() == "POWERMULTIPORT")
            {
                componentName = mpConnector->getEndPort()->getGuiModelObject()->getName();
                portName = mpConnector->getEndPort()->getPortName();
            }
            else
            {
                componentName = mpConnector->getStartPort()->getGuiModelObject()->getName();
                portName = mpConnector->getStartPort()->getPortName();
            }
            mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(componentName, portName, "Pressure", data);
            mpAnimationWidget->mpContainer->getCoreSystemAccessPtr()->getLastNodeData(componentName, portName, "Flow", flowData);
        }
        else
        {
            data = mvIntensityData[index];
            flowData = mvFlowData[index];
        }

        int red = std::min(255.0, 255*(data-min)/(0.8*max-min));
        int blue = 255-red;
        tempPen.setColor(QColor(red,0,blue));
        tempPen.setDashOffset(mDirectionCorrection*50000*flowData*fmod(mpAnimationWidget->getLastAnimationTime(),10.0)/10.0);  //HARD CODED!!!

        for(int i=0; i<mpLines.size(); ++i)
        {
            mpLines[i]->setPen(tempPen);
        }
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
