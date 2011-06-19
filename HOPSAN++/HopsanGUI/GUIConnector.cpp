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
//! @file   GUIConnector.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIConnector class
//!
//$Id$

#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <iostream>

#include "MainWindow.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "loadObjects.h"
#include "Configuration.h"
#include <math.h>

class UndoStack;

//! @brief Constructor for connector class
//! @param startpos defines the start position of the connector, normally the center of the starting port.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUIContainerObject *pParentContainer, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->commonConstructorCode();
    this->setParentContainer(pParentContainer);

    mpParentContainerObject->getContainedScenePtr()->addItem(this);

    this->setStartPort(startPort);
    startPort->getGuiModelObject()->rememberConnector(this);
    QPointF startPos = mapToScene(mapFromItem(startPort, startPort->boundingRect().center()));
    this->setPos(startPos);
    this->updateStartPoint(startPos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance("Undefined", mpParentContainerObject->mGfxType);
    this->addPoint(startPos);
    this->addPoint(startPos);
    this->drawConnector();
}


//! @brief Constructor used to create a whole connector at once. Used when for example loading models.
//! @param *startPort Pointer to the start port
//! @param *endPort Pointer to the end port
//! @param points Point vector for the connector
//! @param *parentView Pointer to the GraphicsView the connector belongs to
//! @param *parent Pointer to parent of the port
GUIConnector::GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> points, GUIContainerObject *pParentContainer, QStringList geometries, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->commonConstructorCode();

    this->setParentContainer(pParentContainer);

    mpStartPort = startPort;
    mpEndPort = endPort;
    mpStartPort->addConnection(this);
    mpEndPort->addConnection(this);

    connect(mpStartPort->getGuiModelObject(),SIGNAL(objectSelected()),this,SLOT(selectIfBothComponentsSelected()));
    QPointF startPos = getStartPort()->getGuiModelObject()->getCenterPos();
    this->setPos(startPos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance(startPort->getPortType(), mpParentContainerObject->mGfxType);

    mPoints = points;

        //Setup the geometries vector based on the point geometry or supplied geometry list
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        if(geometries.empty())
        {
            if(mPoints[i].x() == mPoints[i+1].x())
                mGeometries.push_back(HORIZONTAL);
            else if(mPoints[i].y() == mPoints[i+1].y())
                mGeometries.push_back(VERTICAL);
            else
                mGeometries.push_back(DIAGONAL);
        }
        else
        {
            if(geometries.at(i) == "horizontal")
                mGeometries.push_back(HORIZONTAL);
            else if(geometries.at(i) == "vertical")
                mGeometries.push_back(VERTICAL);
            else
                mGeometries.push_back(DIAGONAL);
        }
    }

    mIsConnected = true;
    emit connectionFinished();
    this->setPassive();

        //Create the lines, so that drawConnector has something to work with
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        mpTempLine = new GUIConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                          mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                          mpGUIConnectorAppearance, i, this);
        //qDebug() << "Creating line from " << mPoints[i].x() << ", " << mPoints[i].y() << " to " << mPoints[i+1].x() << " " << mPoints[i+1].y();
        mpLines.push_back(mpTempLine);
        mpTempLine->setConnected();
        mpTempLine->setPassive();
        connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
        connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
        connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
        connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
        connect(this,SIGNAL(connectionFinished()),mpTempLine,SLOT(setConnected()));
    }

    this->determineAppearance();
    this->drawConnector();
    this->connectPortSigSlots(mpEndPort);

        //Make all lines selectable and all lines except first and last movable
    for(int i=1; i<mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i<mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

    mpStartPort->getGuiModelObject()->rememberConnector(this);
    mpEndPort->getGuiModelObject()->rememberConnector(this);
    mpParentContainerObject->getContainedScenePtr()->addItem(this);

    if(mpGUIConnectorAppearance->getStyle() == SIGNALCONNECTOR)
    {
        connect(gpMainWindow->toggleSignalsAction, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
    }
}


void GUIConnector::commonConstructorCode()
{
    //Init members
    mpParentContainerObject = 0;
    mpStartPort = 0;
    mpEndPort = 0;
    mIsConnected = false;
    mMakingDiagonal = false;
    mIsDashed = false;
}


//! @brief Destructor for connector class
GUIConnector::~GUIConnector()
{
    delete mpGUIConnectorAppearance;

    mpStartPort->getGuiModelObject()->forgetConnector(this);
    if(mIsConnected)
    {
        mpEndPort->getGuiModelObject()->forgetConnector(this);
    }
}

void GUIConnector::disconnectPortSigSlots(GUIPort* pPort)
{
    bool sucess1 = true;
    bool sucess2 = true;
    if (pPort != 0)
    {
        sucess1 = disconnect(pPort->getGuiModelObject(), SIGNAL(objectDeleted()), this, SLOT(deleteMeWithNoUndo()));
        sucess2 = disconnect(pPort->getGuiModelObject(), SIGNAL(objectSelected()), this, SLOT(selectIfBothComponentsSelected()));
    }

    if (!sucess1 || !sucess2)
    {
        qDebug() << "GUIConnector::disconnectPortSigLots(): Disconnect failed: " << sucess1 << " " << sucess2;
    }

}

//! @todo if we would let the guimodelobjects connect to the connector we could avoid having two separate disconnect / connect functions we could just let the modelobject refresh the sigslot connections against the connector
void GUIConnector::connectPortSigSlots(GUIPort* pPort)
{
    bool sucess1 = true;
    bool sucess2 = true;

    if (pPort != 0)
    {
        sucess1 = connect(pPort->getGuiModelObject(),   SIGNAL(objectDeleted()),    this,   SLOT(deleteMeWithNoUndo()), Qt::UniqueConnection);
        sucess2 = connect(pPort->getGuiModelObject(),   SIGNAL(objectSelected()),   this,   SLOT(selectIfBothComponentsSelected()), Qt::UniqueConnection);
    }

    if (!sucess1 || !sucess2)
    {
        qDebug() << "GUIConnector::disconnectPortSigLots(): Connect failed: " << sucess1 << " " << sucess2;
    }
}

void GUIConnector::setParentContainer(GUIContainerObject *pParentContainer)
{
    if (mpParentContainerObject != 0)
    {
        //Disconnect all old sigslot connections
        disconnect(mpParentContainerObject, SIGNAL(selectAllGUIConnectors()),       this, SLOT(select()));
        disconnect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)),    this, SLOT(setIsoStyle(graphicsType)));
    }

    mpParentContainerObject = pParentContainer;

    //Establish new connections
    connect(mpParentContainerObject, SIGNAL(selectAllGUIConnectors()),      this, SLOT(select()),                   Qt::UniqueConnection);
    connect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)),   this, SLOT(setIsoStyle(graphicsType)),  Qt::UniqueConnection);
}

