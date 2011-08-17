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
//! @file   GUIPort.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPort class
//!
//$Id$

#include <QtGui>
#include <QSvgRenderer>
#include <iostream>

#include "common.h"
#include "GUIPort.h"
#include "GUIConnector.h"

#include "MainWindow.h"
#include "PlotWindow.h"
#include "CoreAccess.h"
#include "GraphicsView.h"
#include "Configuration.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Utilities/GUIUtilities.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/ProjectTabWidget.h"


QPointF getOffsetPointfromPort(GUIPort *pStartPort, GUIPort *pEndPort)
{
    QPointF point;

    if((pEndPort->getPortDirection() == LEFTRIGHT) && (pEndPort->getGuiModelObject()->mapToScene(pEndPort->getGuiModelObject()->boundingRect().center()).x() > pEndPort->scenePos().x()))
    {
        point.setX(-1 * std::min(20.0, abs(pStartPort->scenePos().x()-pEndPort->scenePos().x())/2.0));
    }
    else if((pEndPort->getPortDirection() == LEFTRIGHT) && (pEndPort->getGuiModelObject()->mapToScene(pEndPort->getGuiModelObject()->boundingRect().center()).x() < pEndPort->scenePos().x()))
    {
        point.setX(std::min(20.0, abs(pStartPort->scenePos().x()-pEndPort->scenePos().x())/2.0));
    }
    else if((pEndPort->getPortDirection() == TOPBOTTOM) && (pEndPort->getGuiModelObject()->mapToScene(pEndPort->getGuiModelObject()->boundingRect().center()).y() > pEndPort->scenePos().y()))
    {
        point.setY(-1 * std::min(20.0, abs(pStartPort->scenePos().y()-pEndPort->scenePos().y())/2.0));
    }
    else if((pEndPort->getPortDirection() == TOPBOTTOM) && (pEndPort->getGuiModelObject()->mapToScene(pEndPort->getGuiModelObject()->boundingRect().center()).y() < pEndPort->scenePos().y()))
    {
        point.setY(std::min(20.0, abs(pStartPort->scenePos().y()-pEndPort->scenePos().y())/2.0));
    }
    return point;
}

//! Constructor.
//! @param portName The name of the port
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param QString(ICONPATH) a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(QString portName, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParentGUIModelObject)
    : QGraphicsWidget(pParentGUIModelObject)
{
//    qDebug() << "parentType: " << pParentGUIModelObject->type() << " GUISYSTEM=" << GUISYSTEM << " GUICONTAINER=" << GUICONTAINEROBJECT;
//    qDebug() << "======================= parentName: " << pParentGUIModelObject->getName();

    mpParentGuiModelObject = pParentGUIModelObject;
    mpPortAppearance = pPortAppearance;
    mPortName = portName;

    //Setup port label (ports allways have lables)
    mpPortLabel = new QGraphicsTextItem(this);
    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mpPortLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    mpPortLabel->hide();

    mpMultiPortIconOverlay = 0;
    mpCQSIconOverlay = 0;
    mpMainIcon = new QGraphicsSvgItem(mpPortAppearance->mMainIconPath, this);
    this->setGeometry(0.0, 0.0, mpMainIcon->boundingRect().width(), mpMainIcon->boundingRect().height());
    setTransformOriginPoint(boundingRect().center());     //All rotaion and other transformation should be aplied around the port center

    //Now adjust position
    setCenterPos(xpos, ypos);

    this->setAcceptHoverEvents(true);

    //Setup overlay (if it exists)
    this->refreshPortGraphicsOverlayGraphics();
    this->setRotation(mpPortAppearance->rot);
    this->refreshPortOverlayPosition();

    mMag = GOLDENRATIO;
    mOverlaySetScale = 1.0;
    mIsMagnified = false;

    //Create connections to the parent container object
    //this->refreshParentContainerSigSlotConnections();
    this->setPortOverlayScale(mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab->getGraphicsView()->getZoomFactor());

    if(mpParentGuiModelObject->mpParentContainerObject != 0)
    {
        this->hideIfNotConnected(!mpParentGuiModelObject->mpParentContainerObject->arePortsHidden());
    }

    //Create a permanent connection to the mainwindow buttons and the view zoom change signal for port overlay scaleing
    GraphicsView *pView = mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab->getGraphicsView(); //! @todo need to be able to access this in some nicer way then ptr madness
    connect(gpMainWindow->mpTogglePortsAction,  SIGNAL(triggered(bool)),    this, SLOT(hideIfNotConnected(bool)));
    connect(pView,                          SIGNAL(zoomChange(qreal)),  this, SLOT(setPortOverlayScale(qreal)));
}

