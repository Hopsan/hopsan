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
//! @file   Connector.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the Connector class
//!
//$Id$

#include <QDebug>
#include <QStyleOptionGraphicsItem>

#include "MainWindow.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "loadFunctions.h"
#include "Configuration.h"

class UndoStack;

//! @brief Constructor for creation of empty non connected connector
//! @param [in] startPort The initial port the connector
//! @param [in] pParentContainer The parent container object who ones this connector
Connector::Connector(ContainerObject *pParentContainer)
        : QGraphicsWidget()
{
    // Init members
    mpParentContainerObject = 0;
    mpStartPort = 0;
    mpEndPort = 0;
    mIsConnected = false;
    mMakingDiagonal = false;
    mIsDashed = false;

    // Set parent
    this->setParentContainer(pParentContainer);

    // Add this item to the correct scene, whcih should also set the QtParent, the scene own the qt object
    mpParentContainerObject->getContainedScenePtr()->addItem(this);

    // Determine inital appearance
    mpConnectorAppearance = new ConnectorAppearance("Undefined", mpParentContainerObject->getGfxType());
}


//! @brief Destructor for connector class
Connector::~Connector()
{
    // Delete all the line segments
    this->removeAllLines();

    delete mpConnectorAppearance;

    mpStartPort->getParentModelObject()->forgetConnector(this);
    if(mIsConnected)
    {
        mpEndPort->getParentModelObject()->forgetConnector(this);
    }
}


Connector *Connector::createDummyCopy()
{
    Connector* pTempConnector = new Connector(mpParentContainerObject);

    pTempConnector->setStartPort(mpStartPort);
    pTempConnector->setEndPort(mpEndPort);
    pTempConnector->setParentContainer(mpParentContainerObject);
    mpParentContainerObject->rememberSubConnector(pTempConnector);
    pTempConnector->setConnected();

    //Convert geometry vector to string list (because setPointAndGeometries wants strings)
    QStringList geometries;
    for(int i=0; i<mGeometries.size(); ++i)
    {
        if(mGeometries.at(i) == DIAGONAL)
            geometries.append("diagonal");
        else if(mGeometries.at(i) == VERTICAL)
            geometries.append("vertical");
        else
            geometries.append("horizontal");
    }

    pTempConnector->setPointsAndGeometries(mPoints, geometries);
    return pTempConnector;
}


void Connector::disconnectPortSigSlots(Port* pPort)
{
    bool sucess1 = true;
    bool sucess2 = true;
    if (pPort != 0)
    {
        sucess1 = disconnect(pPort->getParentModelObject(), SIGNAL(objectDeleted()), this, SLOT(deleteMeWithNoUndo()));
        sucess2 = disconnect(pPort->getParentModelObject(), SIGNAL(objectSelected()), this, SLOT(selectIfBothComponentsSelected()));
    }

    if (!sucess1 || !sucess2)
    {
        qDebug() << "Connector::disconnectPortSigLots(): Disconnect failed: " << sucess1 << " " << sucess2;
    }

}

//! @todo if we would let the guimodelobjects connect to the connector we could avoid having two separate disconnect / connect functions we could just let the modelobject refresh the sigslot connections against the connector
void Connector::connectPortSigSlots(Port* pPort)
{
    bool sucess1 = true;
    bool sucess2 = true;

    if (pPort != 0)
    {
        sucess1 = connect(pPort->getParentModelObject(),   SIGNAL(objectDeleted()),    this,   SLOT(deleteMeWithNoUndo()), Qt::UniqueConnection);
        sucess2 = connect(pPort->getParentModelObject(),   SIGNAL(objectSelected()),   this,   SLOT(selectIfBothComponentsSelected()), Qt::UniqueConnection);
    }

    if (!sucess1 || !sucess2)
    {
        qDebug() << "Connector::disconnectPortSigLots(): Connect failed: " << sucess1 << " " << sucess2;
    }
}