GUIContainerObject *GUIConnector::getParentContainer()
{
    return mpParentContainerObject;
}


//! @brief Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point Position where the point shall be inserted (normally mouse cursor position)
//! @see removePoint(bool deleteIfEmpty)
void GUIConnector::addPoint(QPointF point)
{
    //point = this->mapFromScene(point);
    mPoints.push_back(point);


    qDebug() << "the enum: " << getStartPort()->getPortDirection();

    if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == TOPBOTTOM)
    {
        mGeometries.push_back(HORIZONTAL);
    }
    else if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == LEFTRIGHT)
    {
        mGeometries.push_back(VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == HORIZONTAL)
    {
        mGeometries.push_back(VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == VERTICAL)
    {
        mGeometries.push_back(HORIZONTAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == DIAGONAL)
    {
        mGeometries.push_back(DIAGONAL);
        //Give new line correct angle!
    }
    if(mPoints.size() > 1)
        drawConnector();
}


//! @brief Removes the last point in the connecetor. Asks to delete the connector if deleteIfEmpty is true and if no lines or only one non-diagonal lines remains.
//! @param deleteIfEmpty True if the connector shall be deleted if too few points remains.
//! @see addPoint(QPointF point)
void GUIConnector::removePoint(bool deleteIfEmpty)
{
    mPoints.pop_back();
    mGeometries.pop_back();
    //qDebug() << "removePoint, getNumberOfLines = " << getNumberOfLines();
    if( (getNumberOfLines() > 3) && !mMakingDiagonal )
    {
        if((mGeometries[mGeometries.size()-1] == DIAGONAL) || ((mGeometries[mGeometries.size()-2] == DIAGONAL)))
        {
            //if(mGeometries[mGeometries.size()-3] == HORIZONTAL)
            if(abs(mPoints[mPoints.size()-3].x() - mPoints[mPoints.size()-4].x()) > abs(mPoints[mPoints.size()-3].y() - mPoints[mPoints.size()-4].y()))
            {
                mGeometries[mGeometries.size()-2] = HORIZONTAL;
                mGeometries[mGeometries.size()-1] = VERTICAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-1].y());
            }
            else
            {
                mGeometries[mGeometries.size()-2] = VERTICAL;
                mGeometries[mGeometries.size()-1] = HORIZONTAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-1].x(), mPoints[mPoints.size()-3].y());
            }
        }
    }
    else if( (getNumberOfLines() == 3) && !mMakingDiagonal)
    {
        if(getStartPort()->getPortDirection() == LEFTRIGHT)
        {
            mGeometries[1] = HORIZONTAL;
            mGeometries[0] = VERTICAL;
            mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
        }
        else
        {
            mGeometries[1] = VERTICAL;
            mGeometries[0] = HORIZONTAL;
            mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
        }
    }

    if( (mPoints.size() == 2) && !mMakingDiagonal )
    {
        mPoints.pop_back();
        mGeometries.pop_back();
    }
    drawConnector();
    if(mPoints.size() == 1 && deleteIfEmpty)
    {
        deleteMeWithNoUndo();
    }
}


//! @brief Sets the pointer to the start port of a connector.
//! @param *port Pointer to the new start port
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setStartPort(GUIPort *port)
{
    this->disconnectPortSigSlots(mpStartPort);
    mpStartPort = port;
    mpStartPort->addConnection(this);
    this->connectPortSigSlots(mpStartPort);
}


