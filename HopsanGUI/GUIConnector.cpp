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
//! @file   Connector.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the Connector class
//!
//$Id$

#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QApplication>

#include "global.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "Widgets/ModelWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "loadFunctions.h"
#include "Configuration.h"
#include "LibraryHandler.h"

class UndoStack;

//! @brief Constructor for creation of empty non connected connector
//! @param [in] startPort The initial port the connector
//! @param [in] pParentSystem The parent container object who ones this connector
Connector::Connector(SystemObject *pParentSystem)
        : QGraphicsWidget()
{
    // Init members
    mpParentSystemObject = 0;
    mpStartPort = 0;
    mpEndPort = 0;

    mpVolunectorComponent = 0;

    mMakingDiagonal = false;
    mIsDashed = false;
    mIsBroken = false;
    mIsActive = false;

    // Set parent
    this->setParentContainer(pParentSystem);

    // Add this item to the correct scene, which should also set the QtParent, the scene own the qt object
    mpParentSystemObject->getContainedScenePtr()->addItem(this);

    // Determine initial appearance
    mpConnectorAppearance = new ConnectorAppearance("Undefined", mpParentSystemObject->getGfxType());
}


//! @brief Destructor for connector class
Connector::~Connector()
{
    // Make port parent object forget this connector
    if (mpStartPort)
    {
        mpStartPort->getParentModelObject()->forgetConnector(this);
    }
    if (mpEndPort)
    {
        mpEndPort->getParentModelObject()->forgetConnector(this);
    }

    // Delete all the line segments
    this->removeAllLines();
    delete mpConnectorAppearance;
}


//Connector *Connector::createDummyCopy()
//{
//    Connector* pTempConnector = new Connector(mpParentSystemObject);

//    pTempConnector->setStartPort(mpStartPort);
//    pTempConnector->setEndPort(mpEndPort);
//    pTempConnector->setParentContainer(mpParentSystemObject);
//    mpParentSystemObject->rememberSubConnector(pTempConnector);
//    //pTempConnector->setConnected();

//    //Convert geometry vector to string list (because setPointAndGeometries wants strings)
//    QStringList geometries;
//    for(int i=0; i<mGeometries.size(); ++i)
//    {
//        if(mGeometries.at(i) == Diagonal)
//            geometries.append("diagonal");
//        else if(mGeometries.at(i) == Vertical)
//            geometries.append("vertical");
//        else
//            geometries.append("horizontal");
//    }

//    pTempConnector->setPointsAndGeometries(mPoints, geometries);
//    return pTempConnector;
//}


void Connector::disconnectPortSigSlots(Port* pPort)
{
    if (pPort)
    {
        disconnect(pPort->getParentModelObject(), SIGNAL(objectDeleted()),  this, SLOT(deleteMeWithNoUndo()));
        disconnect(pPort->getParentModelObject(), SIGNAL(objectSelected()), this, SLOT(selectIfBothComponentsSelected()));
    }
}

//! @todo if we would let the guimodelobjects connect to the connector we could avoid having two separate disconnect / connect functions we could just let the modelobject refresh the sigslot connections against the connector
void Connector::connectPortSigSlots(Port* pPort)
{
    if (pPort)
    {
        connect(pPort->getParentModelObject(),   SIGNAL(objectDeleted()),    this,   SLOT(deleteMeWithNoUndo()), Qt::UniqueConnection);
        connect(pPort->getParentModelObject(),   SIGNAL(objectSelected()),   this,   SLOT(selectIfBothComponentsSelected()), Qt::UniqueConnection);
    }
}

void Connector::setParentContainer(SystemObject *pParentSystem)
{
    if (mpParentSystemObject)
    {
        //Disconnect all old sigslot connections
        disconnect(mpParentSystemObject, SIGNAL(selectAllConnectors()),          this, SLOT(select()));
        disconnect(mpParentSystemObject, SIGNAL(setAllGfxType(GraphicsTypeEnumT)),    this, SLOT(setIsoStyle(GraphicsTypeEnumT)));
    }

    mpParentSystemObject = pParentSystem;

    //Establish new connections
    connect(mpParentSystemObject, SIGNAL(selectAllConnectors()),      this, SLOT(select()),                   Qt::UniqueConnection);
    connect(mpParentSystemObject, SIGNAL(setAllGfxType(GraphicsTypeEnumT)),   this, SLOT(setIsoStyle(GraphicsTypeEnumT)),  Qt::UniqueConnection);
}

SystemObject *Connector::getParentContainer()
{
    return mpParentSystemObject;
}


//! @brief Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point Position where the point shall be inserted (normally mouse cursor position)
//! @see removePoint(bool deleteIfEmpty)
void Connector::addPoint(QPointF point)
{
    mPoints.push_back(point);

    PortDirectionT startPortDir= LeftRightDirectionType;;
    if (mpStartPort)
    {
        startPortDir = mpStartPort->getPortDirection();
    }

    if(getNumberOfLines() == 0 && startPortDir == TopBottomDirectionType)
    {
        mGeometries.push_back(Horizontal);
    }
    else if(getNumberOfLines() == 0 && startPortDir == LeftRightDirectionType)
    {
        mGeometries.push_back(Vertical);
    }
    else if(getNumberOfLines() > 0 && mGeometries.back() == Horizontal)
    {
        mGeometries.push_back(Vertical);
    }
    else if(getNumberOfLines() > 0 && mGeometries.back() == Vertical)
    {
        mGeometries.push_back(Horizontal);
    }
    else if(getNumberOfLines() > 0 && mGeometries.back() == Diagonal)
    {
        mGeometries.push_back(Diagonal);
        //Give new line correct angle!
    }
    if(mPoints.size() > 1)
        drawConnector();
}


