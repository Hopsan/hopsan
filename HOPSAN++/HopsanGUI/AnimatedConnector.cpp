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
//#include "Utilities/GUIUtilities.h"
#include "AnimatedConnector.h"
//#include "UndoStack.h"
#include "Widgets/ProjectTabWidget.h"
//#include "GUIObjects/GUISystem.h"
//#include "Configuration.h"

class UndoStack;

//! @brief Constructor for creation of empty non connected connector
//! @param [in] startPort The initial port the connector
//! @param [in] pParentContainer The parent container object who ones this connector
AnimatedConnector::AnimatedConnector(Connector *pConnector, AnimationWidget *parent)
        : QGraphicsWidget()
{
    mpParentAnimationWidget = parent;

    if(pConnector->getStartPort()->getNodeType() == "NodeHydraulic")
    {
        if(pConnector->getStartPort()->getPortType() == "POWERMULTIPORT")
        {
            int g = pConnector->getParentContainer()->getNumberOfPlotGenerations()-1;
            QString componentName = pConnector->getEndPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getEndPort()->getPortName();
            mvIntensityData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Pressure");
        }
        else
        {
            int g = pConnector->getParentContainer()->getNumberOfPlotGenerations()-1;
            QString componentName = pConnector->getStartPort()->getGuiModelObject()->getName();
            QString portName = pConnector->getStartPort()->getPortName();
            mvIntensityData = pConnector->getParentContainer()->getPlotData(g, componentName, portName, "Pressure");
        }

        if(!mpParentAnimationWidget->mIntensityMinMap.contains("NodeHydraulic"))
        {
            mpParentAnimationWidget->mIntensityMinMap.insert("NodeHydraulic", 0);
        }
        if(!mpParentAnimationWidget->mIntensityMaxMap.contains("NodeHydraulic"))
        {
            mpParentAnimationWidget->mIntensityMaxMap.insert("NodeHydraulic", 0);
        }
        for(int i=0; i<mvIntensityData.size(); ++i)
        {
            if(mvIntensityData.at(i) > mpParentAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value())
            {
                mpParentAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value() = mvIntensityData.at(i);
                qDebug() << "Max = " << mpParentAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value();
            }
        }
    }

    // Determine inital appearance
    mpConnectorAppearance = pConnector->mpConnectorAppearance;



    mPoints = pConnector->mPoints;

    // Create the lines, so that drawConnector has something to work with
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        AnimatedConnectorLine *pLine = new AnimatedConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                                 mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                                 mpConnectorAppearance, i, this);

//            QPen tempPen = pLine->pen();
//            if(pConnector->mIsDashed)
//            {
//                tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
//                tempPen.setStyle(Qt::CustomDashLine);
//            }
//            else
//            {
//                tempPen.setStyle(Qt::SolidLine);
//            }
        QPen tempPen = pConnector->mpLines.at(i)->pen();
        pLine->setPen(tempPen);
        mpLines.push_back(pLine);
        if(pConnector->mpLines.at(i)->mHasStartArrow)
        {
            pLine->addStartArrow();
        }
        if(pConnector->mpLines.at(i)->mHasEndArrow)
        {
            pLine->addEndArrow();
        }
    }

    //mpParentAnimationWidget->mpGraphicsView->updateViewPort();
}


//! @brief Destructor for connector class
AnimatedConnector::~AnimatedConnector()
{
    delete mpConnectorAppearance;
}


void AnimatedConnector::update()
{
    if(!mvIntensityData.isEmpty())
    {
        double max = mpParentAnimationWidget->mIntensityMaxMap.find("NodeHydraulic").value();
        double min = mpParentAnimationWidget->mIntensityMinMap.find("NodeHydraulic").value();


        QPen tempPen = mpLines.first()->pen();
        double data = mvIntensityData.at(mpParentAnimationWidget->getIndex());
        int red = 255*(data-min)/(0.8*max-min);
        int blue = 255-255*(data-min)/(0.8*max-min);
        qDebug() << "Red = " << red;
        tempPen.setColor(QColor(red,0,blue));

        for(int i=0; i<mpLines.size(); ++i)
        {
            mpLines.at(i)->setPen(tempPen);
        }
    }
}


//------------------------------------------------------------------------------------------------------------------------//


//! @brief Constructor for connector lines
//! @param x1 X-coordinate of the start position of the line.
//! @param y1 Y-coordinate of the start position of the line.
//! @param x2 X-coordinate of the end position of the line.
//! @param y2 Y-coordinate of the end position of the line.
//! @param pConnApp Pointer to the connector appearance data, containing pens
//! @param lineNumber Number of the line in the connector's line vector.
//! @param *parent Pointer to the parent object (the connector)
AnimatedConnectorLine::AnimatedConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance* pConnApp, int lineNumber, AnimatedConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mpConnectorAppearance = pConnApp;
    mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    mParentConnectorEndPortConnected = false;
    this->mStartPos = QPointF(x1,y1);
    this->mEndPos = QPointF(x2,y2);
    mHasStartArrow = false;
    mHasEndArrow = false;
    mArrowSize = 8.0;
    mArrowAngle = 0.5;
}


//! @brief Destructor for connector lines
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


//! @brief Changes the style of the line to active
//! @see setPassive()
//! @see setHovered()
void AnimatedConnectorLine::setActive()
{
        this->setPen(mpConnectorAppearance->getPen("Active"));
        if(mpParentConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
        {
            QPen tempPen = this->pen();
            tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
            tempPen.setStyle(Qt::CustomDashLine);
            this->setPen(tempPen);
        }
        this->mpParentConnector->setZValue(CONNECTOR_Z);
}


//! @brief Changes the style of the line to default
//! @see setActive()
//! @see setHovered()
void AnimatedConnectorLine::setPassive()
{
    this->setPen(mpConnectorAppearance->getPen("Primary"));
    if(mpParentConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
    {
        QPen tempPen = this->pen();
        tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
        tempPen.setStyle(Qt::CustomDashLine);
        this->setPen(tempPen);
    }
}


//! @brief Changes the style of the line to hovered
//! @see setActive()
//! @see setPassive()
void AnimatedConnectorLine::setHovered()
{
    this->setPen(mpConnectorAppearance->getPen("Hover"));
    if(mpParentConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
    {
        QPen tempPen = this->pen();
        tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
        tempPen.setStyle(Qt::CustomDashLine);
        this->setPen(tempPen);
    }
}


//! @brief Returns the number of the line in the connector
int AnimatedConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! @brief Tells the line that its parent connector has been connected at both ends
void AnimatedConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! @brief Reimplementation of setLine, stores the start and end positions before changing them
//! @param x1 X-coordinate of the start position.
//! @param y1 Y-coordinate of the start position.
//! @param x2 X-coordinate of the end position.
//! @param y2 Y-coordinate of the end position.
void AnimatedConnectorLine::setLine(QPointF pos1, QPointF pos2)
{
    this->mStartPos = this->mapFromParent(pos1);
    this->mEndPos = this->mapFromParent(pos2);
    if(mHasEndArrow)
    {
        this->addEndArrow();
    }
    else if(mHasStartArrow)
    {
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(this->mapFromParent(pos1).x(),this->mapFromParent(pos1).y(),
                               this->mapFromParent(pos2).x(),this->mapFromParent(pos2).y());
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
    this->setPen(this->pen());
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
    this->setPen(this->pen());
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
        mArrowLine1->line();
    }
}