void Connector::setParentContainer(ContainerObject *pParentContainer)
{
    if (mpParentContainerObject != 0)
    {
        //Disconnect all old sigslot connections
        disconnect(mpParentContainerObject, SIGNAL(selectAllConnectors()),          this, SLOT(select()));
        disconnect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)),    this, SLOT(setIsoStyle(graphicsType)));
    }

    mpParentContainerObject = pParentContainer;

    //Establish new connections
    connect(mpParentContainerObject, SIGNAL(selectAllConnectors()),      this, SLOT(select()),                   Qt::UniqueConnection);
    connect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)),   this, SLOT(setIsoStyle(graphicsType)),  Qt::UniqueConnection);
}

ContainerObject *Connector::getParentContainer()
{
    return mpParentContainerObject;
}


//! @brief Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point Position where the point shall be inserted (normally mouse cursor position)
//! @see removePoint(bool deleteIfEmpty)
void Connector::addPoint(QPointF point)
{
    //point = this->mapFromScene(point);
    mPoints.push_back(point);


    //qDebug() << "the enum: " << getStartPort()->getPortDirection();

    if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == TopBottomDirectionType)
    {
        mGeometries.push_back(HORIZONTAL);
    }
    else if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == LeftRightDirectionType)
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
void Connector::removePoint(bool deleteIfEmpty)
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
        if(getStartPort()->getPortDirection() == LeftRightDirectionType)
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
void Connector::setStartPort(Port *port)
{
    this->disconnectPortSigSlots(mpStartPort);
    mpStartPort = port;
    mpStartPort->rememberConnection(this);
    this->connectPortSigSlots(mpStartPort);
}


//! @brief Sets the pointer to the end port of a connector
//! @param *port Pointer to the new end port
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void Connector::setEndPort(Port *port)
{
    this->disconnectPortSigSlots(mpEndPort);
    mpEndPort = port;
    mpEndPort->rememberConnection(this);
    this->connectPortSigSlots(mpEndPort);
}


//! @brief Executes the final tasks before creation of the connetor is complete. Then flags that the connection if finished.
void Connector::finishCreation()
{
    mIsConnected = true;

        //Figure out whether or not the last line had the right direction, and make necessary corrections
    if( ( ((mpEndPort->getPortDirection() == LeftRightDirectionType) && (mGeometries.back() == HORIZONTAL)) ||
          ((mpEndPort->getPortDirection() == TopBottomDirectionType) && (mGeometries.back() == VERTICAL)) ) ||
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
        this->determineAppearance();    //Figure out which connector appearance to use
        this->drawConnector();
        mpParentContainerObject->mpParentProjectTab->getGraphicsView()->updateViewPort();
    }

        //Make sure the end point of the connector is the center position of the end port
    this->updateEndPoint(mpEndPort->mapToScene(mpEndPort->boundingRect().center()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(int i=1; i<(mpLines.size()-1); ++i)
        {
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
    }
    for(int i=0; i<mpLines.size(); ++i)
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
            if(mpStartPort->getParentModelObject()->getConnectorPtrs().size() == 1)
            {
                mpStartPort->getParentModelObject()->moveBy(mPoints.last().x() - mPoints.first().x(), 0);
            }
            else if (mpEndPort->getParentModelObject()->getConnectorPtrs().size() == 1)
            {
                mpEndPort->getParentModelObject()->moveBy(mPoints.first().x() - mPoints.last().x(), 0);
            }
        }
        else if( ((getNumberOfLines() == 1) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) ||
                 ((getNumberOfLines() < 4) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) )
        {
            if(mpStartPort->getParentModelObject()->getConnectorPtrs().size() == 1)
            {
                mpStartPort->getParentModelObject()->moveBy(0, mPoints.last().y() - mPoints.first().y());
            }
            else if (mpEndPort->getParentModelObject()->getConnectorPtrs().size() == 1)
            {
                mpEndPort->getParentModelObject()->moveBy(0, mPoints.first().y() - mPoints.last().y());
            }
        }
    }


        //Hide ports; connected ports shall not be visible
    mpStartPort->hide();
    mpEndPort->hide();

    if(mpConnectorAppearance->getStyle() == SIGNALCONNECTOR)
    {
        connect(mpParentContainerObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisible(bool)), Qt::UniqueConnection);
    }
}