GUIPort::~GUIPort()
{
    //If any connectors are present they need to be deleated also
    // We need to use while and access first element every time as Vector will be modified when connector removes itself
    //! @todo What about Undo, right now these deleations are not registerd
    while (mConnectedConnectors.size() > 0)
    {
        mConnectedConnectors[0]->deleteMeWithNoUndo();
    }
    //! @todo Maybe we should use signal and slots instead to handle connector removal on port delete, we are doing that with GuiModelObjects right now

    //We dont need to disconnect the permanent connection to the mainwindow buttons and the view zoom change signal for port overlay scaleing
    //They should be disconnected automatically when the objects die
}

//void GUIPort::refreshParentContainerSigSlotConnections()
//{
//    //! @todo cant we solve this in some other way to avoid the need to refresh connections when moving into groups, OK for now though
//    disconnect(this, SIGNAL(portClicked(GUIPort*)), 0, 0);
//    connect(this, SIGNAL(portClicked(GUIPort*)), this->getParentContainerObjectPtr(), SLOT(createConnector(GUIPort*)));
//}


//! Magnify the port with a class mebmer factor 'mMag'. Is used i.e. at hovering over disconnected port.
//! @param blowup says if the port should be magnified or not.
void GUIPort::magnify(bool doMagnify)
{
    if( doMagnify && !mIsMagnified )
    {
        this->scale(mMag, mMag);
        this->moveBy(-(mMag-1.0)*boundingRect().width()/2.0, -(mMag-1.0)*boundingRect().height()/2.0);
        this->mpPortLabel->moveBy((mMag-1.0)*boundingRect().width()/2.0, (mMag-1.0)*boundingRect().height()/2.0);

        mIsMagnified = true; //This must be set before setPortOverlayScale
        this->setPortOverlayScale(mOverlaySetScale); //Set the scale to the same as current but this time the extra scale factor will be added
    }
    else if ( !doMagnify && mIsMagnified )
    {
        this->moveBy((mMag-1.0)*boundingRect().width()/2.0, (mMag-1.0)*boundingRect().height()/2.0);
        this->mpPortLabel->moveBy(-(mMag-1.0)*boundingRect().width()/2.0, -(mMag-1.0)*boundingRect().height()/2.0);
        this->scale(1.0/mMag,1.0/mMag);

        mIsMagnified = false; //This must be set before setPortOverlayScale
        this->setPortOverlayScale(mOverlaySetScale); //Set the scale to the same as current, no extra magnification will be added
    }
}


//! Reimplemented to call custom show hide instead
void GUIPort::setVisible(bool value)
{
    if (value)
    {
        this->show();
    }
    else
    {
        this->hide();
    }
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //qDebug() << "hovering over port beloning to: " << mpParentGuiModelObject->getName();
    QGraphicsWidget::hoverEnterEvent(event);

    this->setCursor(Qt::CrossCursor);

    magnify(true);
    this->setZValue(1.0);
    mpPortLabel->show();
}

//! @brief Updates the gui port position on its parent object, taking coordinates in parent coordinate system
//! @todo is it necessary to be able to update orientation also?
void GUIPort::setCenterPos(const qreal x, const qreal y)
{
    //Place the guiport with center in x and y, assume x and y in parent local coordinates
    this->setPos(x-boundingRect().width()/2.0, y-boundingRect().height()/2.0);
}

