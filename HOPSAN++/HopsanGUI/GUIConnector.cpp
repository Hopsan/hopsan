/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$


#include <QDebug>
#include <QStyleOptionGraphicsItem>

#include "GUIPort.h"
#include "GraphicsView.h"
#include "GUIUtilities.h"
#include "GUIObject.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "ProjectTabWidget.h"
#include <math.h>

//! Constructor.
//! @param startpos defines the start position of the connector, normally the center of the starting port.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(QPointF startpos, GraphicsView *parentView, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    this->mpParentGraphicsView = parentView;

    setFlags(QGraphicsItem::ItemIsFocusable);

    this->setPos(startpos);

    this->updateStartPoint(startpos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance("notconnected", mpParentGraphicsView->mpParentProjectTab->useIsoGraphics);

    this->mEndPortConnected = false;

    this->drawConnector();

    this->mMakingDiagonal = false;

    connect(mpParentGraphicsView, SIGNAL(zoomChange()), this, SLOT(adjustToZoom()));
}


//! Constructor used to create a whole connector at once. Used when loading models.
//! @param *startPort is a pointer to the start port.
//! @param *endPort is a pointer to the end port.
//! @param points is the point vector for the connector.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> points, GraphicsView *parentView, QGraphicsItem *parent)
{
    this->mpParentGraphicsView = parentView;
    setFlags(QGraphicsItem::ItemIsFocusable);
    mpStartPort = startPort;
    mpEndPort = endPort;
    mpStartPort->isConnected = true;
    mpEndPort->isConnected = true;
    connect(this->mpStartPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
    connect(this->mpEndPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
    QPointF startPos = getStartPort()->mapToScene(getStartPort()->boundingRect().center());
    this->setPos(startPos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance(startPort->getPortType(), mpParentGraphicsView->mpParentProjectTab->useIsoGraphics);
//        //Set pen styles
//    this->mPrimaryPen = primaryPen;
//    this->mActivePen = activePen;
//    this->mHoverPen = hoverPen;
    mPoints = points;

        //Setup the geometries vector based on the point geometry
    for(int i=0; i != mPoints.size()-1; ++i)
    {
        if(mPoints[i].x() == mPoints[i+1].x())
            mGeometries.push_back(GUIConnector::HORIZONTAL);
        else if(mPoints[i].y() == mPoints[i+1].y())
            mGeometries.push_back(GUIConnector::VERTICAL);
        else
            mGeometries.push_back(GUIConnector::DIAGONAL);
    }

    mEndPortConnected = true;
    emit endPortConnected();
    this->setPassive();
    connect(this->mpEndPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));

        //Create the lines, so that drawConnector has something to work with
    for(int i = 0; i != mPoints.size()-1; ++i)
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
        connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
    }

    this->determineAppearance();
    this->drawConnector();

        //Make all lines selectable and all lines except first and last movable
    for(int i=1; i!=mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);



//      //Add arrow to the connector if it is of signal type
//    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        this->getLastLine()->addEndArrow();
//    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        this->mpLines[0]->addStartArrow();

    mpStartPort->getGuiObject()->addConnector(this);
    mpEndPort->getGuiObject()->addConnector(this);

    connect(mpParentGraphicsView, SIGNAL(zoomChange()), this, SLOT(adjustToZoom()));
}


//! Destructor.
GUIConnector::~GUIConnector()
{
    //qDebug() << this->getNumberOfLines();

    //mpParentGraphicsView->undoStack->registerDeletedConnector(this);

//    QMap<QString, GUIConnector *>::iterator it;
//    for(it = mpParentGraphicsView->mConnectorVector.begin(); it!=mpParentGraphicsView->mConnectorVector.end(); ++it)
//    {
//        if(mpParentGraphicsView->mConnectorVector.empty())
//        {
//            break;
//        }
//        else if(it.value() = this)
//        {
//            mpParentGraphicsView->mConnectorVector.erase(it);
//        }
//    }

    //mpLines.clear();
    //! @todo more cleanup
    delete mpGUIConnectorAppearance;
}


//! Inserts a new point to the connector and adjusts the previous point accordingly, depending on the geometry vector.
//! @param point is the position where the point shall be inserted, normally the cursor position.
//! @see removePoint(bool deleteIfEmpty)
void GUIConnector::addPoint(QPointF point)
{
    //point = this->mapFromScene(point);
    mPoints.push_back(point);

    if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == PortAppearance::VERTICAL)
    {
        mGeometries.push_back(GUIConnector::HORIZONTAL);
    }
    else if(getNumberOfLines() == 0 && getStartPort()->getPortDirection() == PortAppearance::HORIZONTAL)
    {
        mGeometries.push_back(GUIConnector::VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::HORIZONTAL)
    {
        mGeometries.push_back(GUIConnector::VERTICAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::VERTICAL)
    {
        mGeometries.push_back(GUIConnector::HORIZONTAL);
    }
    else if(getNumberOfLines() != 0 && mGeometries.back() == GUIConnector::DIAGONAL)
    {
        mGeometries.push_back(GUIConnector::DIAGONAL);
        //Give new line correct angle!
    }
    if(mPoints.size() > 1)
        drawConnector();
}


//! Removes the last point in the connecetor. Asks to delete the connector if deleteIfEmpty is true and if no lines or only one non-diagonal lines remains.
//! @param deleteIfEmpty tells whether or not the connector shall be deleted if too few points remains.
//! @see addPoint(QPointF point)
void GUIConnector::removePoint(bool deleteIfEmpty)
{
    mPoints.pop_back();
    mGeometries.pop_back();
    //qDebug() << "removePoint, getNumberOfLines = " << getNumberOfLines();
    if(getNumberOfLines() > 3 and !mMakingDiagonal)
    {
        if((mGeometries[mGeometries.size()-1] == GUIConnector::DIAGONAL) or ((mGeometries[mGeometries.size()-2] == GUIConnector::DIAGONAL)))
        {
            //if(mGeometries[mGeometries.size()-3] == GUIConnector::HORIZONTAL)
            if(abs(mPoints[mPoints.size()-3].x() - mPoints[mPoints.size()-4].x()) > abs(mPoints[mPoints.size()-3].y() - mPoints[mPoints.size()-4].y()))
            {
                mGeometries[mGeometries.size()-2] = GUIConnector::HORIZONTAL;
                mGeometries[mGeometries.size()-1] = GUIConnector::VERTICAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-1].y());
            }
            else
            {
                mGeometries[mGeometries.size()-2] = GUIConnector::VERTICAL;
                mGeometries[mGeometries.size()-1] = GUIConnector::HORIZONTAL;
                mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-1].x(), mPoints[mPoints.size()-3].y());
            }
        }
    }
    else if(getNumberOfLines() == 3 and !mMakingDiagonal)
    {
        if(getStartPort()->getPortDirection() == PortAppearance::HORIZONTAL)
        {
            mGeometries[1] = GUIConnector::HORIZONTAL;
            mGeometries[0] = GUIConnector::VERTICAL;
            mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
        }
        else
        {
            mGeometries[1] = GUIConnector::VERTICAL;
            mGeometries[0] = GUIConnector::HORIZONTAL;
            mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
        }
    }

    if(mPoints.size() == 2 and !mMakingDiagonal)
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