//! @brief Sets the pointer to the end port of a connector
//! @param *port Pointer to the new end port
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    this->disconnectPortSigSlots(mpEndPort);
    mpEndPort = port;
    mpEndPort->addConnection(this);
    this->connectPortSigSlots(mpEndPort);
}


//! @brief Executes the final tasks before creation of the connetor is complete. Then flags that the connection if finished.
void GUIConnector::finishCreation()
{
    mIsConnected = true;

        //Figure out whether or not the last line had the right direction, and make necessary corrections
    if( ( ((mpEndPort->getPortDirection() == LEFTRIGHT) && (mGeometries.back() == HORIZONTAL)) ||
          ((mpEndPort->getPortDirection() == TOPBOTTOM) && (mGeometries.back() == VERTICAL)) ) ||
          (mGeometries[mGeometries.size()-2] == DIAGONAL) ||
          mpEndPort->getPortType() == "READMULTIPORT" || mpEndPort->getPortType() == "POWERMULTIPORT")
    {
            //Wrong direction of last line, so remove last point. This is because an extra line was added with the last click, that shall not be there. It is also possible that we end up here because the end port is a multi port, which mean that we shall not add any offset to it.
        this->removePoint();
        this->scene()->removeItem(mpLines.back());
        delete(mpLines.back());
        this->mpLines.pop_back();
    }
    else
    {
            //Correct direction of last line, which was added due to the final mouse click. This means that the last "real" line has the wrong direction.
            //We therefore keep the extra line, and move second last line a bit outwards from the component.
        QPointF offsetPoint = getOffsetPointfromPort(mpStartPort, mpEndPort);
        mPoints[mPoints.size()-2] = mpEndPort->mapToScene(mpEndPort->boundingRect().center()) + offsetPoint;
        if(offsetPoint.x() != 0.0)
        {
            mPoints[mPoints.size()-3].setX(mPoints[mPoints.size()-2].x());
        }
        else
        {
            mPoints[mPoints.size()-3].setY(mPoints[mPoints.size()-2].y());
        }
        this->drawConnector();
        mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

        //Make sure the end point of the connector is the center position of the end port
    this->updateEndPoint(mpEndPort->mapToScene(mpEndPort->boundingRect().center()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(int i=1; i!=mpLines.size()-1; ++i)
        {
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    for(int i=0; i!=mpLines.size(); ++i)
    {
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    emit connectionFinished();      //Let the world know that we are connected!
    this->determineAppearance();    //Figure out which connector appearance to use
    this->setPassive();             //Make line passive (deselected)

        //Snap if close to a snapping position
    if(gConfig.getSnapping())
    {
        if( ((getNumberOfLines() == 1) && (abs(mPoints.first().x() - mPoints.last().x()) < SNAPDISTANCE)) ||
            ((getNumberOfLines() < 3) && (abs(mPoints.first().x() - mPoints.last().x()) < SNAPDISTANCE)) )
        {
            if(mpStartPort->mpParentGuiModelObject->getGUIConnectorPtrs().size() == 1)
            {
                mpStartPort->mpParentGuiModelObject->moveBy(mPoints.last().x() - mPoints.first().x(), 0);
            }
            else if (mpEndPort->mpParentGuiModelObject->getGUIConnectorPtrs().size() == 1)
            {
                mpEndPort->mpParentGuiModelObject->moveBy(mPoints.first().x() - mPoints.last().x(), 0);
            }
        }
        else if( ((getNumberOfLines() == 1) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) ||
                 ((getNumberOfLines() < 4) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) )
        {
            if(mpStartPort->mpParentGuiModelObject->getGUIConnectorPtrs().size() == 1)
            {
                mpStartPort->mpParentGuiModelObject->moveBy(0, mPoints.last().y() - mPoints.first().y());
            }
            else if (mpEndPort->mpParentGuiModelObject->getGUIConnectorPtrs().size() == 1)
            {
                mpEndPort->mpParentGuiModelObject->moveBy(0, mPoints.first().y() - mPoints.last().y());
            }
        }
    }

        //If containerport refresh graphics
    qDebug() << "Port Types: " << getStartPort()->getPortType() << " " << getEndPort()->getPortType();
    QString cqsType, portType, nodeType;
    if (getStartPort()->getPortType() == HOPSANGUICONTAINERPORTTYPENAME)
    {
        cqsType = getStartPort()->getGuiModelObject()->getTypeCQS();
        portType = getStartPort()->getPortType();
        nodeType = getStartPort()->getNodeType();
        getStartPort()->refreshPortGraphics(cqsType, portType, nodeType);
    }
    if (getEndPort()->getPortType() == HOPSANGUICONTAINERPORTTYPENAME)
    {
        cqsType = getEndPort()->getGuiModelObject()->getTypeCQS();
        portType = getEndPort()->getPortType();
        nodeType = getEndPort()->getNodeType();
        getEndPort()->refreshPortGraphics(cqsType, portType, nodeType);
    }

        //Hide ports; connected ports shall not be visible
    mpStartPort->hide();
    mpEndPort->hide();

    if(mpGUIConnectorAppearance->getStyle() == SIGNALCONNECTOR)
    {
        connect(gpMainWindow->toggleSignalsAction, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
    }
}


//! @brief Slot that tells the connector lines whether or not to use ISO style
//! @param gfxType Tells whether or not iso graphics is to be used
void GUIConnector::setIsoStyle(graphicsType gfxType)
{
    mpGUIConnectorAppearance->setIsoStyle(gfxType);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! @brief Returns the number of lines in a connector
int GUIConnector::getNumberOfLines()
{
    return mpLines.size();
}


//! @brief Returns the geometry type of the specified line
//! @param lineNumber Number of the desired line in the mpLines vector
connectorGeometry GUIConnector::getGeometry(int lineNumber)
{
    return mGeometries[lineNumber];
}


//! @brief Returns the point vector used by the connector
QVector<QPointF> GUIConnector::getPointsVector()
{
    return mPoints;
}


//! @brief Returns a pointer to the start port of a connector
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
GUIPort *GUIConnector::getStartPort()
{
    return mpStartPort;
}


//! @brief Returns a pointer to the end port of a connector
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
GUIPort *GUIConnector::getEndPort()
{
    return mpEndPort;
}


//! @brief Returns the name of the start port of a connector
//! @see getEndPortName()
QString GUIConnector::getStartPortName()
{
    return mpStartPort->getName();
}


//! @brief Returns the name of the end port of a connector
//! @see getStartPortName()
QString GUIConnector::getEndPortName()
{
    return mpEndPort->getName();
}


//! @brief Returns the name of the start component of a connector
//! @see getEndComponentName()
QString GUIConnector::getStartComponentName()
{
    return mpStartPort->getGuiModelObjectName();
}


//! @brief Returns the name of the end component of a connector
//! @see getStartComponentName()
QString GUIConnector::getEndComponentName()
{
    return mpEndPort->getGuiModelObjectName();
}


//! @brief Returns the line with specified number
//! @param line Number of the desired line
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mpLines[line];
}


//! @brief Returns the last line of the connector
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getLastLine()
{
    return mpLines[mpLines.size()-1];
}


//! @brief Returns the second last line of the connector
//! @see getThirdLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getSecondLastLine()
{
    return mpLines[mpLines.size()-2];
}


//! @brief Returns the third last line of the connector
//! @see getSecondLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getThirdLastLine()
{
    return mpLines[mpLines.size()-3];
}


//! @brief Returns true if the connector is connected at both ends, otherwise false
bool GUIConnector::isConnected()
{
    //qDebug() << "Entering isConnected()";
    //return (getStartPort()->isConnected and getEndPort()->isConnected);
    return (getStartPort()->isConnected() && mIsConnected);
}


//! @brief Returns true if the line currently being drawn is a diagonal one, otherwise false
//! @see makeDiagonal(bool enable)
bool GUIConnector::isMakingDiagonal()
{
    return mMakingDiagonal;
}


//! @brief Returns true if the connector is active (= "selected")
bool GUIConnector::isActive()
{
    return mIsActive;
}


//! @brief Saves connector to xml format
//! @param rDomElement Reference to the DOM element to write to
void GUIConnector::saveToDomElement(QDomElement &rDomElement)
{
    //Core necessary stuff
    QDomElement xmlConnect = appendDomElement(rDomElement, HMF_CONNECTORTAG);

    xmlConnect.setAttribute(HMF_CONNECTORSTARTCOMPONENTTAG, getStartComponentName());
    xmlConnect.setAttribute(HMF_CONNECTORSTARTPORTTAG, getStartPortName());
    xmlConnect.setAttribute(HMF_CONNECTORENDCOMPONENTTAG, getEndComponentName());
    xmlConnect.setAttribute(HMF_CONNECTORENDPORTTAG, getEndPortName());


    //Save gui data to dom
    QDomElement xmlConnectGUI = appendDomElement(xmlConnect, HMF_HOPSANGUITAG);
    QDomElement xmlCoordinates = appendDomElement(xmlConnectGUI, HMF_COORDINATES);
    for(int j=0; j<mPoints.size(); ++j)
    {
        appendCoordinateTag(xmlCoordinates, mPoints[j].x(), mPoints[j].y());
    }
    QDomElement xmlGeometries = appendDomElement(xmlConnectGUI, HMF_GEOMETRIES);
    for(int j=0; j<mGeometries.size(); ++j)
    {
        if(mGeometries.at(j) == VERTICAL)
            appendDomTextNode(xmlGeometries, HMF_GEOMETRYTAG, "vertical");
        if(mGeometries.at(j) == HORIZONTAL)
            appendDomTextNode(xmlGeometries, HMF_GEOMETRYTAG, "horizontal");
        if(mGeometries.at(j) == DIAGONAL)
            appendDomTextNode(xmlGeometries, HMF_GEOMETRYTAG, "diagonal");
    }
    if(mIsDashed)
        appendDomTextNode(xmlConnectGUI, HMF_STYLETAG, "dashed");
    else
        appendDomTextNode(xmlConnectGUI, HMF_STYLETAG, "solid");
}


//! @brief Draws lines between the points in the mPoints vector, and stores them in the mpLines vector
void GUIConnector::drawConnector(bool alignOperation)
{
    if(!mIsConnected)        //End port is not connected, which means we are creating a new line
    {
            //Remove lines if there are too many
        while(mpLines.size() > mPoints.size()-1)
        {
            this->scene()->removeItem(mpLines.back());
            delete(mpLines.back());
            mpLines.pop_back();
        }
            //Add lines if there are too few
        while(mpLines.size() < mPoints.size()-1)
        {
            mpTempLine = new GUIConnectorLine(0, 0, 0, 0, mpGUIConnectorAppearance, mpLines.size(), this);
            mpTempLine->setPassive();
            connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
            connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
            connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
            connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
            connect(this,SIGNAL(connectionFinished()),mpTempLine,SLOT(setConnected()));
            mpLines.push_back(mpTempLine);
        }
    }
    else        //End port is connected, so the connector is modified or has moved
    {
        if(mpStartPort->getGuiModelObject()->isSelected() && mpEndPort->getGuiModelObject()->isSelected() && this->isActive() && !alignOperation)
        {
                //Both components and connector are selected, so move whole connector along with components
            moveAllPoints(getStartPort()->mapToScene(getStartPort()->boundingRect().center()).x()-mPoints[0].x(),
                          getStartPort()->mapToScene(getStartPort()->boundingRect().center()).y()-mPoints[0].y());
        }
        else
        {
                //Retrieve start and end points from ports in case components have moved
            updateStartPoint(getStartPort()->mapToScene(getStartPort()->boundingRect().center()));
            updateEndPoint(getEndPort()->mapToScene(getEndPort()->boundingRect().center()));
        }
    }

       //Redraw the lines based on the mPoints vector
    for(int i = 0; i != mPoints.size()-1; ++i)
    {
        if( (mpLines[i]->line().p1() != mPoints[i]) || (mpLines[i]->line().p2() != mPoints[i+1]) )   //Don't redraw the line if it has not changed
        mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
    }

    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Updates the first point of the connector, and adjusts the second point accordingly depending on the geometry vector.
//! @param point Position of the new start point
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateStartPoint(QPointF point)
{
    if(mPoints.size() == 0)
        mPoints.push_back(point);
    else
        mPoints[0] = point;

    if(mPoints.size() != 1)
    {
        if(mGeometries[0] == HORIZONTAL)
            mPoints[1] = QPointF(mPoints[0].x(),mPoints[1].y());
        else if(mGeometries[0] == VERTICAL)
            mPoints[1] = QPointF(mPoints[1].x(),mPoints[0].y());
    }
}


//! @brief Updates the last point of the connector, and adjusts the second last point accordingly depending on the geometry vector.
//! @param point Position of the new start point
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateEndPoint(QPointF point)
{
    mPoints.back() = point;
    if(mGeometries.back() == HORIZONTAL)
    {
        mPoints[mPoints.size()-2] = QPointF(point.x(),mPoints[mPoints.size()-2].y());
    }
    else if(mGeometries.back() == VERTICAL)
    {
        mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(),point.y());
    }
}


//! @brief Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber Number of the line to update (the line that has moved)
void GUIConnector::updateLine(int lineNumber)
{
   if ((mIsConnected) && (lineNumber != 0) && (lineNumber != int(mpLines.size())))
    {
        if(mGeometries[lineNumber] == HORIZONTAL)
        {
            mPoints[lineNumber] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).x(), mPoints[lineNumber].y());
            mPoints[lineNumber+1] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).x(), mPoints[lineNumber+1].y());
        }
        else if (mGeometries[lineNumber] == VERTICAL)
        {
            mPoints[lineNumber] = QPointF(mPoints[lineNumber].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).y());
            mPoints[lineNumber+1] = QPointF(mPoints[lineNumber+1].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).y());
        }
    }

    drawConnector();
}