//! Conveniance function when fraction positions are known
void GUIPort::setCenterPosByFraction(qreal x, qreal y)
{
    //qDebug() << "Fraction position: " << x << " " << y;
    //qDebug() << "parent bounding rect: " << mpParentGuiModelObject->boundingRect();
    //qDebug() << "parent icon scene bounding rect" << mpParentGuiObject->mpIcon->sceneBoundingRect();

    //! @todo for now root systems may not have an icon, if icon is empty ports will end up in zero, which is OK, maybe we should always force a default icon
    this->setCenterPos(x*mpParentGuiModelObject->boundingRect().width(), y*mpParentGuiModelObject->boundingRect().height());
}


QPointF GUIPort::getCenterPos()
{
    return this->pos() + QPointF( this->boundingRect().width()/2.0, this->boundingRect().height()/2.0);
}


//! Defines what happens when mouse cursor stops hovering a port.
//! @param *event defines the mouse event.
void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::ArrowCursor);
    magnify(false);

    mpPortLabel->hide();
    this->setZValue(0.0);

    QGraphicsWidget::hoverLeaveEvent(event);
}


//! Defines what happens when clicking on a port.
//! @param *event defines the mouse event.
void GUIPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsSvgItem::mousePressEvent(event); //Don't work if this is called
    if (event->button() == Qt::LeftButton)
    {
        getParentContainerObjectPtr()->createConnector(this);
    }
    else if (event->button() == Qt::RightButton)
    {
        //Nothing
    }
}


//! Defines what happens when double clicking on a port. Nothing should happen.
//! @param *event defines the mouse event.
void GUIPort::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
    //Nothing to do, reimplemented just to do nothing
}


void GUIPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //std::cout << "GUIPort.cpp: " << "contextMenuEvent" << std::endl;

    if ((!this->isConnected()) || (mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getTimeVector(getGuiModelObjectName(), this->getPortName()).empty()))
    {
        event->ignore();
    }
    //Disables the user from plotting MULTIPORTS.
    else if(getPortType() == QString("POWERMULTIPORT"))
    {
        QMenu menu;
        QAction *action;
        action = menu.addAction(QString("Cannot plot MultiPorts"));
        action->setDisabled(true);
        menu.exec(event->screenPos());
    }
    else
    {
        openRightClickMenu(event->screenPos());
    }
}


void GUIPort::openRightClickMenu(QPoint screenPos)
{
    QMenu menu;

    QVector<QString> parameterNames;
    QVector<QString> parameterUnits;
    mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(mpParentGuiModelObject->getName(), this->getPortName(), parameterNames, parameterUnits);

    //QAction *plotPressureAction = menu.addAction("Plot pressure");
    //QAction *plotFlowAction = menu.addAction("Plot flow");
    QVector<QAction *> parameterActions;
    QAction *tempAction;
    for(int i=0; i<parameterNames.size(); ++i)
    {
        tempAction = menu.addAction(QString("Plot "+parameterNames[i]+" ["+parameterUnits[i]+"]"));
        parameterActions.append(tempAction);
    }

    QAction *selectedAction = menu.exec(screenPos);

    for(int i=0; i<parameterNames.size(); ++i)
    {
        if (selectedAction == parameterActions[i])
        {
            //plot(parameterNames[i], parameterUnits[i]);
            plot(parameterNames[i], "");
        }
    }
}