//! Sets the pointer to the start port of a connector.
//! @param *port is the pointer to the new start port.
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setStartPort(GUIPort *port)
{
    mpStartPort = port;
    mpStartPort->isConnected = true;
    connect(this->mpStartPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(this->mpStartPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));
}


//! Sets the pointer to the end port of a connector, and executes the final tasks before creation of the connetor is complete. Then flags that the end port is connected.
//! @param *port is the pointer to the new end port.
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    //! @todo Make connectors add one extra line if end port has wrong direction, and move the line X points outwards from the component.
    mEndPortConnected = true;
    mpEndPort = port;
    mpEndPort->isConnected = true;

//    if(mpStartPort->getPortType() == "POWERPORT" or mpEndPort->getPortType() == "POWERPORT")
//    {
//        this->mpGUIConnectorAppearance->setType("POWERPORT");
//    }

    if( ( ((mpEndPort->getPortDirection() == PortAppearance::HORIZONTAL) and (mGeometries.back() == GUIConnector::HORIZONTAL)) or
          ((mpEndPort->getPortDirection() == PortAppearance::VERTICAL) and (mGeometries.back() == GUIConnector::VERTICAL)) ) or
          (mGeometries[mGeometries.size()-2] == GUIConnector::DIAGONAL))
    {
            //Wrong direction of last line, so remove last point. It will be fine.
        this->removePoint();
    }
    else
    {
            //Move second last line a bit outwards from the component
        if(mpEndPort->getPortDirection() == PortAppearance::HORIZONTAL and mpEndPort->getGuiObject()->mapToScene(mpEndPort->getGuiObject()->boundingRect().center()).x() > mpEndPort->scenePos().x())
        {
            mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x() - 20, mPoints[mPoints.size()-2].y());
            mPoints[mPoints.size()-3] = QPointF(mPoints[mPoints.size()-3].x() - 20, mPoints[mPoints.size()-3].y());
        }
        else if(mpEndPort->getPortDirection() == PortAppearance::HORIZONTAL and mpEndPort->getGuiObject()->mapToScene(mpEndPort->getGuiObject()->boundingRect().center()).x() < mpEndPort->scenePos().x())
        {
            mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x() + 20, mPoints[mPoints.size()-2].y());
            mPoints[mPoints.size()-3] = QPointF(mPoints[mPoints.size()-3].x() + 20, mPoints[mPoints.size()-3].y());
        }
        else if(mpEndPort->getPortDirection() == PortAppearance::VERTICAL and mpEndPort->getGuiObject()->mapToScene(mpEndPort->getGuiObject()->boundingRect().center()).y() > mpEndPort->scenePos().y())
        {
            mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(), mPoints[mPoints.size()-2].y() - 20);
            mPoints[mPoints.size()-3] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-3].y() - 20);
        }
        else if(mpEndPort->getPortDirection() == PortAppearance::VERTICAL and mpEndPort->getGuiObject()->mapToScene(mpEndPort->getGuiObject()->boundingRect().center()).y() < mpEndPort->scenePos().y())
        {
            mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(), mPoints[mPoints.size()-2].y() + 20);
            mPoints[mPoints.size()-3] = QPointF(mPoints[mPoints.size()-3].x(), mPoints[mPoints.size()-3].y() + 20);
        }
        this->drawConnector();
        //this->mpParentGraphicsView->setBackgroundBrush(this->mpParentGraphicsView->mBackgroundColor);
        this->mpParentGraphicsView->resetBackgroundBrush();
    }

    this->updateEndPoint(port->mapToScene(port->boundingRect().center()));
    connect(this->mpEndPort->getGuiObject(),SIGNAL(componentDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(this->mpEndPort->getGuiObject(),SIGNAL(componentSelected()),this,SLOT(selectIfBothComponentsSelected()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(int i=1; i!=mpLines.size()-1; ++i)
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    for(int i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

//        //Add arrow to the connector if it is of signal type
//    if(port->getPortType() == "READPORT" && port->getNodeType() == "NodeSignal")
//        this->getLastLine()->addEndArrow();
//    else if(port->getPortType() == "WRITEPORT" && port->getNodeType() == "NodeSignal")
//        this->mpLines[0]->addStartArrow();

    emit endPortConnected();
    this->determineAppearance();
    this->setPassive();
}


////! Cycles all lines and gives them the specified pen styles.
////! @param primaryPen defines the default width and color of the line.
////! @param activePen defines the width and color of the line when it is selected.
////! @param hoverPen defines the width and color of the line when hovered by the mouse cursor.
//void GUIConnector::setPens(QPen primaryPen, QPen activePen, QPen hoverPen)
//{
//    for (std::size_t i=0; i!=mpLines.size(); ++i )
//    {
//        mpLines[i]->setPens(primaryPen, activePen, hoverPen);
//    }
//}


//! Slot that tells the connector lines whether or not to use ISO style.
//! @param useISO is true if ISO style shall be used.
void GUIConnector::setIsoStyle(bool useISO)
{
    mpGUIConnectorAppearance->setIsoStyle(useISO);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! Slot that tells the lines to adjust their size to the zoom factor. This is to make sure line will not become invisible when zooming out.
void GUIConnector::adjustToZoom()
{
    mpGUIConnectorAppearance->adjustToZoom(mpParentGraphicsView->mZoomFactor);
    for (int i=0; i!=mpLines.size(); ++i )
    {
        //Refresh each line by setting to passive (primary) appearance
        mpLines[i]->setPassive();
    }
}


//! Returns the number of lines in a connector.
int GUIConnector::getNumberOfLines()
{
    return mpLines.size();
}


//! Returns the geometry type of the specified line.
//! @param lineNumber is the number of the specified line in the mpLines vector.
GUIConnector::geometryType GUIConnector::getGeometry(int lineNumber)
{
    return mGeometries[lineNumber];
}


//! Returns the point vector used by the connector.
QVector<QPointF> GUIConnector::getPointsVector()
{
    return mPoints;
}


QPointF GUIConnector::getStartPoint()
{
    if(mPoints.empty())
        assert(false);
    else
    {
        if(mPoints.first()==this->mpLines[0]->startPos)
            return mPoints.first();
        else
            return mPoints.last();
    }
}


QPointF GUIConnector::getEndPoint()
{
    if(mPoints.empty())
        assert(false);
    else
    {
        if(mPoints.last()==this->getLastLine()->endPos)
            return mPoints.last();
        else
            return mPoints.first();
    }
}


//! Returns a pointer to the start port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getEndPort()
GUIPort *GUIConnector::getStartPort()
{
    return this->mpStartPort;
}


//! Returns a pointer to the end port of a connector.
//! @see setStartPort(GUIPort *port)
//! @see setEndPort(GUIPort *port)
//! @see getStartPort()
GUIPort *GUIConnector::getEndPort()
{
    return this->mpEndPort;
}


//! Returns the line with specified number.
//! @param line is the number of the wanted line.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLastLine()
GUIConnectorLine *GUIConnector::getLine(int line)
{
    return mpLines[line];
}


//! Returns the last line of the connector.
//! @see getThirdLastLine()
//! @see getSecondLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getLastLine()
{
    return mpLines[mpLines.size()-1];
}


//! Returns the second last line of the connector.
//! @see getThirdLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getSecondLastLine()
{
    return mpLines[mpLines.size()-2];
}


//! Returns the third last line of the connector.
//! @see getSecondLastLine()
//! @see getLastLine()
//! @see getLine(int line)
GUIConnectorLine *GUIConnector::getThirdLastLine()
{
    return mpLines[mpLines.size()-3];
}


//! Returns true if the connector is connected at both ends, otherwise false.
bool GUIConnector::isConnected()
{
    //qDebug() << "Entering isConnected()";
    //return (getStartPort()->isConnected and getEndPort()->isConnected);
    return (getStartPort()->isConnected and mEndPortConnected);
}


//! Returns true if the line currently being drawn is a diagonal one, otherwise false.
//! @see makeDiagonal(bool enable)
bool GUIConnector::isMakingDiagonal()
{
    return mMakingDiagonal;
}


//! Returns true if the connector is active ("selected").
bool GUIConnector::isActive()
{
    return mIsActive;
}


//! Saves all necessary information about the connetor to a text stream. Used for save, undo and copy operations.
//! @param QTextSream is the text stream with the information.
void GUIConnector::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QString startObjName = getStartPort()->getGuiObject()->getName();
    QString endObjName = getEndPort()->getGuiObject()->getName();
    QString startPortName  = getStartPort()->getName();
    QString endPortName = getEndPort()->getName();
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }
    rStream << ( addQuotes(startObjName) + " " + addQuotes(startPortName) + " " + addQuotes(endObjName) + " " + addQuotes(endPortName) );
    for(int j = 0; j != mPoints.size(); ++j)
    {
        rStream << " " << mPoints[j].x() << " " << mPoints[j].y();
    }
    rStream << "\n";
}


//! Draws lines between the points in the mPoints vector, and stores them in the mpLines vector.
void GUIConnector::drawConnector()
{
    if(!mEndPortConnected)
    {
        //End port is not connected, which means we are creating a new line
        //! @todo Make this smarter, so that lines that are not changed are not removed and then re-added

            //Remove all lines
        while(!mpLines.empty())
        {
            this->scene()->removeItem(mpLines.back());
            mpLines.pop_back();
        }
        mpLines.clear();

            //Create new lines from the mPoints vector
        if(mPoints.size() > 1)
        {
            for(int i = 0; i != mPoints.size()-1; ++i)
            {
                mpTempLine = new GUIConnectorLine(mapFromScene(mPoints[i]).x(), mapFromScene(mPoints[i]).y(),
                                                  mapFromScene(mPoints[i+1]).x(), mapFromScene(mPoints[i+1]).y(),
                                                  mpGUIConnectorAppearance, mpLines.size(), this);
                mpTempLine->setPassive();
                connect(mpTempLine,SIGNAL(lineSelected(bool, int)),this,SLOT(doSelect(bool, int)));
                connect(mpTempLine,SIGNAL(lineMoved(int)),this, SLOT(updateLine(int)));
                connect(mpTempLine,SIGNAL(lineHoverEnter()),this,SLOT(setHovered()));
                connect(mpTempLine,SIGNAL(lineHoverLeave()),this,SLOT(setUnHovered()));
                connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
                mpLines.push_back(mpTempLine);
            }
        }
        if(!mEndPortConnected && mpLines.size() > 1)
        {
            //mpLines.back()->setActive();
            mpLines[mpLines.size()-2]->setPassive();
        }
    }
    else
    {
        if(mpStartPort->getGuiObject()->isSelected() and mpEndPort->getGuiObject()->isSelected() and this->isActive())
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

            //Redraw the lines based on the mPoints vector
        for(int i = 0; i != mPoints.size()-1; ++i)
        {
            mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
        }
    }

        //Remove the extra lines if there are too many
    while(mPoints.size() < int(mpLines.size()+1))
    {
        delete(mpLines.back());
        mpLines.pop_back();
        this->scene()->update();
    }

    //mpParentGraphicsView->setBackgroundBrush(mpParentGraphicsView->mBackgroundColor);
    mpParentGraphicsView->resetBackgroundBrush();
}


//! Updates the first point of the connector, and adjusts the second point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateStartPoint(QPointF point)
{
    if(mPoints.size() == 0)
        mPoints.push_back(point);
    else
        mPoints[0] = point;

    if(mPoints.size() != 1)
    {
        if(mGeometries[0] == GUIConnector::HORIZONTAL)
            mPoints[1] = QPointF(mPoints[0].x(),mPoints[1].y());
        else if(mGeometries[0] == GUIConnector::VERTICAL)
            mPoints[1] = QPointF(mPoints[1].x(),mPoints[0].y());
    }
}


//! Updates the last point of the connector, and adjusts the second last point accordingly depending on the geometry vector.
//! @param point is the new start point.
//! @see updateEndPoint(QPointF point)
void GUIConnector::updateEndPoint(QPointF point)
{
    mPoints.back() = point;
    if(mGeometries.back() == GUIConnector::HORIZONTAL)
    {
        mPoints[mPoints.size()-2] = QPointF(point.x(),mPoints[mPoints.size()-2].y());
    }
    else if(mGeometries.back() == GUIConnector::VERTICAL)
    {
        mPoints[mPoints.size()-2] = QPointF(mPoints[mPoints.size()-2].x(),point.y());
    }
}


//! Updates the mPoints vector when a line has been moved. Used to make lines follow each other when they are moved, and to make sure horizontal lines can only move vertically and vice versa.
//! @param lineNumber is the number of the line that has moved.
void GUIConnector::updateLine(int lineNumber)
{
   if ((mEndPortConnected) && (lineNumber != 0) && (lineNumber != int(mpLines.size())))
    {
        if(mGeometries[lineNumber] == GUIConnector::HORIZONTAL)
        {
            mPoints[lineNumber] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).x(), mPoints[lineNumber].y());
            mPoints[lineNumber+1] = QPointF(getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).x(), mPoints[lineNumber+1].y());
        }
        else if (mGeometries[lineNumber] == GUIConnector::VERTICAL)
        {
            mPoints[lineNumber] = QPointF(mPoints[lineNumber].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p1()).y());
            mPoints[lineNumber+1] = QPointF(mPoints[lineNumber+1].x(), getLine(lineNumber)->mapToScene(getLine(lineNumber)->line().p2()).y());
        }
    }

    drawConnector();
}