//! @brief Slot that moves all points in the connector a specified distance in a specified direction.
//! @param offsetX Distance to move in X direction
//! @param offsetY Distance to move in Y direction
void GUIConnector::moveAllPoints(qreal offsetX, qreal offsetY)
{
    for(int i=0; i<mPoints.size(); ++i)
    {
        mPoints[i] = QPointF(mPoints[i].x()+offsetX, mPoints[i].y()+offsetY);
    }
}


//! @brief Changes the last two vertical/horizontal lines to one diagonal, or changes back to vertical horizontal if last line is already diagonal
//! @param enable True (false) if diagonal mode shall be enabled (disabled)
//! @see isMakingDiagonal()
void GUIConnector::makeDiagonal(bool enable)
{
    QCursor cursor;
    if(enable)
    {
        mMakingDiagonal = true;
        removePoint();
        mGeometries.back() = DIAGONAL;
        mPoints.back() = mpParentContainerObject->mpParentProjectTab->mpGraphicsView->mapToScene(mpParentContainerObject->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos()));
        drawConnector();
    }
    else
    {
        if(this->getNumberOfLines() > 1)
        {
            if(mGeometries[mGeometries.size()-2] == HORIZONTAL)
            {
                mGeometries.back() = VERTICAL;
                mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
            }
            else if(mGeometries[mGeometries.size()-2] == VERTICAL)
            {
                mGeometries.back() = HORIZONTAL;
                mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
            }
            else if(mGeometries[mGeometries.size()-2] == DIAGONAL)
            {
                if(abs(mPoints[mPoints.size()-2].x() - mPoints[mPoints.size()-3].x()) > abs(mPoints[mPoints.size()-2].y() - mPoints[mPoints.size()-3].y()))
                {
                    mGeometries.back() = HORIZONTAL;
                    mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
                }
                else
                {
                    mGeometries.back() = VERTICAL;
                    mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
                }

            }
            addPoint(mpParentContainerObject->mpParentProjectTab->mpGraphicsView->mapToScene(mpParentContainerObject->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentContainerObject->mapToScene(mpParentContainerObject->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
            if(getStartPort()->getPortDirection() == LEFTRIGHT)
            {
                mGeometries[0] = VERTICAL;
                mGeometries[1] = HORIZONTAL;
                mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
            }
            else
            {
                mGeometries[0] = HORIZONTAL;
                mGeometries[1] = VERTICAL;
                mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
            }
        }

        drawConnector();
        mMakingDiagonal = false;
    }
}


//! @brief Slot that activates or deactivates the connector if one of its lines is selected or deselected
//! @param lineSelected Tells whether the signal was induced by selection or deselection of a line
//! @param lineNumber Number of the line that was selected or deselected
//! @see setActive()
//! @see setPassive()
void GUIConnector::doSelect(bool lineSelected, int lineNumber)
{
    if(mIsConnected)     //Non-finished connectors shall not be selectable
    {
        if(lineSelected)
        {
            if(!mpParentContainerObject->mSelectedSubConnectorsList.contains(this))
            {
                mpParentContainerObject->mSelectedSubConnectorsList.append(this);
            }
            connect(mpParentContainerObject, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
            disconnect(mpParentContainerObject, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
            connect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            this->setActive();
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(i != lineNumber)     //This makes sure that only one line in a connector can be "selected" at one time
               {
                   mpLines[i]->setSelected(false);
               }
               if(lineNumber == -1)
               {
                   mpLines[0]->setSelected(true);
               }
            }
        }
        else
        {
            bool noneSelected = true;
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(mpLines[i]->isSelected())
               {
                   noneSelected = false;
               }
            }
            if(noneSelected)
            {
                this->setPassive();
                mpParentContainerObject->mSelectedSubConnectorsList.removeOne(this);
                disconnect(mpParentContainerObject, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
                connect(mpParentContainerObject, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
                disconnect(mpParentContainerObject->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            }
        }
    }
}


//! @brief Slot that selects a connector if both its components are selected
//! @see doSelect(bool lineSelected, int lineNumber)
void GUIConnector::selectIfBothComponentsSelected()
{
    if(mIsConnected && mpStartPort->getGuiModelObject()->isSelected() && mpEndPort->getGuiModelObject()->isSelected())
    {
        mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


//! @brief Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mIsConnected)
    {
        mIsActive = true;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setActive();
            //mpLines[i]->setSelected(true);         //???
        }
    }
}


//! @brief Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void GUIConnector::setPassive()
{
    disconnect(mpParentContainerObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mIsConnected)
    {
        mIsActive = false;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
            mpLines[i]->setSelected(false);       //OBS! Kanske inte blir bra...
        }
    }

    this->setZValue(1);
}


//! @brief Changes connector style to hovered unless it is active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(mIsConnected && !mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setHovered();
        }
    }
}


//! @brief Changes connector style back to normal unless it is active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void GUIConnector::setUnHovered()
{
    if(mIsConnected && !mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
        }
    }
}