//! @brief Removes the last point in the connector. Asks to delete the connector if deleteIfEmpty is true and if no lines or only one non-diagonal lines remains.
//! @param deleteIfEmpty True if the connector shall be deleted if too few points remains.
//! @see addPoint(QPointF point)
void Connector::removePoint(bool deleteIfEmpty)
{
    mPoints.pop_back();
    mGeometries.pop_back();

    if( (getNumberOfLines() > 3) && !mMakingDiagonal )
    {
        if((mGeometries[mGeometries.size()-1] == Diagonal) || ((mGeometries[mGeometries.size()-2] == Diagonal)))
        {
            //if(mGeometries[mGeometries.size()-3] == HORIZONTAL)
            if(qAbs(mPoints[mPoints.size()-3].x() - mPoints[mPoints.size()-4].x()) > qAbs(mPoints[mPoints.size()-3].y() - mPoints[mPoints.size()-4].y()))
            {
                mGeometries[mGeometries.size()-2] = Horizontal;
                mGeometries[mGeometries.size()-1] = Vertical;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-1].y());
            }
            else
            {
                mGeometries[mGeometries.size()-2] = Vertical;
                mGeometries[mGeometries.size()-1] = Horizontal;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-1].x(), mPoints[mPoints.size()-3].y());
            }
        }
    }
    else if( (getNumberOfLines() == 3) && !mMakingDiagonal)
    {
        if(getStartPort()->getPortDirection() == LeftRightDirectionType)
        {
            mGeometries[1] = Horizontal;
            mGeometries[0] = Vertical;
            mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
        }
        else
        {
            mGeometries[1] = Vertical;
            mGeometries[0] = Horizontal;
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
void Connector::setStartPort(Port *pPort)
{
    if (pPort)
    {
        this->disconnectPortSigSlots(mpStartPort);
        mpStartPort = pPort;
        mpStartPort->rememberConnection(this);
        this->connectPortSigSlots(mpStartPort);
    }
}


//! @brief Sets the pointer to the end port of a connector
//! @param *port Pointer to the new end port
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void Connector::setEndPort(Port *pPort)
{
    if (pPort)
    {
        this->disconnectPortSigSlots(mpEndPort);
        mpEndPort = pPort;
        mpEndPort->rememberConnection(this);
        this->connectPortSigSlots(mpEndPort);
    }
}


//! @brief Executes the final tasks before creation of the connector is complete. Then flags that the connection if finished.
void Connector::finishCreation()
{
    if (mpStartPort && mpEndPort)
    {
        // Figure out whether or not the last line had the right direction, and make necessary corrections
        if( ( ((mpEndPort->getPortDirection() == LeftRightDirectionType) && (mGeometries.back() == Horizontal)) ||
              ((mpEndPort->getPortDirection() == TopBottomDirectionType) && (mGeometries.back() == Vertical)) ) ||
                (mGeometries[mGeometries.size()-2] == Diagonal) ||
                mpEndPort->getPortType() == "ReadMultiportType" || mpEndPort->getPortType() == "PowerMultiportType")
        {
            // Wrong direction of last line, so remove last point. This is because an extra line was added with the last click, that shall not be there. It is also possible that we end up here because the end port is a multi port, which mean that we shall not add any offset to it.
            this->removePoint();
            this->scene()->removeItem(mpLines.back());
            delete(mpLines.back());
            this->mpLines.pop_back();
        }
        else
        {
            // Correct direction of last line, which was added due to the final mouse click. This means that the last "real" line has the wrong direction.
            // We therefore keep the extra line, and move second last line a bit outwards from the component.
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
            mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort();
        }

        // Make sure the end point of the connector is the center position of the end port
        this->updateEndPoint(mpEndPort->mapToScene(mpEndPort->boundingRect().center()));

        // Snap if close to a snapping position
        const int nl = getNumberOfLines();
        if(gpConfig->getSnapping() && (nl > 0))
        {
            const QPointF diff = mPoints.first() - mPoints.last();
            // Horizontal snapping (vertical connector)
//            if( ((getNumberOfLines() == 1) && (abs(mPoints.first().x() - mPoints.last().x()) < SNAPDISTANCE)) ||
//                    ((getNumberOfLines() < 3) && (abs(mPoints.first().x() - mPoints.last().x()) < SNAPDISTANCE)) )
            if ( (qAbs(diff.x()) < SNAPDISTANCE) && (nl < 4) )
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
            // Vertical snapping (horizontal connector)
//            else if( ((getNumberOfLines() == 1) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) ||
//                     ((getNumberOfLines() < 4) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE)) )
            else if ( (qAbs(diff.y()) < SNAPDISTANCE) && (nl < 4) )
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

        // Connect show/hide signal-stuff signalto the connector if any of the components are signal components
        if(mpStartPort->getParentModelObject()->getTypeCQS() == "S" || mpEndPort->getParentModelObject()->getTypeCQS() == "S")
        {
            connect(mpParentSystemObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisible(bool)), Qt::UniqueConnection);
        }

        // Hide ports; connected ports shall not be visible
        mpStartPort->hide();
        mpEndPort->hide();

        // Not broken
        mIsBroken = false;
    }

    // Make all lines selectable and all lines except first and last movable
    for(int i=1; i<(mpLines.size()-1); ++i)
    {
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    for(int i=0; i<mpLines.size(); ++i)
    {
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    if (!(mpStartPort && mpEndPort))
    {
        mIsBroken = true;
        setZValue(BrokenConnectorZValue);
    }

    this->determineAppearance();    // Figure out which connector appearance to use
    this->setPassive();             // Make line passive (deselected)

    if(mpStartPort && mpEndPort && ((mpStartPort->getPortType() == "PowerPortType" && mpEndPort->getPortType() == "ReadPortType") ||
       (mpStartPort->getPortType() == "ReadPortType" && mpEndPort->getPortType() == "PowerPortType")))
    {
        this->setDashed(true);
    }

    emit connectionFinished();      // Let everyone know that the connection process is finished
}


//! @brief Slot that tells the connector lines whether or not to use ISO style
//! @param gfxType Tells whether or not iso graphics is to be used
void Connector::setIsoStyle(GraphicsTypeEnumT gfxType)
{
    mpConnectorAppearance->setIsoStyle(gfxType);
    setPassive();
}


//! @brief Returns the number of lines in a connector
int Connector::getNumberOfLines()
{
    return mpLines.size();
}


//! @brief Returns the geometry type of the specified line
//! @param lineNumber Number of the desired line in the mpLines vector
ConnectorGeometryEnumT Connector::getGeometry(const int lineNumber)
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
QString Connector::getStartPortName() const
{
    if (mpStartPort)
    {
        return mpStartPort->getName();
    }
    return QString();
}


//! @brief Returns the name of the end port of a connector
//! @see getStartPortName()
QString Connector::getEndPortName() const
{
    if (mpEndPort)
    {
        return mpEndPort->getName();
    }
    return QString();
}


//! @brief Returns the name of the start component of a connector
//! @see getEndComponentName()
QString Connector::getStartComponentName() const
{
    if (mpStartPort)
    {
        return mpStartPort->getParentModelObjectName();
    }
    return QString();
}


//! @brief Returns the name of the end component of a connector
//! @see getStartComponentName()
QString Connector::getEndComponentName() const
{
    if (mpEndPort)
    {
        return mpEndPort->getParentModelObjectName();
    }
    return QString();
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
    if (mpStartPort && mpEndPort)
    {
        return (getStartPort()->isConnected() && mpEndPort->isConnected());
    }
    return false;
}


//! @brief Returns true if the line currently being drawn is a diagonal one, otherwise false
//! @see makeDiagonal(bool enable)
bool Connector::isMakingDiagonal() const
{
    return mMakingDiagonal;
}


//! @brief Returns true if the connector is active (= "selected")
bool Connector::isActive() const
{
    return mIsActive;
}

bool Connector::isBroken() const
{
    return mIsBroken;
}

bool Connector::isDangling()
{
    return (mpStartPort && !mpEndPort);
}

bool Connector::isVolunector() const
{
    return (mpVolunectorComponent != 0);
}

void Connector::makeVolunector()
{
    //Create the hidden volume component
    ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr("HydraulicVolume").data();
    if (pAppearance)
    {
        mpVolunectorComponent = new Component(mpStartPort->pos(), 0, pAppearance, mpParentSystemObject);

        //Let parent container object take ownership of the hidden component
        //    QList<ModelObject*> modelObjectsList;
        //    QList<Widget*> widgetsList;
        //    modelObjectsList << mpVolunectorComponent;
        //    mpParentSystemObject->takeOwnershipOf(modelObjectsList, widgetsList);

        //Hide the hidden component
        mpVolunectorComponent->hide();
    }
}


void Connector::makeVolunector(Component *pComponent)
{
    mpVolunectorComponent = pComponent;

    //Hide the hidden component
    mpVolunectorComponent->hide();
}


Component *Connector::getVolunectorComponent()
{
    return mpVolunectorComponent;
}


//! @brief Saves connector to xml format
//! @param rDomElement Reference to the DOM element to write to
void Connector::saveToDomElement(QDomElement &rDomElement)
{
    // Ignore if connector broken
    if (mIsBroken)
    {
        if(!mFallbackDomElement.isNull()) {
            rDomElement.appendChild(mFallbackDomElement.cloneNode().toElement());
        }
        return;
    }

    //Core necessary stuff
    QDomElement xmlConnect = appendDomElement(rDomElement, hmf::connector::root);

    xmlConnect.setAttribute(hmf::connector::startcomponent, getStartComponentName());
    xmlConnect.setAttribute(hmf::connector::startport, getStartPortName());
    xmlConnect.setAttribute(hmf::connector::endcomponent, getEndComponentName());
    xmlConnect.setAttribute(hmf::connector::endport, getEndPortName());


    //Save gui data to dom
    QDomElement xmlConnectGUI = appendDomElement(xmlConnect, hmf::hopsangui);
    QDomElement xmlCoordinates = appendDomElement(xmlConnectGUI, hmf::connector::coordinates);
    for(int j=0; j<mPoints.size(); ++j)
    {
        appendCoordinateTag(xmlCoordinates, mPoints[j].x(), mPoints[j].y());
    }
    QDomElement xmlGeometries = appendDomElement(xmlConnectGUI, hmf::connector::geometries);
    for(int j=0; j<mGeometries.size(); ++j)
    {
        if(mGeometries.at(j) == Vertical)
            appendDomTextNode(xmlGeometries, hmf::connector::geometry, "vertical");
        if(mGeometries.at(j) == Horizontal)
            appendDomTextNode(xmlGeometries, hmf::connector::geometry, "horizontal");
        if(mGeometries.at(j) == Diagonal)
            appendDomTextNode(xmlGeometries, hmf::connector::geometry, "diagonal");
    }
    if(mIsDashed)
        appendDomTextNode(xmlConnectGUI, hmf::connector::style, "dashed");
    else
        appendDomTextNode(xmlConnectGUI, hmf::connector::style, "solid");
    QColor customColor = mpConnectorAppearance->getCustomColor();
    if(customColor != QColor())
    {
        appendDomTextNode(xmlConnectGUI, hmf::connector::color, makeRgbString(customColor));
    }
}


//! @brief Draws lines between the points in the mPoints vector, and stores them in the mpLines vector
void Connector::drawConnector(bool alignOperation)
{
    //Do not try to draw if no points have been added yet (avoid crash in code bellow)
    if (mPoints.size() > 0)
    {
        // First prepare connector before the actual redraw
//        // If broken nothing special should happen
//        if (isBroken())
//        {

//        }
        // If end port is not connected, which means we are creating a new line,
        if(isDangling() || isBroken() || !mpStartPort)
        {
            // Remove lines if there are too many
            while(mpLines.size() > mPoints.size()-1)
            {
                this->scene()->removeItem(mpLines.back());
                mpLines.back()->deleteLater();
                mpLines.pop_back();
            }
            // Add lines if there are too few
            while(mpLines.size() < mPoints.size()-1)
            {
                ConnectorLine *pLine = new ConnectorLine(0, 0, 0, 0, mpLines.size(), this);
                this->addLine(pLine);
            }

        }
        // else end port is connected, so the connector is modified or has moved
        else
        {
            if(mpStartPort->getParentModelObject()->isSelected() && mpEndPort->getParentModelObject()->isSelected() && this->isActive() && !alignOperation)
            {
                // Both components and connector are selected, so move whole connector along with components
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

        // Redraw the actual lines based on the mPoints vector
        for(int i=0; i<mPoints.size()-1; ++i)
        {
            if( (mpLines[i]->line().p1() != mPoints[i]) || (mpLines[i]->line().p2() != mPoints[i+1]) )   //Don't redraw the line if it has not changed
                mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
        }

        mpParentSystemObject->mpModelWidget->getGraphicsView()->updateViewPort();
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
        if(mGeometries[0] == Horizontal)
            mPoints[1] = QPointF(mPoints[0].x(),mPoints[1].y());
        else if(mGeometries[0] == Vertical)
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
        if(mGeometries.back() == Horizontal)
        {
            mPoints[mPoints.size()-2] = QPointF(point.x(),mPoints[mPoints.size()-2].y());
        }
        else if(mGeometries.back() == Vertical)
        {
            mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(),point.y());
        }
    }
}


//! @brief Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber Number of the line to update (the line that has moved)
void Connector::updateLine(int lineNumber)
{
    if ( !isDangling() && (lineNumber > 0) && (lineNumber < int(mpLines.size())) )
    {
        if(mGeometries[lineNumber] == Horizontal)
        {
            mPoints[lineNumber] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).x(), mPoints[lineNumber].y());
            mPoints[lineNumber+1] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).x(), mPoints[lineNumber+1].y());
        }
        else if (mGeometries[lineNumber] == Vertical)
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
void Connector::moveAllPoints(double offsetX, double offsetY)
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
        mGeometries.back() = Diagonal;
        mPoints.back() = mpParentSystemObject->mpModelWidget->getGraphicsView()->mapToScene(mpParentSystemObject->mpModelWidget->getGraphicsView()->mapFromGlobal(cursor.pos()));
        drawConnector();
    }
    else
    {
        if(this->getNumberOfLines() > 1)
        {
            if(mGeometries[mGeometries.size()-2] == Horizontal)
            {
                mGeometries.back() = Vertical;
                mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
            }
            else if(mGeometries[mGeometries.size()-2] == Vertical)
            {
                mGeometries.back() = Horizontal;
                mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
            }
            else if(mGeometries[mGeometries.size()-2] == Diagonal)
            {
                if(qAbs(mPoints[mPoints.size()-2].x() - mPoints[mPoints.size()-3].x()) > qAbs(mPoints[mPoints.size()-2].y() - mPoints[mPoints.size()-3].y()))
                {
                    mGeometries.back() = Horizontal;
                    mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
                }
                else
                {
                    mGeometries.back() = Vertical;
                    mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
                }

            }
            addPoint(mpParentSystemObject->mpModelWidget->getGraphicsView()->mapToScene(mpParentSystemObject->mpModelWidget->getGraphicsView()->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentSystemObject->mapToScene(mpParentSystemObject->mpModelWidget->getGraphicsView()->mapFromGlobal(cursor.pos())));
            if(getStartPort()->getPortDirection() == LeftRightDirectionType)
            {
                mGeometries[0] = Vertical;
                mGeometries[1] = Horizontal;
                mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
            }
            else
            {
                mGeometries[0] = Horizontal;
                mGeometries[1] = Vertical;
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
    // Non-finished connectors shall not be selectable
    if(!isDangling() || isBroken())
    {
        if(lineSelected)
        {
            mpParentSystemObject->rememberSelectedSubConnector(this);
            connect(mpParentSystemObject, SIGNAL(deselectAllConnectors()), this, SLOT(deselect()));
            disconnect(mpParentSystemObject, SIGNAL(selectAllConnectors()), this, SLOT(select()));
            connect(mpParentSystemObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
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
                mpParentSystemObject->forgetSelectedSubConnector(this);
                disconnect(mpParentSystemObject, SIGNAL(deselectAllConnectors()), this, SLOT(deselect()));
                connect(mpParentSystemObject, SIGNAL(selectAllConnectors()), this, SLOT(select()));
                disconnect(mpParentSystemObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            }
        }
    }
}


//! @brief Slot that selects a connector if both its components are selected
//! @see doSelect(bool lineSelected, int lineNumber)
void Connector::selectIfBothComponentsSelected()
{
    if(isConnected() && mpStartPort->getParentModelObject()->isSelected() && mpEndPort->getParentModelObject()->isSelected())
    {
        mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


void Connector::setColor(const QColor &rColor)
{
    if (!mIsBroken)
    {
        mpConnectorAppearance->setCustomColor(rColor);
    }
    setPassive();
}


//! @brief Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void Connector::setActive()
{
    connect(mpParentSystemObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if( !isDangling() || isBroken() )
    {
        refreshPen("Active");
        mIsActive = true;
    }
    this->setZValue(ConnectorZValue);
    if(mIsBroken)
    {
        this->setZValue(BrokenConnectorZValue);
    }
}


//! @brief Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void Connector::setPassive()
{
    disconnect(mpParentSystemObject, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(!isDangling() || isBroken())
    {
        mIsActive = false;
        refreshPen("Passive");
        // Deactivate lines
        for (ConnectorLine *pLine : mpLines )
        {
            pLine->setSelected(false);
        }
    }
    this->setZValue(ConnectorZValue);
    if(mIsBroken)
    {
        this->setZValue(BrokenConnectorZValue);
    }
}


//! @brief Changes connector style to hovered unless it is active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void Connector::setHovered()
{
    if( (!isDangling() || isBroken()) && !mIsActive)
    {
        refreshPen("Hover");
    }
}


//! @brief Changes connector style back to normal unless it is active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void Connector::setUnHovered()
{
    if( (!isDangling() || isBroken()) && !mIsActive)
    {
        setPassive();
    }
}


//! @brief Asks the parent system to delete the connector
void Connector::deleteMe(UndoStatusEnumT undo)
{
    if (mpParentSystemObject->getModelLockLevel()==NotLocked)
    {
        mpParentSystemObject->removeSubConnector(this, undo);
    }
}


//! @brief Asks the parent system to delete the connector, and tells it to not add it to the undo stack
//! This is necessary because slots cannot take UNDO or NOUNDO as arguments in a simple way
void Connector::deleteMeWithNoUndo()
{
    deleteMe(NoUndo);
}

void Connector::breakConnection(const Port *pPort)
{
    if(mpStartPort == pPort) {
        mpStartPort = nullptr;
        mIsBroken = true;
        refreshConnectorAppearance();
    }
    else if(mpEndPort == pPort) {
        mpEndPort = nullptr;
        mIsBroken = true;
        refreshConnectorAppearance();
    }
}


//! @brief Function that returns true if first or last line is diagonal. Used to determine snapping stuff.
bool Connector::isFirstOrLastDiagonal()
{
    return ( (mGeometries.first() == Diagonal) || (mGeometries.last() == Diagonal) );
}


//! @brief Function that returns true if both first and last line is diagonal. Used to determine snapping stuff.
bool Connector::isFirstAndLastDiagonal()
{
    return ( (mGeometries.first() == Diagonal) && (mGeometries.last() == Diagonal) );
}


//! @brief Updates the appearance of the connector (setting its type and line endings)
void Connector::determineAppearance()
{
    if (isBroken() || isDangling())
    {
        mpConnectorAppearance->setStyle(BrokenConnectorStyle);
    }
    else
    {
        QString startPortType = mpStartPort->getPortType();
        QString endPortType = mpEndPort->getPortType();

        //We need to determine if we want arrows before we replace systemporttypes with internal port types
        //Add arrow to the connector if it is of signal type
        if (mpEndPort->getNodeType() == "NodeSignal")
        {
            if( !( (endPortType == "ReadPortType" || endPortType == "ReadMultiportType") && (startPortType == "ReadPortType" || startPortType == "ReadMultiportType") ) )    //No arrow if connecting two read ports
            {
                if ( (endPortType == "ReadPortType") || (endPortType == "ReadMultiportType") || (startPortType == "WritePortType" && endPortType == "SystemPortType"))
                {
                    this->getLastLine()->addEndArrow();
                }
                else if ( (startPortType == "ReadPortType") || (startPortType == "ReadMultiportType") || (startPortType == "SystemPortType" && endPortType == "WritePortType"))
                {
                    //Assumes that the startport was a read port or multiread port
                    mpLines[0]->addStartArrow();
                }
            }
        }

        //Now replace tpes if systemports to select correct connector graphics
        if (startPortType == "SystemPortType")
        {
            startPortType = mpStartPort->getPortType(CoreSystemAccess::InternalPortType);
        }
        if (endPortType == "SystemPortType")
        {
            endPortType = mpEndPort->getPortType(CoreSystemAccess::InternalPortType);
        }

        if( (startPortType == "PowerPortType") || (endPortType == "PowerPortType") || (startPortType == "PowerMultiportType") || (endPortType == "PowerMultiportType") )
        {
            mpConnectorAppearance->setStyle(PowerConnectorStyle);
        }
        else if( (startPortType == "ReadPortType") || (endPortType == "ReadPortType") || (startPortType == "ReadMultiportType") || (endPortType == "ReadMultiportType") )
        {
            mpConnectorAppearance->setStyle(SignalConnectorStyle);
        }
        else if( (startPortType == "WritePortType") || (endPortType == "WritePortType") )
        {
            mpConnectorAppearance->setStyle(SignalConnectorStyle);
        }
        else
        {
            mpConnectorAppearance->setStyle(UndefinedConnectorStyle);
        }
    }

    //Run this to actually change the pen
    refreshPen();
}

//! @brief Redraws the connector after redetermining what appearance to use
void Connector::refreshConnectorAppearance()
{
    determineAppearance();
    drawConnector();
}

void Connector::refreshPen(const QString &type)
{
    // Decide which pen to use
    QPen pen;
    if (type == "Active")
    {
        pen = mpConnectorAppearance->getPen("Active");
    }
    else if (type == "Passive")
    {
        // Decide pen
        if(!isConnected() && !isBroken())
        {
            pen = mpConnectorAppearance->getPen("NonFinished");
        }
        else
        {
            pen = mpConnectorAppearance->getPen("Primary");
        }

    }
    else if (type == "Hover")
    {
        pen = mpConnectorAppearance->getPen("Hover");
    }

    // Dashed or not
    if (mpConnectorAppearance->getStyle() != SignalConnectorStyle)
    {
        if(mIsDashed)
        {
            pen.setDashPattern(QVector<double>() << 1.5 << 3.5);
            pen.setStyle(Qt::CustomDashLine);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
        }
    }
    // Set pen for all lines
    for (ConnectorLine *pLine : mpLines )
    {
        pLine->setPen(pen);
    }
}

void Connector::refreshPen()
{
    if (mIsActive)
    {
        refreshPen("Active");
    }
    else
    {
        refreshPen("Passive");
    }
}


//! @brief Slot that "deactivates" a connector if it is deselected
void Connector::deselect()
{
    this->setPassive();
}


//! @brief Slot that "activates" a connector if it is selected
void Connector::select()
{
    this->doSelect(true, -1);
}


//! @Brief Slot that makes a connector dashed or solid
//! @param value Boolean that is true if connector shall be dashed
void Connector::setDashed(bool value)
{
    if(mpConnectorAppearance->getStyle() == SignalConnectorStyle)
        return;

    mIsDashed=value;
    refreshPen();
    mpParentSystemObject->mpModelWidget->hasChanged();
}

//! @brief Stores XML data that can be used if the component is missing and cannot save itself
void Connector::setFallbackDomElement(const QDomElement &rElement)
{
    mFallbackDomElement = rElement.cloneNode().toElement();
}


void Connector::setVisible(bool visible)
{
    for(ConnectorLine *pLine : mpLines)
    {
        pLine->setVisible(visible);
    }
}

//! @brief Help function to setup points and create line segments, if all points are already known, old lines will be removed
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
                                                 i, this);
        //qDebug() << "Creating line from " << mPoints[i].x() << ", " << mPoints[i].y() << " to " << mPoints[i+1].x() << " " << mPoints[i+1].y();
        this->addLine(pLine);
        pLine->setConnectorFinished();
    }

    // Make all lines selectable and all lines except first and last movable
    for(int i=1; i<mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i<mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

    // Setup geometries
    //! @todo maybe we should clear this always and set up new, (this function is "loading" a new appearance)
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        if(rGeometries.empty() || rGeometries.size() < mPoints.size()-1)
        {
            if(mPoints[i].x() == mPoints[i+1].x())
                mGeometries.push_back(Horizontal);
            else if(mPoints[i].y() == mPoints[i+1].y())
                mGeometries.push_back(Vertical);
            else
                mGeometries.push_back(Diagonal);
        }
        else
        {
            if(rGeometries.at(i) == "horizontal")
                mGeometries.push_back(Horizontal);
            else if(rGeometries.at(i) == "vertical")
                mGeometries.push_back(Vertical);
            else
                mGeometries.push_back(Diagonal);
        }
    }

    // Make sure the start point and end point of the connector is the center position of the end port
    updateStartEndPositions();
}

//! @brief Helpfunction to add line segment to connector
void Connector::addLine(ConnectorLine *pLine)
{
    mpLines.push_back(pLine);
    connect(pLine,  SIGNAL(lineSelected(bool, int)),    this,       SLOT(doSelect(bool, int)),  Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineMoved(int)),             this,       SLOT(updateLine(int)),      Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineHoverEnter()),           this,       SLOT(setHovered()),         Qt::UniqueConnection);
    connect(pLine,  SIGNAL(lineHoverLeave()),           this,       SLOT(setUnHovered()),       Qt::UniqueConnection);
    connect(this,   SIGNAL(connectionFinished()),       pLine,      SLOT(setConnectorFinished()),       Qt::UniqueConnection);
}

void Connector::removeAllLines()
{
    for (int i=0; i<mpLines.size(); ++i)
    {
        mpLines[i]->deleteLater();
    }
    mpLines.clear();
}

void Connector::updateStartEndPositions()
{
    if (mpStartPort)
    {
        updateStartPoint(mpStartPort->mapToScene(mpStartPort->boundingRect().center()));
    }

    if (mpEndPort)
    {
        updateEndPoint(mpEndPort->mapToScene(mpEndPort->boundingRect().center()));
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
ConnectorLine::ConnectorLine(double x1, double y1, double x2, double y2, int lineNumber, Connector *pParentConnector)
        : QGraphicsLineItem(x1,y1,x2,y2,pParentConnector)
{
    mpParentConnector = pParentConnector;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    mParentConnectorFinished = false;
    this->mStartPos = QPointF(x1,y1);
    this->mEndPos = QPointF(x2,y2);
    mHasStartArrow = false;
    mHasEndArrow = false;
    mArrowSize = 8.0;
    mArrowAngle = 0.5;
    mpVolunectorLine = 0;
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




//! @brief Defines what happens if a mouse key is pressed while hovering a connector line
void ConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        mOldPos = this->pos();
    }
    QGraphicsLineItem::mousePressEvent(event);
}


//! @brief Defines what happens if a mouse key is released while hovering a connector line
void ConnectorLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if((this->pos() != mOldPos) && (event->button() == Qt::LeftButton))
    {
        mpParentConnector->mpParentSystemObject->getUndoStackPtr()->newPost();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
        mpParentConnector->mpParentSystemObject->getUndoStackPtr()->registerModifiedConnector(mOldPos, this->pos(), mpParentConnector, getLineNumber());
    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

void ConnectorLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mpParentConnector->mpParentSystemObject->mpModelWidget->getCurrentLockLevel() == NotLocked) {
        QGraphicsLineItem::mouseMoveEvent(event);
    }
}

//! @brief Defines what happens if the mouse cursor enters the line (changes cursor if the line is movable)
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)

void ConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent */*event*/)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(mParentConnectorFinished && mpParentConnector->getGeometry(getLineNumber()) == Vertical)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(mParentConnectorFinished && mpParentConnector->getGeometry(getLineNumber()) == Horizontal)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    if(mpParentConnector->isConnected())
    {
        mpParentConnector->setZValue(HoveredConnectorZValue);
    }
    emit lineHoverEnter();
}


//! @brief Defines what happens when mouse cursor leaves the line
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void ConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent */*event*/)
{
    if(!mpParentConnector->mIsActive)
    {
        mpParentConnector->setZValue(ConnectorZValue);
    }
    this->mpParentConnector->setZValue(ConnectorZValue);
    if(mpParentConnector->mIsBroken)
    {
        mpParentConnector->setZValue(BrokenConnectorZValue);
    }
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

    //Ignore if ctrl key is pressed
    if(mpParentConnector->getParentContainer()->mpModelWidget->getGraphicsView()->isCtrlKeyPressed())
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

    QMap<QAction*, QString> actionToTypeNameMap;
    if(mpParentConnector->isVolunector())
    {
        menu.addSeparator();
        QMenu *pVolunectorComponentsMenu = new QMenu("Change Volunector Component");
        QStringList components;
        components << "HydraulicVolume" << "HydraulicTLMlossless" << "HydraulicHose";
        for(const QString &component : components) {
            QString name = gpLibraryHandler->getModelObjectAppearancePtr(component)->getTypeName();
            QIcon icon = gpLibraryHandler->getModelObjectAppearancePtr(component)->getIcon(UserGraphics);
            QAction *pComponentAction = pVolunectorComponentsMenu->addAction(icon, name);
            pComponentAction->setIconVisibleInMenu(true);
            actionToTypeNameMap.insert(pComponentAction, component);
        }
        menu.addMenu(pVolunectorComponentsMenu);
    }

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
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    QColor newConnectorColor = QColor();
    if(selectedAction == pBlueAction)
    {
        newConnectorColor = QColor("Blue");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pRedAction)
    {
        newConnectorColor = QColor("Red");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pGreenAction)
    {
        newConnectorColor = QColor("Green");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pYellowAction)
    {
        newConnectorColor = QColor("Yellow");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pPurpleAction)
    {
        newConnectorColor = QColor("Purple");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pBrownAction)
    {
        newConnectorColor = QColor("Brown");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pOrangeAction)
    {
        newConnectorColor = QColor("Orange");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }
    if(selectedAction == pPinkAction)
    {
        newConnectorColor = QColor("Pink");
        mpParentConnector->setPassive();
        mpParentConnector->mpParentSystemObject->mpModelWidget->hasChanged();
    }

    //
    if(newConnectorColor != QColor())
    {
        QList<ModelObject*> selectedModelObjectPtrs = mpParentConnector->mpParentSystemObject->getSelectedModelObjectPtrs();
        for(int i=0; i<selectedModelObjectPtrs.size(); ++i)
        {
            QList<Connector*> connectorPtrs = selectedModelObjectPtrs[i]->getConnectorPtrs();
            for(int j=0; j<connectorPtrs.size(); ++j)
            {
                if(connectorPtrs[j]->getStartPort()->getParentModelObject()->isSelected() && connectorPtrs[j]->getEndPort()->getParentModelObject()->isSelected())
                {
                    connectorPtrs[j]->setColor(newConnectorColor);
                }
            }
        }
        mpParentConnector->setColor(newConnectorColor);
    }

    if(actionToTypeNameMap.contains(selectedAction))
    {
        QString typeName = actionToTypeNameMap.find(selectedAction).value();

        ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr(typeName).data();
        Component *pNewComponent = new Component(mpParentConnector->getStartPort()->pos(), 0, pAppearance, mpParentConnector->getParentContainer());
        Component *pOldComponent = mpParentConnector->getVolunectorComponent();

        QString startPort = mpParentConnector->getStartPortName();
        QString endPort = mpParentConnector->getEndPortName();
        QString startComponent = mpParentConnector->getStartComponentName();
        QString endComponent = mpParentConnector->getEndComponentName();

        this->scene()->removeItem(pOldComponent);
        pOldComponent->deleteInHopsanCore();
        pOldComponent->deleteLater();

        mpParentConnector->mpParentSystemObject->getCoreSystemAccessPtr()->connect(startComponent, startPort, pNewComponent->getName(), "P1");
        mpParentConnector->mpParentSystemObject->getCoreSystemAccessPtr()->connect(pNewComponent->getName(), "P2", endComponent, endPort);

        mpParentConnector->makeVolunector(pNewComponent);
        pNewComponent->hide();

        mpParentConnector->drawConnector();
    }
}

void ConnectorLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mpParentConnector->isVolunector())
    {
        //QApplication::sendEvent(mpParentConnector->getVolunectorComponent(), event);
        mpParentConnector->getVolunectorComponent()->openPropertiesDialog();
    }

    QGraphicsLineItem::mouseDoubleClickEvent(event);
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
void ConnectorLine::setConnectorFinished()
{
    mParentConnectorFinished = true;
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

    if(mpParentConnector->isVolunector())
    {
        if(!mpVolunectorLine)
        {
            mpVolunectorLine = new QGraphicsLineItem(this->line(), this->parentItem());
            this->setPen(this->pen());
        }
        else
        {
            mpVolunectorLine->setLine(this->line());
        }
    }
}