//! Slot that moves all points in the connector a specified distance in a specified direction. This is used in copy-paste operations.
//! @param offsetX is the distance to move in X direction.
//! @param offsetY is the distance to move in Y direction.
void GUIConnector::moveAllPoints(qreal offsetX, qreal offsetY)
{
    for(int i=0; i != mPoints.size(); ++i)
    {
        mPoints[i] = QPointF(mPoints[i].x()+offsetX, mPoints[i].y()+offsetY);
    }
}


//! Tells the connector to create one diagonal lines instead of the last two horizontal/vertical, or to return to horizontal/diagonal mode.
//! @param enable indicates whether diagonal mode shall be enabled or disabled.
//! @see isMakingDiagonal()
void GUIConnector::makeDiagonal(bool enable)
{
    QCursor cursor;
    if(enable)
    {
        mMakingDiagonal = true;
        removePoint();
        mGeometries.back() = GUIConnector::DIAGONAL;
        mPoints.back() = mpParentGraphicsView->mapToScene(mpParentGraphicsView->mapFromGlobal(cursor.pos()));
        drawConnector();
    }
    else
    {
        if(this->getNumberOfLines() > 1)
        {
            if(mGeometries[mGeometries.size()-2] == GUIConnector::HORIZONTAL)
            {
                mGeometries.back() = GUIConnector::VERTICAL;
                mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
            }
            else if(mGeometries[mGeometries.size()-2] == GUIConnector::VERTICAL)
            {
                mGeometries.back() = GUIConnector::HORIZONTAL;
                mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
            }
            else if(mGeometries[mGeometries.size()-2] == GUIConnector::DIAGONAL)
            {
                if(abs(mPoints[mPoints.size()-2].x() - mPoints[mPoints.size()-3].x()) > abs(mPoints[mPoints.size()-2].y() - mPoints[mPoints.size()-3].y()))
                {
                    mGeometries.back() = GUIConnector::HORIZONTAL;
                    mPoints.back() = QPointF(mPoints[mPoints.size()-2].x(), mPoints.back().y());
                }
                else
                {
                    mGeometries.back() = GUIConnector::VERTICAL;
                    mPoints.back() = QPointF(mPoints.back().x(), mPoints[mPoints.size()-2].y());
                }

            }
            addPoint(mpParentGraphicsView->mapToScene(mpParentGraphicsView->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentGraphicsView->mapToScene(mpParentGraphicsView->mapFromGlobal(cursor.pos())));
            if(getStartPort()->getPortDirection() == PortAppearance::HORIZONTAL)
            {
                mGeometries[0] = GUIConnector::VERTICAL;
                mGeometries[1] = GUIConnector::HORIZONTAL;
                mPoints[1] = QPointF(mPoints[2].x(), mPoints[0].y());
            }
            else
            {
                mGeometries[0] = GUIConnector::HORIZONTAL;
                mGeometries[1] = GUIConnector::VERTICAL;
                mPoints[1] = QPointF(mPoints[0].x(), mPoints[2].y());
            }
        }

        drawConnector();
        mMakingDiagonal = false;
    }
}


//! Slot that activates or deactivates the connector if one of its lines is selected or deselected.
//! @param lineSelected tells whether the signal was induced by selection or deselection of a line.
//! @see setActive()
//! @see setPassive()
void GUIConnector::doSelect(bool lineSelected, int lineNumber)
{
    if(this->mEndPortConnected)     //Non-finished lines shall not be selectable
    {
        if(lineSelected)
        {
            this->setActive();
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(i != lineNumber)     //I think this means that only one line in a connector can be selected at one time
                   mpLines[i]->setSelected(false);
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
            }
       }
    }
}