//! @brief Slot that tells the connector lines whether or not to use ISO style
//! @param gfxType Tells whether or not iso graphics is to be used
void Connector::setIsoStyle(graphicsType gfxType)
{
    mpConnectorAppearance->setIsoStyle(gfxType);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! @brief Returns the number of lines in a connector
int Connector::getNumberOfLines()
{
    return mpLines.size();
}


//! @brief Returns the geometry type of the specified line
//! @param lineNumber Number of the desired line in the mpLines vector
connectorGeometry Connector::getGeometry(const int lineNumber)
{
    return mGeometries[lineNumber];
}


//! @brief Returns a pointer to the start port of a connector
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
Port *Connector::getStartPort()
{
    return mpStartPort;
}


//! @brief Returns a pointer to the end port of a connector
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
Port *Connector::getEndPort()
{
    return mpEndPort;
}


//! @brief Returns the start point of the connector
QPointF Connector::getStartPoint()
{
    return mPoints.first();
}


//! @brief Returns the end point of the connector
QPointF Connector::getEndPoint()
{
    return mPoints.last();
}

//! @brief Returns the name of the start port of a connector
//! @see getEndPortName()
QString Connector::getStartPortName()
{
    return mpStartPort->getName();
}


//! @brief Returns the name of the end port of a connector
//! @see getStartPortName()
QString Connector::getEndPortName()
{
    return mpEndPort->getName();
}


//! @brief Returns the name of the start component of a connector
//! @see getEndComponentName()
QString Connector::getStartComponentName()
{
    return mpStartPort->getParentModelObjectName();
}


//! @brief Returns the name of the end component of a connector
//! @see getStartComponentName()
QString Connector::getEndComponentName()
{
    return mpEndPort->getParentModelObjectName();
}


//! @brief Returns the line with specified number
//! @param line Number of the desired line
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
ConnectorLine *Connector::getLine(int line)
{
    return mpLines[line];
}


//! @brief Returns the last line of the connector
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
ConnectorLine *Connector::getLastLine()
{
    return mpLines[mpLines.size()-1];
}


//! @brief Returns true if the connector is connected at both ends, otherwise false
bool Connector::isConnected()
{
    //qDebug() << "Entering isConnected()";
    //return (getStartPort()->isConnected and getEndPort()->isConnected);
    return (getStartPort()->isConnected() && mIsConnected);
}


//! @brief Returns true if the line currently being drawn is a diagonal one, otherwise false
//! @see makeDiagonal(bool enable)
bool Connector::isMakingDiagonal()
{
    return mMakingDiagonal;
}


//! @brief Returns true if the connector is active (= "selected")
bool Connector::isActive()
{
    return mIsActive;
}


//! @brief Saves connector to xml format
//! @param rDomElement Reference to the DOM element to write to
void Connector::saveToDomElement(QDomElement &rDomElement)
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
    QColor customColor = mpConnectorAppearance->getCustomColor();
    if(customColor != QColor())
    {
        appendDomTextNode(xmlConnectGUI, HMF_COLORTAG, makeRgbString(customColor));
    }
}