//! @brief Asks the parent system to delete the connector
void GUIConnector::deleteMe(undoStatus undo)
{
    mpParentContainerObject->removeConnector(this, undo);
}


//! @brief Asks the parent system to delete the connector, and tells it to not add it to the undo stack
//! This is necessary because slots cannot take UNDO or NOUNDO as arguments in a simple way
void GUIConnector::deleteMeWithNoUndo()
{
    deleteMe(NOUNDO);
}


//! @brief Function that returns true if first or last line is diagonal. Used to determine snapping stuff.
bool GUIConnector::isFirstOrLastDiagonal()
{
    return ( (mGeometries.first() == DIAGONAL) || (mGeometries.last() == DIAGONAL) );
}


//! @brief Function that returns true if both first and last line is diagonal. Used to determine snapping stuff.
bool GUIConnector::isFirstAndLastDiagonal()
{
    return ( (mGeometries.first() == DIAGONAL) && (mGeometries.last() == DIAGONAL) );
}


//! @brief Uppdates the appearance of the connector (setting its type and line endings)
void GUIConnector::determineAppearance()
{
    QString startPortType = mpStartPort->getPortType();
    QString endPortType = mpEndPort->getPortType();

    qDebug() << "startPortType = " << startPortType;
    qDebug() << "endPortType = " << endPortType;

    //We need to determine if we want arrows before we replace systemporttypes with internal port types
    //Add arrow to the connector if it is of signal type
    if (mpEndPort->getNodeType() == "NodeSignal")
    {
        if( !( (endPortType == "READPORT" || endPortType == "READMULTIPORT") && (startPortType == "READPORT" || startPortType == "READMULTIPORT") ) )    //No arrow if connecting two read ports
        {
            if ( (endPortType == "READPORT") || (endPortType == "READMULTIPORT") || (startPortType == "WRITEPORT" && endPortType == "SYSTEMPORT"))
            {
                this->getLastLine()->addEndArrow();
            }
            else if ( (startPortType == "READPORT") || (startPortType == "READMULTIPORT") || (startPortType == "SYSTEMPORT" && endPortType == "WRITEPORT"))
            {
                //Assumes that the startport was a read port or multiread port
                mpLines[0]->addStartArrow();
            }
        }
    }

    //Now replace tpes if systemports to select correct connector graphics
    if (startPortType == "SYSTEMPORT")
    {
        startPortType = mpStartPort->getPortType(GUIPort::INTERNALPORTTYPE);
    }
    if (endPortType == "SYSTEMPORT")
    {
        endPortType = mpEndPort->getPortType(GUIPort::INTERNALPORTTYPE);
    }

    if( (startPortType == "POWERPORT") || (endPortType == "POWERPORT") || (startPortType == "POWERMULTIPORT") || (endPortType == "POWERMULTIPORT") )
    {
        mpGUIConnectorAppearance->setStyle(POWERCONNECTOR);
    }
    else if( (startPortType == "READPORT") || (endPortType == "READPORT") || (startPortType == "READMULTIPORT") || (endPortType == "READMULTIPORT") )
    {
        mpGUIConnectorAppearance->setStyle(SIGNALCONNECTOR);
    }
    else if( (startPortType == "WRITEPORT") || (endPortType == "WRITEPORT") )
    {
        mpGUIConnectorAppearance->setStyle(SIGNALCONNECTOR);
    }
    else
    {
        mpGUIConnectorAppearance->setStyle(UNDEFINEDCONNECTOR);
    }

    //Run this to actually change the pen
    this->setPassive();
}