//! Slot that selects a connector if both its components are selected.
//! @see doSelect(bool lineSelected, int lineNumber)
void GUIConnector::selectIfBothComponentsSelected()
{
    if(mEndPortConnected and mpStartPort->getGuiObject()->isSelected() and mpEndPort->getGuiObject()->isSelected())
    {
        this->mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


//! Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = true;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setActive();
            //mpLines[i]->setSelected(true);         //???
        }
    }
}


//! Deactivates a connector, deactivates each line and disconnects delete function with delete key.
//! @see setActive()
void GUIConnector::setPassive()
{
    disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
    if(this->mEndPortConnected)
    {
        mIsActive = false;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
            mpLines[i]->setSelected(false);       //OBS! Kanske inte blir bra...
        }
    }
}


//! Changes connector style to hovered if it is not active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setHovered();
        }
    }
}


//! Changes connector style back to normal if it is not active. Used when mouse stops hovering a line.
//! @see setHovered()
//! @see setPassive()
void GUIConnector::setUnHovered()
{
    if(this->mEndPortConnected && !this->mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
        }
    }
}


//! Asks my parent to delete myself.
void GUIConnector::deleteMe()
{
    //qDebug() << "deleteMe()";
    mpParentGraphicsView->removeConnector(this, false);
}


//! Asks my parent to delete myself, and tells it to not add me to the undo stack.
void GUIConnector::deleteMeWithNoUndo()
{
    mpParentGraphicsView->removeConnector(this, true);
}