//! @brief Draws lines between the points in the mPoints vector, and stores them in the mpLines vector
void Connector::drawConnector(bool alignOperation)
{
    //Do not try to draw if no points have been added yet (avoid crash in code bellow)
    if (mPoints.size() > 0)
    {
        if(!mIsConnected)        //End port is not connected, which means we are creating a new line
        {
            //Remove lines if there are too many
            while(mpLines.size() > mPoints.size()-1)
            {
                this->scene()->removeItem(mpLines.back());
                mpLines.back()->deleteLater();
                mpLines.pop_back();
            }
            //Add lines if there are too few
            while(mpLines.size() < mPoints.size()-1)
            {
                ConnectorLine *pLine = new ConnectorLine(0, 0, 0, 0, mpConnectorAppearance, mpLines.size(), this);
                this->addLine(pLine);
            }

        }
        else        //End port is connected, so the connector is modified or has moved
        {
            if(mpStartPort->getParentModelObject()->isSelected() && mpEndPort->getParentModelObject()->isSelected() && this->isActive() && !alignOperation)
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
        for(int i=0; i<mPoints.size()-1; ++i)
        {
            if( (mpLines[i]->line().p1() != mPoints[i]) || (mpLines[i]->line().p2() != mPoints[i+1]) )   //Don't redraw the line if it has not changed
                mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
        }

        mpParentContainerObject->mpParentProjectTab->getGraphicsView()->updateViewPort();
    }
}


//! @brief Updates the first point of the connector, and adjusts the second point accordingly depending on the geometry vector.
//! @param point Position of the new start point
//! @see updateEndPoint(QPointF point)
void Connector::updateStartPoint(QPointF point)
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
//! @param point Position of the new end point
//! @see updateEndPoint(QPointF point)
void Connector::updateEndPoint(QPointF point)
{
    //Skip if we have no end point yet
    if (mPoints.size() > 1)
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
}


//! @brief Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber Number of the line to update (the line that has moved)
void Connector::updateLine(int lineNumber)
{
    //! @todo what da heck is (lineNumber != int(mpLines.size()) supposed to mean, maybe it should be < instead of !=
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
void Connector::moveAllPoints(qreal offsetX, qreal offsetY)
{
    for(int i=0; i<mPoints.size(); ++i)
    {
        mPoints[i] = QPointF(mPoints[i].x()+offsetX, mPoints[i].y()+offsetY);
    }
}


//! @brief Changes the last two vertical/horizontal lines to one diagonal, or changes back to vertical horizontal if last line is already diagonal
//! @param enable True (false) if diagonal mode shall be enabled (disabled)
//! @see isMakingDiagonal()
void Connector::makeDiagonal(bool enable)
{
    QCursor cursor;
    if(enable)
    {
        mMakingDiagonal = true;
        removePoint();
        mGeometries.back() = DIAGONAL;
        mPoints.back() = mpParentContainerObject->mpParentProjectTab->getGraphicsView()->mapToScene(mpParentContainerObject->mpParentProjectTab->getGraphicsView()->mapFromGlobal(cursor.pos()));
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
            addPoint(mpParentContainerObject->mpParentProjectTab->getGraphicsView()->mapToScene(mpParentContainerObject->mpParentProjectTab->getGraphicsView()->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentContainerObject->mapToScene(mpParentContainerObject->mpParentProjectTab->getGraphicsView()->mapFromGlobal(cursor.pos())));
            if(getStartPort()->getPortDirection() == LeftRightDirectionType)
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
void Connector::doSelect(bool lineSelected, int lineNumber)
{
    if(mIsConnected)     //Non-finished connectors shall not be selectable
    {
        if(lineSelected)
        {
            mpParentContainerObject->rememberSelectedSubConnector(this);
            connect(mpParentContainerObject, SIGNAL(deselectAllConnectors()), this, SLOT(deselect()));
            disconnect(mpParentContainerObject, SIGNAL(selectAllConnectors()), this, SLOT(select()));
            connect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
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
                mpParentContainerObject->forgetSelectedSubConnector(this);
                disconnect(mpParentContainerObject, SIGNAL(deselectAllConnectors()), this, SLOT(deselect()));
                connect(mpParentContainerObject, SIGNAL(selectAllConnectors()), this, SLOT(select()));
                disconnect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            }
        }
    }
}


//! @brief Slot that selects a connector if both its components are selected
//! @see doSelect(bool lineSelected, int lineNumber)
void Connector::selectIfBothComponentsSelected()
{
    if(mIsConnected && mpStartPort->getParentModelObject()->isSelected() && mpEndPort->getParentModelObject()->isSelected())
    {
        mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


void Connector::setColor(QColor color)
{
    mpConnectorAppearance->setCustomColor(color);
    setPassive();
}


//! @brief Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void Connector::setActive()
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
void Connector::setPassive()
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

    this->setZValue(CONNECTOR_Z);
}


//! @brief Changes connector style to hovered unless it is active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void Connector::setHovered()
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
void Connector::setUnHovered()
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
void Connector::deleteMe(undoStatus undo)
{
    mpParentContainerObject->removeSubConnector(this, undo);
}


//! @brief Asks the parent system to delete the connector, and tells it to not add it to the undo stack
//! This is necessary because slots cannot take UNDO or NOUNDO as arguments in a simple way
void Connector::deleteMeWithNoUndo()
{
    deleteMe(NOUNDO);
}


//! @brief Function that returns true if first or last line is diagonal. Used to determine snapping stuff.
bool Connector::isFirstOrLastDiagonal()
{
    return ( (mGeometries.first() == DIAGONAL) || (mGeometries.last() == DIAGONAL) );
}


//! @brief Function that returns true if both first and last line is diagonal. Used to determine snapping stuff.
bool Connector::isFirstAndLastDiagonal()
{
    return ( (mGeometries.first() == DIAGONAL) && (mGeometries.last() == DIAGONAL) );
}


//! @brief Uppdates the appearance of the connector (setting its type and line endings)
void Connector::determineAppearance()
{
    QString startPortType = mpStartPort->getPortType();
    QString endPortType = mpEndPort->getPortType();

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
        startPortType = mpStartPort->getPortType(CoreSystemAccess::InternalPortType);
    }
    if (endPortType == "SYSTEMPORT")
    {
        endPortType = mpEndPort->getPortType(CoreSystemAccess::InternalPortType);
    }

    if( (startPortType == "POWERPORT") || (endPortType == "POWERPORT") || (startPortType == "POWERMULTIPORT") || (endPortType == "POWERMULTIPORT") )
    {
        mpConnectorAppearance->setStyle(POWERCONNECTOR);
    }
    else if( (startPortType == "READPORT") || (endPortType == "READPORT") || (startPortType == "READMULTIPORT") || (endPortType == "READMULTIPORT") )
    {
        mpConnectorAppearance->setStyle(SIGNALCONNECTOR);
    }
    else if( (startPortType == "WRITEPORT") || (endPortType == "WRITEPORT") )
    {
        mpConnectorAppearance->setStyle(SIGNALCONNECTOR);
    }
    else
    {
        mpConnectorAppearance->setStyle(UNDEFINEDCONNECTOR);
    }

    //Run this to actually change the pen
    this->setPassive();
}

//! @brief Redraws the connector after redetermining what appearanche to use
void Connector::refreshConnectorAppearance()
{
    determineAppearance();
    drawConnector();
}


//! @brief Slot that "deactivates" a connector if it is deselected
void Connector::deselect()
{
    //qDebug() << "Deselecting connector!";
    this->setPassive();
}


//! @brief Slot that "activates" a connector if it is selected
void Connector::select()
{
    //qDebug() << "Selecting connector!";
    this->doSelect(true, -1);
}


//! @Brief Slot that makes a connector dashed or solid
//! @param value Boolean that is true if connector shall be dashed
void Connector::setDashed(bool value)
{
    if(mpConnectorAppearance->getStyle() == SIGNALCONNECTOR)
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


void Connector::setConnected()
{
    mIsConnected = true;
}


void Connector::setVisible(bool visible)
{
    for(int i=0; i<mpLines.size(); ++i)
    {
        mpLines.at(i)->setVisible(visible);
    }
}

//! @brief Helpfunction to setup points and create line segments, if all points are already known, old lines will be removed
void Connector::setPointsAndGeometries(const QVector<QPointF> &rPoints, const QStringList &rGeometries)
{
    // First clear any old lines, points and geometries
    this->removeAllLines();
    mPoints.clear();
    mGeometries.clear();

    mPoints = rPoints;

    // Create the lines, so that drawConnector has something to work with
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        ConnectorLine *pLine = new ConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                                 mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                                 mpConnectorAppearance, i, this);
        //qDebug() << "Creating line from " << mPoints[i].x() << ", " << mPoints[i].y() << " to " << mPoints[i+1].x() << " " << mPoints[i+1].y();
        this->addLine(pLine);
        pLine->setConnected();
    }

    // Make all lines selectable and all lines except first and last movable
    for(int i=1; i<mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i<mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

    // Setup geometries
    //! @todo maybe we should clear this allways and set up new, (this function is "loading" a new appearance)
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        if(rGeometries.empty())
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
            if(rGeometries.at(i) == "horizontal")
                mGeometries.push_back(HORIZONTAL);
            else if(rGeometries.at(i) == "vertical")
                mGeometries.push_back(VERTICAL);
            else
                mGeometries.push_back(DIAGONAL);
        }
    }

    //Make sure the start point and end point of the connector is the center position of the end port
    this->updateStartPoint(getStartPort()->mapToScene(getStartPort()->boundingRect().center()));
    this->updateEndPoint(getEndPort()->mapToScene(getEndPort()->boundingRect().center()));
}

//! @brief Helpfunction to add linesegment to connector
void Connector::addLine(ConnectorLine *pLine)
{
    mpLines.push_back(pLine);
    pLine->setPassive();
    connect(pLine,  SIGNAL(lineSelected(bool, int)),    this,       SLOT(doSelect(bool, int)),  Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineMoved(int)),             this,       SLOT(updateLine(int)),      Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineHoverEnter()),           this,       SLOT(setHovered()),         Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineHoverLeave()),           this,       SLOT(setUnHovered()),       Qt::UniqueConnection);
    connect(this,   SIGNAL(connectionFinished()),       pLine,      SLOT(setConnected()),       Qt::UniqueConnection);
}