//! @brief Slot that "deactivates" a connector if it is deselected
void GUIConnector::deselect()
{
    //qDebug() << "Deselecting connector!";
    this->setPassive();
}


//! @brief Slot that "activates" a connector if it is selected
void GUIConnector::select()
{
    //qDebug() << "Selecting connector!";
    this->doSelect(true, -1);
}


//! @Brief Slot that makes a connector dashed or solid
//! @param value Boolean that is true if connector shall be dashed
void GUIConnector::setDashed(bool value)
{
    if(mpGUIConnectorAppearance->getStyle() == SIGNALCONNECTOR)
        return;

    mpParentContainerObject->mpParentProjectTab->hasChanged();
    mIsDashed=value;
    for(int i=0; i<mpLines.size(); ++i)
    {
        QPen tempPen = mpLines.at(i)->pen();
        if(value)
        {
            tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
            tempPen.setStyle(Qt::CustomDashLine);
        }
        else
            tempPen.setStyle(Qt::SolidLine);
        mpLines.at(i)->setPen(tempPen);
    }
}


void GUIConnector::setVisible(bool visible)
{
    for(int i=0; i<mpLines.size(); ++i)
    {
        mpLines.at(i)->setVisible(visible);
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
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, GUIConnectorAppearance* pConnApp, int lineNumber, GUIConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentGUIConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mpConnectorAppearance = pConnApp;
    mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    mHasStartArrow = false;
    mHasEndArrow = false;
    mArrowSize = 8.0;
    mArrowAngle = 0.5;
}


//! @brief Destructor for connector lines
GUIConnectorLine::~GUIConnectorLine()
{
}


//! @brief Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! @brief Changes the style of the line to active
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mpConnectorAppearance->getPen("Active"));
        if(mpParentGUIConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
        {
            QPen tempPen = this->pen();
            tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
            tempPen.setStyle(Qt::CustomDashLine);
            this->setPen(tempPen);
        }
        this->mpParentGUIConnector->setZValue(1);
}