//! @brief Adds an arrow at the end of the line
//! @see addStartArrow()
void ConnectorLine::addEndArrow()
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
    this->setPen(this->pen());
    mHasEndArrow = true;
}


//! @brief Adds an arrow at the start of the line
//! @see addEndArrow()
void ConnectorLine::addStartArrow()
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
        tempPen = QPen(tempPen.color(), tempPen.widthF(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        //mArrowLine1->line();
    }
    if(mpParentConnector->isVolunector() && mpVolunectorLine)
    {
        QPen tempPen = this->pen();
        QPen tempPen2 = this->pen();
        tempPen = QPen(tempPen.color(), pen.widthF()*3, Qt::SolidLine);
        tempPen2 = QPen(QColor(Qt::white), pen.widthF(), Qt::SolidLine);
        QGraphicsLineItem::setPen(tempPen);
        mpVolunectorLine->setPen(tempPen2);
    }
}


Volunector::Volunector(SystemObject *pParentSystem)
    : Connector(pParentSystem)
{
    //ModelObjectAppearance *pAppearance = gpLibraryHandler->getModelObjectAppearancePtr("HydraulicVolume");
    //mpVolunectorComponent = new Component(this->center(), 0, pAppearance, pParentSystem);
    //mpVolunectorComponent->hide();
}
