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
#include "ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "loadObjects.h"
#include <math.h>

//! @brief Constructor for connector class
//! @param startpos defines the start position of the connector, normally the center of the starting port.
//! @param *parentView is a pointer to the GraphicsView the connector belongs to.
//! @param parent is the parent of the port.
GUIConnector::GUIConnector(GUIPort *startPort, GUISystem *parentSystem, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentSystem = parentSystem;
    mpParentSystem->getContainedScenePtr()->addItem(this);
    startPort->getGuiModelObject()->rememberConnector(this);
    qDebug() << "startPort->getGuiObject()->getName(): " << startPort->getGuiModelObject()->getName();

    setFlags(QGraphicsItem::ItemIsFocusable);

    connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange()), this, SLOT(adjustToZoom()));
    connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
    connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIsoStyle(graphicsType)));

    QPointF startPos = startPort->mapToScene(startPort->boundingRect().center());
    this->setPos(startPos);
    this->updateStartPoint(startPos);
    mpGUIConnectorAppearance = new GUIConnectorAppearance("notconnected", mpParentSystem->mGfxType);
    mEndPortConnected = false;
    this->drawConnector();
    mMakingDiagonal = false;
    this->setStartPort(startPort);
    this->addPoint(startPos);
    this->addPoint(startPos);
}


//! @brief Constructor used to create a whole connector at once. Used when for example loading models.
//! @param *startPort Pointer to the start port
//! @param *endPort Pointer to the end port
//! @param points Point vector for the connector
//! @param *parentView Pointer to the GraphicsView the connector belongs to
//! @param *parent Pointer to parent of the port
GUIConnector::GUIConnector(GUIPort *startPort, GUIPort *endPort, QVector<QPointF> points, GUISystem *parentSystem, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentSystem = parentSystem;
    setFlags(QGraphicsItem::ItemIsFocusable);
    mpStartPort = startPort;
    mpEndPort = endPort;
    mpStartPort->setIsConnected(true);
    mpEndPort->setIsConnected(true);
    connect(mpStartPort->getGuiModelObject(),SIGNAL(objectSelected()),this,SLOT(selectIfBothComponentsSelected()));
    connect(mpEndPort->getGuiModelObject(),SIGNAL(objectSelected()),this,SLOT(selectIfBothComponentsSelected()));
    QPointF startPos = getStartPort()->mapToScene(getStartPort()->boundingRect().center());
    this->setPos(startPos);

    mpGUIConnectorAppearance = new GUIConnectorAppearance(startPort->getPortType(), mpParentSystem->mGfxType);

    mPoints = points;

        //Setup the geometries vector based on the point geometry
    for(int i=0; i < mPoints.size()-1; ++i)
    {
        if(mPoints[i].x() == mPoints[i+1].x())
            mGeometries.push_back(HORIZONTAL);
        else if(mPoints[i].y() == mPoints[i+1].y())
            mGeometries.push_back(VERTICAL);
        else
            mGeometries.push_back(DIAGONAL);
    }

    mEndPortConnected = true;
    emit endPortConnected();
    this->setPassive();
    connect(mpEndPort->getGuiModelObject(),SIGNAL(objectDeleted()),this,SLOT(deleteMeWithNoUndo()));

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
        connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
    }

    this->determineAppearance();
    this->drawConnector();

        //Make all lines selectable and all lines except first and last movable
    for(int i=1; i<mpLines.size()-1; ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    for(int i=0; i<mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);



//      //Add arrow to the connector if it is of signal type
//    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        this->getLastLine()->addEndArrow();
//    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
//        mpLines[0]->addStartArrow();

    mpStartPort->getGuiModelObject()->rememberConnector(this);
    mpEndPort->getGuiModelObject()->rememberConnector(this);

    connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(zoomChange(qreal)), this, SLOT(adjustToZoom(qreal)));
    connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
    connect(mpParentSystem, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIsoStyle(graphicsType)));
}