//! @brief Changes the style of the line to default
//! @see setActive()
//! @see setHovered()
void GUIConnectorLine::setPassive()
{
    if(!mpParentGUIConnector->isConnected())
    {
        this->setPen(mpConnectorAppearance->getPen("NonFinished"));
    }
    else
    {
        this->setPen(mpConnectorAppearance->getPen("Primary"));
    }
    if(mpParentGUIConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
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
void GUIConnectorLine::setHovered()
{
    this->setPen(mpConnectorAppearance->getPen("Hover"));
    if(mpParentGUIConnector->mIsDashed && mpConnectorAppearance->getStyle() != SIGNALCONNECTOR)
    {
        QPen tempPen = this->pen();
        tempPen.setDashPattern(QVector<qreal>() << 1.5 << 3.5);
        tempPen.setStyle(Qt::CustomDashLine);
        this->setPen(tempPen);
    }
}


//! @brief Defines what happens if a mouse key is pressed while hovering a connector line
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mOldPos = this->pos();
    }
    QGraphicsLineItem::mousePressEvent(event);
}


//! @brief Defines what happens if a mouse key is released while hovering a connector line
void GUIConnectorLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if((this->pos() != mOldPos) && (event->button() == Qt::LeftButton))
    {
        mpParentGUIConnector->mpParentContainerObject->mUndoStack->newPost();
        mpParentGUIConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
        mpParentGUIConnector->mpParentContainerObject->mUndoStack->registerModifiedConnector(mOldPos, this->pos(), mpParentGUIConnector, getLineNumber());
    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

//! @brief Defines what happens if the mouse cursor enters the line (changes cursor if the line is movable)
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(mParentConnectorEndPortConnected && mpParentGUIConnector->getGeometry(getLineNumber()) == VERTICAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(mParentConnectorEndPortConnected && mpParentGUIConnector->getGeometry(getLineNumber()) == HORIZONTAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    if(mpParentGUIConnector->isConnected())
    {
        mpParentGUIConnector->setZValue(11);
    }
    emit lineHoverEnter();
}


//! @brief Defines what happens when mouse cursor leaves the line
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!mpParentGUIConnector->mIsActive)
    {
        mpParentGUIConnector->setZValue(1);
    }
    this->mpParentGUIConnector->setZValue(1);
    emit lineHoverLeave();
}