//! Uppdate the appearance of the connector (setting its type and line endings)
//! @todo right now this only set the type and ending arrows, maybe should handle ALLA appearance update like switching when howering, or maybe have two different update appearance functions (this one only needs to be run once when a conector is created)
void GUIConnector::determineAppearance()
{

    if(mpStartPort->getPortType() == "POWERPORT" or mpEndPort->getPortType() == "POWERPORT")
    {
        this->mpGUIConnectorAppearance->setType("POWERPORT");
    }
    else if (mpStartPort->getPortType() == "READPORT" or mpEndPort->getPortType() == "READPORT")
    {
        this->mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else if (mpStartPort->getPortType() == "WRITEPORT" or mpEndPort->getPortType() == "WRITEPORT")
    {
        this->mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else
    {
        //! @todo this maight be bad if unknown not handled
        this->mpGUIConnectorAppearance->setType("UNKNOWN");
    }

    //Add arrow to the connector if it is of signal type
    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        this->getLastLine()->addEndArrow();
    }
    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        //Assumes that the startport was a read port
        this->mpLines[0]->addStartArrow();
    }

    //Run this to actually change the pen
    this->setPassive(); //!< @todo Not sure if setPassive is allways correct, but it is a good guess
}

//------------------------------------------------------------------------------------------------------------------------//