//! @brief Destructor for connector class
GUIConnector::~GUIConnector()
{
    delete mpGUIConnectorAppearance;

    mpStartPort->getGuiModelObject()->forgetConnector(this);
    if(mEndPortConnected)
    {
        mpEndPort->getGuiModelObject()->forgetConnector(this);
    }
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
    mpStartPort = port;
    mpStartPort->setIsConnected(true);
    connect(mpStartPort->getGuiModelObject(),SIGNAL(objectDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(mpStartPort->getGuiModelObject(),SIGNAL(objectSelected()),this,SLOT(selectIfBothComponentsSelected()));
}


//! @brief Sets the pointer to the end port of a connector, and executes the final tasks before creation of the connetor is complete. Then flags that the end port is connected.
//! @param *port Pointer to the new end port
//! @see setStartPort(GUIPort *port)
//! @see getStartPort()
//! @see getEndPort()
void GUIConnector::setEndPort(GUIPort *port)
{
    mEndPortConnected = true;
    mpEndPort = port;
    mpEndPort->setIsConnected(true);

    if( ( ((mpEndPort->getPortDirection() == LEFTRIGHT) && (mGeometries.back() == HORIZONTAL)) ||
          ((mpEndPort->getPortDirection() == TOPBOTTOM) && (mGeometries.back() == VERTICAL)) ) ||
          (mGeometries[mGeometries.size()-2] == DIAGONAL))
    {
            //Wrong direction of last line, so remove last point. It will be fine.
        this->removePoint();
        this->mpLines.pop_back();
    }
    else
    {
            //Move second last line a bit outwards from the component
        QPointF offsetPoint = getOffsetPointfromPort(mpEndPort);
        mPoints[mPoints.size()-2] += offsetPoint;
        mPoints[mPoints.size()-3] += offsetPoint;
        this->drawConnector();
        //mpParentSystem->setBackgroundBrush(mpParentSystem->mBackgroundColor);
        mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

    this->updateEndPoint(port->mapToScene(port->boundingRect().center()));
    connect(mpEndPort->getGuiModelObject(),SIGNAL(objectDeleted()),this,SLOT(deleteMeWithNoUndo()));
    connect(mpEndPort->getGuiModelObject(),SIGNAL(objectSelected()),this,SLOT(selectIfBothComponentsSelected()));

        //Make all lines selectable and all lines except first and last movable
    if(mpLines.size() > 1)
    {
        for(int i=1; i!=mpLines.size()-1; ++i)
            mpLines[i]->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    for(int i=0; i!=mpLines.size(); ++i)
        mpLines[i]->setFlag(QGraphicsItem::ItemIsSelectable, true);

    emit endPortConnected();
    this->determineAppearance();
    this->setPassive();


        //Snap if close to a snapping position
    if(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mSnapping)
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
        else if( (getNumberOfLines() == 1) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE) ||
                 (getNumberOfLines() < 4) && (abs(mPoints.first().y() - mPoints.last().y()) < SNAPDISTANCE) )
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


//! @brief Slot that tells the lines to adjust their size to the zoom factor.
void GUIConnector::adjustToZoom(qreal zoomfactor)
{
    mpGUIConnectorAppearance->adjustToZoom(zoomfactor);
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
    return mpStartPort->getGUIComponentName();
}


//! @brief Returns the name of the end component of a connector
//! @see getStartComponentName()
QString GUIConnector::getEndComponentName()
{
    return mpEndPort->getGUIComponentName();
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
    return (getStartPort()->isConnected() && mEndPortConnected);
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


//! @brief Saves all necessary information about the connetor to a text stream. Used for save, undo and copy operations.
//! @param QTextSream Text stream with the information
void GUIConnector::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QString startObjName = getStartComponentName();
    QString endObjName = getEndComponentName();
    QString startPortName  = getStartPortName();
    QString endPortName = getEndPortName();
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

void GUIConnector::saveToDomElement(QDomElement &rDomElement)
{
    //Core necessary stuff
    QDomElement xmlConnect = appendDomElement(rDomElement, HMF_CONNECTORTAG);
    appendDomTextNode(xmlConnect, HMF_CONNECTORSTARTCOMPONENTTAG, getStartComponentName());
    appendDomTextNode(xmlConnect, HMF_CONNECTORSTARTPORTTAG, getStartPortName());
    appendDomTextNode(xmlConnect, HMF_CONNECTORENDCOMPONENTTAG, getEndComponentName());
    appendDomTextNode(xmlConnect, HMF_CONNECTORENDPORTTAG, getEndPortName());

    //Save gui data to dom
    QDomElement xmlConnectGUI = appendDomElement(xmlConnect, HMF_HOPSANGUITAG);
    for(size_t j=0; j<mPoints.size(); ++j)
    {
        appendDomValueNode2(xmlConnectGUI, HMF_XYTAG, mPoints[j].x(), mPoints[j].y());
//        appendDomTextNode(xmlConnectGUI, "ptx", mPoints[j].x());
//        appendDomTextNode(xmlConnectGUI, "pty", mPoints[j].y());
    }
}



//! @brief Draws lines between the points in the mPoints vector, and stores them in the mpLines vector
void GUIConnector::drawConnector()
{
    if(!mEndPortConnected)        //End port is not connected, which means we are creating a new line
    {
            //Remove lines if there are too many
        while(mpLines.size() > mPoints.size()-1)
        {
            this->scene()->removeItem(mpLines.back());
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
            connect(this,SIGNAL(endPortConnected()),mpTempLine,SLOT(setConnected()));
            mpLines.push_back(mpTempLine);
        }
    }
    else        //End port is connected, so the connector is modified or has moved
    {
        if(mpStartPort->getGuiModelObject()->isSelected() && mpEndPort->getGuiModelObject()->isSelected() && this->isActive())
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
        if( (mpLines[i]->line().p1() != mPoints[i]) || (mpLines[i]->line().p2() != mPoints[i+1]) );   //Don't redraw the line if it has not changed
        mpLines[i]->setLine(mapFromScene(mPoints[i]), mapFromScene(mPoints[i+1]));
    }

    mpParentSystem->mpParentProjectTab->mpGraphicsView->updateViewPort();
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
   if ((mEndPortConnected) && (lineNumber != 0) && (lineNumber != int(mpLines.size())))
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
    for(int i=0; i != mPoints.size(); ++i)
    {
        mPoints[i] = QPointF(mPoints[i].x()+offsetX, mPoints[i].y()+offsetY);
    }
}


//! @brief Tells the connector to create one diagonal lines instead of the last two horizontal/vertical, or to return to horizontal/diagonal mode.
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
        mPoints.back() = mpParentSystem->mpParentProjectTab->mpGraphicsView->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos()));
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
            addPoint(mpParentSystem->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
        }
        else    //Only one (diagonal) line exist, so special solution is required
        {
            addPoint(mpParentSystem->mapToScene(mpParentSystem->mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos())));
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
    if(mEndPortConnected)     //Non-finished connectors shall not be selectable
    {
        if(lineSelected)
        {
            if(!mpParentSystem->mSelectedSubConnectorsList.contains(this))
            {
                mpParentSystem->mSelectedSubConnectorsList.append(this);
            }
            connect(mpParentSystem, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
            disconnect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
            connect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            this->setActive();
            for (int i=0; i != mpLines.size(); ++i)
            {
               if(i != lineNumber)     //This makes sure that only one line in a connector can be "selected" at one time
               {
                   mpLines[i]->setSelected(false);
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
                mpParentSystem->mSelectedSubConnectorsList.removeOne(this);
                disconnect(mpParentSystem, SIGNAL(deselectAllGUIConnectors()), this, SLOT(deselect()));
                connect(mpParentSystem, SIGNAL(selectAllGUIConnectors()), this, SLOT(select()));
                disconnect(mpParentSystem->mpParentProjectTab->mpGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            }
        }
    }
}


//! @brief Slot that selects a connector if both its components are selected
//! @see doSelect(bool lineSelected, int lineNumber)
void GUIConnector::selectIfBothComponentsSelected()
{
    if(mEndPortConnected && mpStartPort->getGuiModelObject()->isSelected() && mpEndPort->getGuiModelObject()->isSelected())
    {
        mpLines[0]->setSelected(true);
        doSelect(true,0);
    }
}


//! @brief Activates a connector, activates each line and connects delete function with delete key.
//! @see setPassive()
void GUIConnector::setActive()
{
    connect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mEndPortConnected)
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
    disconnect(mpParentSystem, SIGNAL(deleteSelected()), this, SLOT(deleteMe()));
    if(mEndPortConnected)
    {
        mIsActive = false;
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
            mpLines[i]->setSelected(false);       //OBS! Kanske inte blir bra...
        }
    }

    this->setZValue(0);
}


//! @brief Changes connector style to hovered unless it is active. Used when mouse starts hovering a line.
//! @see setUnHovered()
void GUIConnector::setHovered()
{
    if(mEndPortConnected && !mIsActive)
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
    if(mEndPortConnected && !mIsActive)
    {
        for (int i=0; i!=mpLines.size(); ++i )
        {
            mpLines[i]->setPassive();
        }
    }
}


//! @brief Asks the parent system to delete the connector
//! @todo Rename this to something less childish!
void GUIConnector::deleteMe()
{
    //qDebug() << "deleteMe()";
    mpParentSystem->removeConnector(this, UNDO);
}


//! @brief Asks the parent system to delete the connector, and tells it to not add it to the undo stack
//! @todo Rename this to something better
//! @todo Perhaps this function and deleteMe() can be combined to one, wtih UNDO or NOUNDO as input parameter?
void GUIConnector::deleteMeWithNoUndo()
{
    mpParentSystem->removeConnector(this, NOUNDO);
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
//! @todo right now this only set the type and ending arrows, maybe should handle ALLA appearance update like switching when howering, or maybe have two different update appearance functions (this one only needs to be run once when a conector is created)
void GUIConnector::determineAppearance()
{
    //! @todo problem when connecting outside systemport  with internal powerport to readport, will not know that internal port is powerport and line will be signalline
    //! @todo need to figure out a new way to handle this
    if( (mpStartPort->getPortType() == "POWERPORT") || (mpEndPort->getPortType() == "POWERPORT") )
    {
        mpGUIConnectorAppearance->setType("POWERPORT");
    }
    else if( (mpStartPort->getPortType() == "READPORT") || (mpEndPort->getPortType() == "READPORT") )
    {
        mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else if( (mpStartPort->getPortType() == "WRITEPORT") || (mpEndPort->getPortType() == "WRITEPORT") )
    {
        mpGUIConnectorAppearance->setType("SIGNALPORT");
    }
    else
    {
        //! @todo this maight be bad if unknown not handled
        mpGUIConnectorAppearance->setType("UNKNOWN");
    }

    //Add arrow to the connector if it is of signal type
    if(mpEndPort->getPortType() == "READPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        this->getLastLine()->addEndArrow();
    }
    else if(mpEndPort->getPortType() == "WRITEPORT" && mpEndPort->getNodeType() == "NodeSignal")
    {
        //Assumes that the startport was a read port
        mpLines[0]->addStartArrow();
    }

    //Run this to actually change the pen
    this->setPassive(); //!< @todo Not sure if setPassive is allways correct, but it is a good guess
}


//! @brief Slot that "deactivates" a connector if it is deselected
void GUIConnector::deselect()
{
    this->setPassive();
}


//! @brief Slot that "activates" a connector if it is selected
void GUIConnector::select()
{
    this->doSelect(true, -1);
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
        this->mpParentGUIConnector->setZValue(0);
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
}


//! @brief Changes the style of the line to hovered
//! @see setActive()
//! @see setPassive()
void GUIConnectorLine::setHovered()
{
        this->setPen(mpConnectorAppearance->getPen("Hover"));
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
        mpParentGUIConnector->mpParentSystem->mUndoStack->newPost();
        mpParentGUIConnector->mpParentSystem->mpParentProjectTab->hasChanged();
        mpParentGUIConnector->mpParentSystem->mUndoStack->registerModifiedConnector(mOldPos, this->pos(), mpParentGUIConnector, getLineNumber());
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
        mpParentGUIConnector->setZValue(0);
    }
    this->mpParentGUIConnector->setZValue(0);
    emit lineHoverLeave();
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