void GUIPort::refreshPortGraphicsOverlayGraphics()
{
    //! @todo maybe put main icon in here also

    //Ok if new appearance is different from previous, then remove old graphics and replace with new one
    //Check CQS overlay
    if (mpPortAppearance->mCQSOverlayPath != mPortAppearanceAfterLastRefresh.mCQSOverlayPath)
    {
        if (mpCQSIconOverlay != 0)
        {
            mpCQSIconOverlay->deleteLater();
        }

        if (mpPortAppearance->mCQSOverlayPath.isEmpty())
        {
            mpCQSIconOverlay = 0;
        }
        else
        {
            //! @todo check if file exist
            mpCQSIconOverlay = new QGraphicsSvgItem(mpPortAppearance->mCQSOverlayPath, this);
            mpCQSIconOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        }
    }

    //Check multiport overlay
    if (mpPortAppearance->mMultiPortOverlayPath != mPortAppearanceAfterLastRefresh.mMultiPortOverlayPath)
    {
        if (mpMultiPortIconOverlay != 0)
        {
            mpMultiPortIconOverlay->deleteLater();
        }

        if (mpPortAppearance->mMultiPortOverlayPath.isEmpty())
        {
            mpMultiPortIconOverlay = 0;
        }
        else
        {
            //! @todo check if file exist
            mpMultiPortIconOverlay = new QGraphicsSvgItem(mpPortAppearance->mMultiPortOverlayPath, this);
            mpMultiPortIconOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        }
    }

    //Port label must exist and be set up before we run setDisplayName
    this->setDisplayName(this->getPortName());
}


//! @brief Refreshes the port overlay graphics and lable position
void GUIPort::refreshPortOverlayPosition()
{
    QTransform transf;
    QPointF pt1, pt2, pt3;

    //Refresh the port label position and orientation
//    //! @todo something wierd with the portlable it seems to be bigger than what you can see in the gui
//    pt1 = this->mpPortLabel->boundingRect().center();
//    transf.rotate(-(this->mpPortLabel->rotation() + this->rotation() + this->mpParentGuiModelObject->rotation()));
//    if (this->mpParentGuiModelObject->isFlipped())
//    {
//        transf.scale(-1.0,1.0);
//    }
//    pt2 =  transf * pt1;
    pt3 = this->boundingRect().center();
    //this->mpPortLabel->setPos(pt3-pt2+QPointF(10, 10)); //! @todo This is little messy, GUIPort::magnify fucks the pos for the label a bit

    qDebug() << "Gemotry: " << this->geometry();
    qDebug() << "bounding rect: " << this->boundingRect();
    qDebug() << "Gemotry.center: " << this->geometry().center();
    qDebug() << "Gemotry.center in scene 0: " << this->mapToScene(this->boundingRect().center());
    pt1 = getGuiModelObject()->mapToScene(this->geometry().center());
    qDebug() << "Gemotry.center in scene 1: " << pt1;

    pt1 = this->mapToScene(this->boundingRect().center());
    pt2 = this->mapFromScene( pt1 + QPointF(4, 4) );
    this->mpPortLabel->setPos(pt2);


    //Put overlay ptrs into a vector to avoid duplicating transformation code, ignore if null
    QVector<QGraphicsSvgItem*> overlayPtrs;
    if (mpCQSIconOverlay != 0)
    {
        overlayPtrs.append(mpCQSIconOverlay);
    }
    if (mpMultiPortIconOverlay != 0)
    {
        overlayPtrs.append(mpMultiPortIconOverlay);
    }
    //Refresh the port overlay graphics
    for(int i=0; i<overlayPtrs.size(); ++i)
    {
        transf.reset();
        pt1 = overlayPtrs[i]->boundingRect().center();
        transf.rotate(-(overlayPtrs[i]->rotation() + this->rotation() + mpParentGuiModelObject->rotation()));
        if (mpParentGuiModelObject->isFlipped())
        {
            transf.scale(-1.0,1.0);
        }
        pt2 =  transf * pt1;
        overlayPtrs[i]->setPos(pt3-pt2);
    }

}


