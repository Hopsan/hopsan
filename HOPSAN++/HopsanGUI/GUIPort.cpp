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

#include <QtGui>

#include "common.h"

#include "GUIPort.h"
#include "PlotWidget.h"
#include "MainWindow.h"
#include "CoreSystemAccess.h"
#include "GUIObject.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "ProjectTabWidget.h"
#include "GUISystem.h"

using namespace std;

//! Constructor.
//! @param portName The name of the port
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param QString(ICONPATH) a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(QString portName, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIObject *pParentGUIObject, CoreSystemAccess *pGUIRootSystem)
    : QGraphicsSvgItem(pPortAppearance->mIconPath, pParentGUIObject)
{
    qDebug() << "parentType: " << pParentGUIObject->type() << " GUISYSTEM=" << GUISYSTEM;
    qDebug() << "======================= parentName: " << pParentGUIObject->getName();

    if ( pParentGUIObject->mpParentSystem != 0 )
    {
        mpParentSystem = pParentGUIObject->mpParentSystem;
    }
    else if ( pParentGUIObject->type() == GUISYSTEM )
    {
        //Assume that parentGuiObject (which is a asystem) is the root system
        //! @todo not sure that this is really 100% correct
        mpParentSystem = qobject_cast<GUISystem*>(pParentGUIObject);
    }
    else
    {
        qDebug() << "This should not happen";
        assert(false);
    }

    mpParentGuiObject = pParentGUIObject;
    mpPortAppearance = pPortAppearance;
    mpGUIRootSystem = pGUIRootSystem; //Use this to indicate system port

    this->name = portName;

    setTransformOriginPoint(boundingRect().center());

    mXpos = xpos;
    mYpos = ypos;

    updatePosition();

//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    mpPortLabel = new QGraphicsTextItem(this);
    QString label("<p><span style=\"background-color:lightyellow\">");
    label.append(this->getName()).append("</span></p>");
    mpPortLabel->setHtml(label);
    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mpPortLabel->setPos(7.0,7.0);
    mpPortLabel->hide();

    //! @todo this kind of harcoded stuff should not be here, fix the problem in some other way
    if(this->getPortType() == "POWERPORT")
    {
        this->setRotation(0.0);
        mpPortLabel->setRotation(0.0);
    }
    else
    {
        this->setRotation(mpPortAppearance->rot);
        mpPortLabel->setRotation(-mpPortAppearance->rot);
    }

    mMag = GOLDENRATIO;
    mIsMag = false;
    isConnected = false;

    MainWindow *pMainWindow = mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentSystem(),SLOT(createConnector(GUIPort*)));
    connect(pMainWindow->hidePortsAction,SIGNAL(triggered(bool)),this, SLOT(hideIfNotConnected(bool)));
    //connect(pMainWindow->showPortsAction,SIGNAL(triggered()),this, SLOT(showIfNotConnected()));
}


//! Magnify the port with a class mebmer factor 'mMag'. Is used i.e. at hovering over disconnected port.
//! @param blowup says if the port should be magnified or not.
void GUIPort::magnify(bool blowup)
{
    if ((!blowup) && (mIsMag))
    {
        this->moveBy((mMag-1)*boundingRect().width()/2, (mMag-1)*boundingRect().height()/2);
        this->scale(1/mMag,1/mMag);
        mpPortLabel->scale(mMag,mMag);
        mIsMag = false;
    }
    else if ((blowup) && (!mIsMag))
    {
        this->scale(mMag, mMag);
        this->moveBy(-(mMag-1)*boundingRect().width()/2, -(mMag-1)*boundingRect().height()/2);
        mpPortLabel->scale(1/mMag,1/mMag);
        mIsMag = true;
    }
}


void GUIPort::setVisible(bool value)
{
    QGraphicsSvgItem::setVisible(value);
    mpPortLabel->setVisible(false);
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsSvgItem::hoverEnterEvent(event);

    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    qDebug() << "hovering over port beloning to: " << mpParentGuiObject->getName();
    magnify(true);

    mpPortLabel->setRotation(-mpParentGuiObject->rotation()-this->rotation());
    //qDebug() << "label: " << mpPortLabel->rotation() << " this: " << this->rotation();
    this->setZValue(1.0);
    mpPortLabel->show();
}


void GUIPort::updatePosition()
{
    if(mpParentGuiObject->rotation() == 0)
        setPos(mXpos-this->boundingRect().width()/2.0, mYpos-this->boundingRect().height()/2.0);
    else if(mpParentGuiObject->rotation() == 90)
        setPos(mXpos-this->boundingRect().width()/2.0, mYpos+this->boundingRect().height()/2.0);
    else if(mpParentGuiObject->rotation() == 180)
        setPos(mXpos+this->boundingRect().width()/2.0, mYpos+this->boundingRect().height()/2.0);
    else
        setPos(mXpos+this->boundingRect().width()/2.0, mYpos-this->boundingRect().height()/2.0);
}


//! Defines what happens when mouse cursor stops hovering a port.
//! @param *event defines the mouse event.
void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //QBrush brush(Qt::green);
    //this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
    magnify(false);

    mpPortLabel->hide();
    this->setZValue(0.0);

    QGraphicsSvgItem::hoverLeaveEvent(event);
}


//! Defines what happens when clicking on a port.
//! @param *event defines the mouse event.
void GUIPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //QGraphicsItem::mousePressEvent(event); //Don't work if this is called
    if (event->button() == Qt::LeftButton)
    {
        std::cout << "GUIPort.cpp: " << "portClick emitted\n";
        emit portClicked(this);
    }
    else if (event->button() == Qt::RightButton)
    {
        std::cout << "GUIPort.cpp: " << "RightClick" << std::endl;
    }
    magnify(false);

    QGraphicsItem::mousePressEvent(event);
}


void GUIPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    std::cout << "GUIPort.cpp: " << "contextMenuEvent" << std::endl;

    if ((!this->isConnected) || (mpParentSystem->mpCoreSystemAccess->getTimeVector(getGUIComponentName(), this->getName()).empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;

        QVector<QString> parameterNames;
        QVector<QString> parameterUnits;
        mpParentGuiObject->mpParentSystem->mpCoreSystemAccess->getPlotDataNamesAndUnits(mpParentGuiObject->getName(), this->getName(), parameterNames, parameterUnits);

        //QAction *plotPressureAction = menu.addAction("Plot pressure");
        //QAction *plotFlowAction = menu.addAction("Plot flow");
        QVector<QAction *> parameterActions;
        QAction *tempAction;
        for(int i=0; i<parameterNames.size(); ++i)
        {

            tempAction = menu.addAction(QString("Plot "+parameterNames[i]+" ["+parameterUnits[i]+"]"));
            parameterActions.append(tempAction);
        }

        QAction *selectedAction = menu.exec(event->screenPos());

        for(int i=0; i<parameterNames.size(); ++i)
        {
            if (selectedAction == parameterActions[i])
            {
                plot(parameterNames[i], parameterUnits[i]);
            }
        }
    }
}


//! Returns a pointer to the GraphicsView that the port belongs to.
GUISystem *GUIPort::getParentSystem()
{
    return mpParentSystem;
}


//! Returns a pointer to the GUIComponent the port belongs to.
GUIObject *GUIPort::getGuiObject()
{
    return mpParentGuiObject;
}


//! Plots the varable number 'nVar' in the node the port is connected to.
//! @param nVar tells which variable to plot.
//! @todo If dataUnit not supplied no unit will be shown, we should maybe lookup the unit if not supplid, or allways look it up, or demand that it is supplied
void GUIPort::plot(QString dataName, QString dataUnit) //En del vansinne i denna metoden...
{
    if(dataUnit.isEmpty())
        dataUnit = this->mpParentSystem->mpCoreSystemAccess->getPlotDataUnit(this->getGUIComponentName(),this->getName(),dataName);

    MainWindow *pMainWindow = mpParentGuiObject->mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;

    if(pMainWindow->mpPlotWidget == 0)
    {
        pMainWindow->mpPlotWidget = new PlotWidget(pMainWindow);
    }

    pMainWindow->mpPlotWidget->mpPlotParameterTree->createPlotWindow(mpParentGuiObject->getName(), this->getName(), dataName, dataUnit);
}


//! Wrapper for the Core getPortTypeString() function
QString GUIPort::getPortType()
{
    return mpParentSystem->mpCoreSystemAccess->getPortType(getGUIComponentName(), this->getName());
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return mpParentSystem->mpCoreSystemAccess->getNodeType(getGUIComponentName(), this->getName());
}


bool GUIPort::haveStartValues()
{
    return mpParentSystem->mpCoreSystemAccess->haveStartValues(getGUIComponentName(), this->getName());
}

void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    mpParentSystem->mpCoreSystemAccess->getStartValueDataNamesValuesAndUnits(getGUIComponentName(), this->getName(), rNames, rValues, rUnits);
}


void GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<double> values)
{
    mpParentSystem->mpCoreSystemAccess->setStartValueDataByNames(getGUIComponentName(), this->getName(), names, values);
}

portDirection GUIPort::getPortDirection()
{
    return mpPortAppearance->direction;
}


void GUIPort::setPortDirection(portDirection direction)
{
    mpPortAppearance->direction = direction;
}


void GUIPort::hide()
{
    mpPortLabel->hide();
    QGraphicsSvgItem::hide();
}


QString GUIPort::getName()
{
    return this->name;
}


void GUIPort::setDisplayName(const QString name)
{
    this->name = name;
    QString label("<p><span style=\"background-color:lightyellow\">");
    label.append(this->name).append("</span></p>");
    mpPortLabel->setHtml(label);
}


//! Get the name of the GUIComponent or GUISubsystem that the port is connected to, This is not necessarily the same as the parent GUIObject name (SystemPorts)
//! @todo this is a very ugly way of handeling system ports should try to think of something better
//! @todo the name should maybe be getConnectComponentName or similar to make it more clear what is returned
QString GUIPort::getGUIComponentName()
{
    if (mpGUIRootSystem == 0)
    {
        //qDebug() << "return guiobject name: " << mpParentGuiObject->getName();
        return mpParentGuiObject->getName();
    }
    else
    {
        //qDebug() << "return root name: " << mpGUIRootSystem->getName();
        return mpGUIRootSystem->getRootSystemName();
    }
}


bool GUIPort::getLastNodeData(QString dataName, double& rData)
{
    return mpParentSystem->mpCoreSystemAccess->getLastNodeData(getGUIComponentName(), this->getName(), dataName, rData);
}


//! Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param hidePortsActionTriggered is true if ports shall be hidden, otherwise false.
void GUIPort::hideIfNotConnected(bool hidePortsActionTriggered)
{
    if(mpParentSystem->mpParentProjectTab == mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->getCurrentTab())
    {
        if(!isConnected && hidePortsActionTriggered)
        {
            this->hide();
        }
        else if(!isConnected && !hidePortsActionTriggered)
        {
            this->show();
        }
    }
}