//! Constructor.
//! @param x1 is the x-coordinate of the start position of the line.
//! @param y1 is the y-coordinate of the start position of the line.
//! @param x2 is the x-coordinate of the end position of the line.
//! @param y2 is the y-coordinate of the end position of the line.
////! @param primaryPen defines the default color and width of the line.
////! @param activePen defines the color and width of the line when it is selected.
////! @param hoverPen defines the color and width of the line when it is hovered by the mouse cursor.
//! @param pConnApp A pointer to the connector appearance data, containing pens
//! @param lineNumber is the number of the line in the connector.
//! @param *parent is a pointer to the parent object.
GUIConnectorLine::GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, GUIConnectorAppearance* pConnApp, int lineNumber, GUIConnector *parent)
        : QGraphicsLineItem(x1,y1,x2,y2,parent)
{
    mpParentGUIConnector = parent;
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    mpConnectorAppearance = pConnApp;
    this->mLineNumber = lineNumber;
    this->setAcceptHoverEvents(true);
    this->mParentConnectorEndPortConnected = false;
    this->startPos = QPointF(x1,y1);
    this->endPos = QPointF(x2,y2);
    //this->mpParentGUIConnector->mGeometries.push_back(GUIConnector::HORIZONTAL);
    this->mHasStartArrow = false;
    this->mHasEndArrow = false;
    this->mArrowSize = 8.0;
    this->mArrowAngle = 0.5;
}