//! @brief Handles context menu events for connector
void GUIConnectorLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!mpParentGUIConnector->isConnected() || mpParentGUIConnector->getStartPort()->getNodeType() == "NodeSignal")
    {
        event->ignore();
        return;
    }

    QMenu menu;

    QAction *pMakeDashedAction = 0;
    QAction *pMakeSolidAction = 0;

    if(!mpParentGUIConnector->mIsDashed)
    {
        pMakeDashedAction = menu.addAction("Make Connector Dashed");
    }
    else
    {
        pMakeSolidAction = menu.addAction("Make Connector Solid");
    }


    //-- User interaction --//
    QAction *selectedAction = menu.exec(event->screenPos());
    //----------------------//


    if(pMakeDashedAction != 0 && selectedAction == pMakeDashedAction)         //Make connector dashed
    {
        mpParentGUIConnector->setDashed(true);
    }
    if(pMakeSolidAction != 0 && selectedAction == pMakeSolidAction)          //Make connector solid
    {
        mpParentGUIConnector->setDashed(false);
    }
}



//! @brief Returns the number of the line in the connector
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! @brief Defines what shall happen if the line is selected or moved
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        emit lineSelected(this->isSelected(), mLineNumber);
    }
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit lineMoved(mLineNumber);
    }
    return value;
}


//! @brief Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! @brief Reimplementation of setLine, stores the start and end positions before changing them
//! @param x1 X-coordinate of the start position.
//! @param y1 Y-coordinate of the start position.
//! @param x2 X-coordinate of the end position.
//! @param y2 Y-coordinate of the end position.
void GUIConnectorLine::setLine(QPointF pos1, QPointF pos2)
{
    this->startPos = this->mapFromParent(pos1);
    this->endPos = this->mapFromParent(pos2);
    if(mHasEndArrow)
    {
        delete(mArrowLine1);
        delete(mArrowLine2);
        this->addEndArrow();
    }
    else if(mHasStartArrow)
    {
        delete(mArrowLine1);
        delete(mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(this->mapFromParent(pos1).x(),this->mapFromParent(pos1).y(),
                               this->mapFromParent(pos2).x(),this->mapFromParent(pos2).y());
}


//! @brief Adds an arrow at the end of the line
//! @see addStartArrow()
void GUIConnectorLine::addEndArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle+mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->endPos.x(),
                                        this->endPos.y(),
                                        this->endPos.x()-mArrowSize*cos(angle-mArrowAngle),
                                        this->endPos.y()-mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    mHasEndArrow = true;
}


//! @brief Adds an arrow at the start of the line
//! @see addEndArrow()
void GUIConnectorLine::addStartArrow()
{
    qreal angle = atan2((this->endPos.y()-this->startPos.y()), (this->endPos.x()-this->startPos.x()));
    mArrowLine1 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle+mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle+mArrowAngle), this);
    mArrowLine2 = new QGraphicsLineItem(this->startPos.x(),
                                        this->startPos.y(),
                                        this->startPos.x()+mArrowSize*cos(angle-mArrowAngle),
                                        this->startPos.y()+mArrowSize*sin(angle-mArrowAngle), this);
    this->setPen(this->pen());
    mHasStartArrow = true;
}


//! @brief Reimplementation of setPen function, used to set the pen style for the arrow lines too
void GUIConnectorLine::setPen (const QPen &pen)
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