void Connector::removeAllLines()
{
    for (int i=0; i<mpLines.size(); ++i)
    {
        mpLines[i]->deleteLater();
    }
    mpLines.clear();
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
ConnectorLine::ConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, ConnectorAppearance* pConnApp, int lineNumber, Connector *parent)
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
ConnectorLine::~ConnectorLine()
{
    clearArrows();
}


//! @brief Reimplementation of paint function. Removes the ugly dotted selection box.
void ConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! @brief Changes the style of the line to active
//! @see setPassive()
//! @see setHovered()
void ConnectorLine::setActive()
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
void ConnectorLine::setPassive()
{
    if(!mpParentConnector->isConnected())
    {
        this->setPen(mpConnectorAppearance->getPen("NonFinished"));
    }
    else
    {
        this->setPen(mpConnectorAppearance->getPen("Primary"));
    }
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
void ConnectorLine::setHovered()
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


//! @brief Defines what happens if a mouse key is pressed while hovering a connector line
void ConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        mOldPos = this->pos();
    }
    QGraphicsLineItem::mousePressEvent(event);
}


//! @brief Defines what happens if a mouse key is released while hovering a connector line
void ConnectorLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if((this->pos() != mOldPos) && (event->button() == Qt::LeftButton))
    {
        mpParentConnector->mpParentContainerObject->getUndoStackPtr()->newPost();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
        mpParentConnector->mpParentContainerObject->getUndoStackPtr()->registerModifiedConnector(mOldPos, this->pos(), mpParentConnector, getLineNumber());
    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

//! @brief Defines what happens if the mouse cursor enters the line (changes cursor if the line is movable)
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)

void ConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent */*event*/)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(mParentConnectorEndPortConnected && mpParentConnector->getGeometry(getLineNumber()) == VERTICAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(mParentConnectorEndPortConnected && mpParentConnector->getGeometry(getLineNumber()) == HORIZONTAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    if(mpParentConnector->isConnected())
    {
        mpParentConnector->setZValue(HOVEREDCONNECTOR_Z);
    }
    emit lineHoverEnter();
}


//! @brief Defines what happens when mouse cursor leaves the line
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void ConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent */*event*/)
{
    if(!mpParentConnector->mIsActive)
    {
        mpParentConnector->setZValue(CONNECTOR_Z);
    }
    this->mpParentConnector->setZValue(CONNECTOR_Z);
    emit lineHoverLeave();
}


//! @brief Handles context menu events for connector
void ConnectorLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!mpParentConnector->isConnected())
    {
        event->ignore();
        return;
    }

    QMenu menu;

    QAction *pMakeDashedAction = 0;
    QAction *pMakeSolidAction = 0;

    if(mpParentConnector->getStartPort()->getNodeType() != "NodeSignal")
    {
        if(!mpParentConnector->mIsDashed)
        {
            pMakeDashedAction = menu.addAction("Make Connector Dashed");
        }
        else
        {
            pMakeSolidAction = menu.addAction("Make Connector Solid");
        }
    }

    QMenu *pColorMenu = new QMenu("Change Color");
    QAction *pResetAction = pColorMenu->addAction("Reset Default");
    QAction *pBlueAction = pColorMenu->addAction("Blue");
    QAction *pRedAction = pColorMenu->addAction("Red");
    QAction *pGreenAction = pColorMenu->addAction("Green");
    QAction *pYellowAction = pColorMenu->addAction("Yellow");
    QAction *pPurpleAction = pColorMenu->addAction("Purple");
    QAction *pBrownAction = pColorMenu->addAction("Brown");
    QAction *pOrangeAction = pColorMenu->addAction("Orange");
    QAction *pPinkAction = pColorMenu->addAction("Pink");
    menu.addMenu(pColorMenu);

    //-- User interaction --//
    QAction *selectedAction = menu.exec(event->screenPos());
    //----------------------//


    if(pMakeDashedAction != 0 && selectedAction == pMakeDashedAction)         //Make connector dashed
    {
        mpParentConnector->setDashed(true);
    }
    if(pMakeSolidAction != 0 && selectedAction == pMakeSolidAction)          //Make connector solid
    {
        mpParentConnector->setDashed(false);
    }
    if(selectedAction == pResetAction)
    {
        mpParentConnector->setColor(QColor());
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pBlueAction)
    {
        mpParentConnector->setColor(QColor("Blue"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pRedAction)
    {
        mpParentConnector->setColor(QColor("Red"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pGreenAction)
    {
        mpParentConnector->setColor(QColor("Green"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pYellowAction)
    {
        mpParentConnector->setColor(QColor("Yellow"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pPurpleAction)
    {
        mpParentConnector->setColor(QColor("Purple"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pBrownAction)
    {
        mpParentConnector->setColor(QColor("Brown"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pOrangeAction)
    {
        mpParentConnector->setColor(QColor("Orange"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    if(selectedAction == pPinkAction)
    {
        mpParentConnector->setColor(QColor("Pink"));
        mpParentConnector->setPassive();
        mpParentConnector->mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
}



//! @brief Returns the number of the line in the connector
int ConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! @brief Defines what shall happen if the line is selected or moved
QVariant ConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
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
void ConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! @brief Reimplementation of setLine, stores the start and end positions before changing them
//! @param x1 X-coordinate of the start position.
//! @param y1 Y-coordinate of the start position.
//! @param x2 X-coordinate of the end position.
//! @param y2 Y-coordinate of the end position.
void ConnectorLine::setLine(QPointF pos1, QPointF pos2)
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
void ConnectorLine::addEndArrow()
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
void ConnectorLine::addStartArrow()
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
void ConnectorLine::clearArrows()
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
void ConnectorLine::setPen (const QPen &pen)
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