//! Destructor
GUIConnectorLine::~GUIConnectorLine()
{
}


//! Reimplementation of paint function. Removes the ugly dotted selection box.
void GUIConnectorLine::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    QStyleOptionGraphicsItem *_o = const_cast<QStyleOptionGraphicsItem*>(o);
    _o->state &= ~QStyle::State_Selected;
    QGraphicsLineItem::paint(p,_o,w);
}


//! Changes the style of the line to active.
//! @see setPassive()
//! @see setHovered()
void GUIConnectorLine::setActive()
{
        this->setPen(mpConnectorAppearance->getPen("Active"));
}


//! Changes the style of the line to default.
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
}


//! Changes the style of the line to hovered.
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mpConnectorAppearance->getPen("Hover"));
}


//! Defines what shall happen if a mouse key is pressed while hovering a connector line.
void GUIConnectorLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //emit lineClicked();
    if(event->button() == Qt::LeftButton)
    {
        mOldPos = this->pos();
    }
    QGraphicsLineItem::mousePressEvent(event);
}


//! Defines what shall happen if a mouse key is released while hovering a connector line.
void GUIConnectorLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if((this->pos() != mOldPos) and (event->button() == Qt::LeftButton))
    {
        mpParentGUIConnector->mpParentGraphicsView->undoStack->newPost();
        mpParentGUIConnector->mpParentGraphicsView->mpParentProjectTab->hasChanged();
        mpParentGUIConnector->mpParentGraphicsView->undoStack->registerModifiedConnector(mOldPos, this->pos(), mpParentGUIConnector, getLineNumber());
    }
    QGraphicsLineItem::mouseReleaseEvent(event);
}