//! @brief recreate the port graphics overlay
//! @todo This needs to be synced and clean up with addPortOverlayGraphics, right now duplicate work, also should not change if icon same as before
//! @todo Maybe we should preload all cqs and multiport overlays (just three of them) and create som kind of shared svg renderer that all ports can share, then we dont need to reload graphics from file every freaking time it changes and in every systemport
void GUIPort::refreshPortGraphics()
{
    qDebug() << "!!! REFRESHING PORT GRAPHICS !!!";

    //Systemports may change appearance depending on what is connected
    if (getPortType() == "SYSTEMPORT")
    {
        QString cqsType;
        if (getGuiModelObject()->getTypeName() == HOPSANGUICONTAINERPORTTYPENAME)
        {
            //If we are port in containerport model object then ask our parent system model object about cqs-type
            //cqsType = getParentContainerObjectPtr()->getTypeCQS();
            cqsType = "NULL"; //Dont show cqs typ internally, it will become confusing
        }
        else
        {
            //If we are external systemport then ask our model object about the cqs-type
            cqsType = getGuiModelObject()->getTypeCQS();
        }
        mpPortAppearance->selectPortIcon(cqsType, getPortType(), getNodeType());
    }


    if (mPortAppearanceAfterLastRefresh.mMainIconPath != mpPortAppearance->mMainIconPath)
    {
        mpMainIcon->deleteLater();
        mpMainIcon = new QGraphicsSvgItem(mpPortAppearance->mMainIconPath, this);
        this->resize(mpMainIcon->boundingRect().width(), mpMainIcon->boundingRect().height());
        setTransformOriginPoint(boundingRect().center());
    }

    this->refreshPortGraphicsOverlayGraphics();

    //Remember current appearance so that we can check what has changed the next time, (to avoid reloading graphics)
    mPortAppearanceAfterLastRefresh = *mpPortAppearance;

    //Make sure connected connectors are refreshed as well
    for(int i=0; i<mConnectedConnectors.size(); ++i)
    {
        mConnectedConnectors[i]->drawConnector();
    }
}

//! @brief Scales the port overlay graphics and tooltip
void GUIPort::setPortOverlayScale(qreal scale)
{
    mOverlaySetScale = scale;   //Remember what scale we are supposed to have
    this->mpPortLabel->setScale(mOverlaySetScale);

    //Should we add extra overlay scale when magnified
    qreal overlayExtraScaleFactor;
    if (mIsMagnified)
    {
        overlayExtraScaleFactor = mMag;
    }
    else
    {
        overlayExtraScaleFactor = 1.0;
    }

    if (mpCQSIconOverlay != 0)
    {
        mpCQSIconOverlay->setScale(mOverlaySetScale * overlayExtraScaleFactor);
    }
    if (mpMultiPortIconOverlay != 0)
    {
        mpMultiPortIconOverlay->setScale(mOverlaySetScale * overlayExtraScaleFactor);
    }

}

//! Returns a pointer to the GraphicsView that the port belongs to.
GUIContainerObject *GUIPort::getParentContainerObjectPtr()
{
    return mpParentGuiModelObject->getParentContainerObject();
}


//! Returns a pointer to the GUIComponent the port belongs to.
GUIModelObject *GUIPort::getGuiModelObject()
{
    return mpParentGuiModelObject;
}


//! Plots the variable with name 'dataName' in the node the port is connected to.
//! @param dataName tells which variable to plot.
//! @param dataUnit sets the unit to show in the plot (has no connection to data, just text).
PlotWindow *GUIPort::plot(QString dataName, QString dataUnit)
{
    if(this->isConnected())
    {
        return gpMainWindow->mpPlotWidget->mpPlotParameterTree->createPlotWindow(mpParentGuiModelObject->getName(), this->getPortName(), dataName, dataUnit);
    }

    return 0;       //Fail!
}


void GUIPort::plotToPlotWindow(PlotWindow *pPlotWindow, QString dataName, QString dataUnit)
{
    pPlotWindow->addPlotCurve(mpParentGuiModelObject->mpParentContainerObject->getNumberOfPlotGenerations()-1, mpParentGuiModelObject->getName(), this->getPortName(), dataName, dataUnit);
}


//! Wrapper for the Core getPortTypeString() function
QString GUIPort::getPortType(const PortTypeIndicationT ind)
{
    if (ind == ACTUALPORTTYPE)
    {
        return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(getGuiModelObjectName(), this->getPortName());
    }
    else /*if (ind == INTERNAL)*/
    {
        return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(getGuiModelObjectName(), this->getPortName(), CoreSystemAccess::INTERNALPORTTYPE);
    }
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getNodeType(getGuiModelObjectName(), this->getPortName());
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getPortName(), rNames, rValues, rUnits);
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getPortName(), rNames, rValuesTxt, rUnits);
}