//! Devines what shall happen if the mouse cursor enters the line. Change cursor if the line is movable.
//! @see hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(this->flags().testFlag((QGraphicsItem::ItemIsMovable)))
    {
        if(this->mParentConnectorEndPortConnected && this->mpParentGUIConnector->getGeometry(getLineNumber()) == GUIConnector::VERTICAL)
        {
            this->setCursor(Qt::SizeVerCursor);
        }
        else if(this->mParentConnectorEndPortConnected && this->mpParentGUIConnector->getGeometry(getLineNumber()) == GUIConnector::HORIZONTAL)
        {
            this->setCursor(Qt::SizeHorCursor);
        }
    }
    emit lineHoverEnter();
}


//! Defines what shall happen when mouse cursor leaves the line.
//! @see hoverEnterEvent(QGraphicsSceneHoverEvent *event)
void GUIConnectorLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    emit lineHoverLeave();
}


//! Returns the number of the line in the connector.
int GUIConnectorLine::getLineNumber()
{
    return mLineNumber;
}


//! Defines what shall happen if the line is selected or moved.
QVariant GUIConnectorLine::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
         emit lineSelected(this->isSelected(), this->mLineNumber);
    }
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit lineMoved(this->mLineNumber);
    }
    return value;
}


//! Tells the line that its parent connector has been connected at both ends
void GUIConnectorLine::setConnected()
{
    mParentConnectorEndPortConnected = true;
}


//! Reimplementation of setLine; stores the start and end positions before changing them
//! @param x1 is the x-coordinate of the start position.
//! @param y1 is the y-coordinate of the start position.
//! @param x2 is the x-coordinate of the end position.
//! @param y2 is the y-coordinate of the end position.
void GUIConnectorLine::setLine(QPointF pos1, QPointF pos2)
{
    this->startPos = this->mapFromParent(pos1);
    this->endPos = this->mapFromParent(pos2);
    if(this->mHasEndArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addEndArrow();
    }
    else if(this->mHasStartArrow)
    {
        delete(this->mArrowLine1);
        delete(this->mArrowLine2);
        this->addStartArrow();
    }
    QGraphicsLineItem::setLine(this->mapFromParent(pos1).x(),this->mapFromParent(pos1).y(),
                               this->mapFromParent(pos2).x(),this->mapFromParent(pos2).y());
}


//! Adds an arrow at the end of the line.
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
    this->mHasEndArrow = true;
}


//! Adds an arrow at the start of the line.
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
    this->mHasStartArrow = true;
}


//! Reimplementation of inherited setPen function to include arrow pen too.
void GUIConnectorLine::setPen (const QPen &pen)
{
    QGraphicsLineItem::setPen(pen);
    if(this->mHasStartArrow | this->mHasEndArrow)       //Update arrow lines two, but ignore dashes
    {
        QPen tempPen = this->pen();
        tempPen = QPen(tempPen.color(), tempPen.width(), Qt::SolidLine);
        mArrowLine1->setPen(tempPen);
        mArrowLine2->setPen(tempPen);
        mArrowLine1->line();
    }
}


////! Set function for all three pen styles (primary, active and hover).
//void GUIConnectorLine::setPens(QPen primaryPen, QPen activePen, QPen hoverPen)
//{
//    mPrimaryPen = primaryPen;
//    mActivePen = activePen;
//    mHoverPen = hoverPen;
//    this->setPassive();
//}