//void GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<double> values)
//{
//    mpParentContainerObject->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getName(), names, values);
//}

bool GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt)
{
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getPortName(), names, valuesTxt);
}

portDirection GUIPort::getPortDirection()
{
    qreal scene_angle = this->mpParentGuiModelObject->rotation() + this->rotation();
    while(scene_angle > 359)
    {
        scene_angle -= 360;
    }
    qDebug() << "scene_angle = " << scene_angle;
    if( (scene_angle == 0) || (scene_angle == 180) )
    {
        qDebug() << "Returning LEFTRIGHT";
        return LEFTRIGHT;
    }
    else
    {
        qDebug() << "Returning TOPBOTTOM";
        return TOPBOTTOM;
    }
}


void GUIPort::addConnection(GUIConnector *pConnector)
{
    mConnectedConnectors.append(pConnector);
    //qDebug() << "Adding connection, connections = " << mnConnections;
}


void GUIPort::removeConnection(GUIConnector *pConnector)
{
    int idx = mConnectedConnectors.indexOf(pConnector);
    mConnectedConnectors.remove(idx);
    //qDebug() << "Removing connection, connections = " << mnConnections;
}


//! @brief Ask if the port is connected or not
//! @return if the port is connected or not
bool GUIPort::isConnected()
{
    return (mConnectedConnectors.size() > 0);
}


//! @brief Returns a vector with all ports connected to this port.
QVector<GUIPort *> GUIPort::getConnectedPorts()
{
    QVector<GUIPort *> vector;
    for(int i=0; i<mConnectedConnectors.size(); ++i)
    {
        if(mConnectedConnectors.at(i)->getStartPort() == this)
            vector.append(mConnectedConnectors.at(i)->getEndPort());
        else
            vector.append(mConnectedConnectors.at(i)->getStartPort());
    }
    return vector;
}


//! @todo Do we really need both direction and heading
qreal GUIPort::getPortHeading()
{
    return mpPortAppearance->rot;
}



void GUIPort::hide()
{
    this->magnify(false);
    mpPortLabel->hide();
    QGraphicsWidget::hide();
}

void GUIPort::show()
{
    QGraphicsWidget::show();
    mpPortLabel->hide();
}


QString GUIPort::getPortName()
{
    return mPortName;
}

QString GUIPort::getGuiModelObjectName()
{
    return mpParentGuiModelObject->getName();
}


void GUIPort::setDisplayName(const QString name)
{
    mPortName = name;
    //Set the lable, &#160 menas extra white space, to make lable more readable
    QString label("<p><span style=\"background-color:lightyellow\">&#160;&#160;");
    label.append(this->mPortName).append("&#160;&#160;</span></p>");
    mpPortLabel->setHtml(label);
}


bool GUIPort::getLastNodeData(QString dataName, double& rData)
{
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getLastNodeData(getGuiModelObjectName(), this->getPortName(), dataName, rData);
}


//! Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param togglePortsActionTriggered is true if ports shall be hidden, otherwise false.
void GUIPort::hideIfNotConnected(bool togglePortsActionTriggered)
{
    if(mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab == mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab->mpParentProjectTabWidget->getCurrentTab())
    {
        if(!isConnected() && !togglePortsActionTriggered)
        {
            this->hide();
        }
        else if(!isConnected() && togglePortsActionTriggered)
        {
            this->show();
        }
    }
}


GroupPort::GroupPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent)
    : GUIPort(name, xpos, ypos, pPortAppearance, pParent)
{
    //Nothing extra yet
}

//! Overloaded as groups laks core connection
QString GroupPort::getPortType(const PortTypeIndicationT /*ind*/)
{
    //! @todo Return something smart
    return "GropPortType";
}


//! Overloaded as groups laks core connection
QString GroupPort::getNodeType()
{
    //! @todo Return something smart
    return "GropPortNodeType";
}
