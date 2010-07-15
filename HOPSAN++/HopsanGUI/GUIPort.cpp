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

#include "GUIPort.h"
#include "PlotWidget.h"
#include "MainWindow.h"
#include "GUIRootSystem.h"
#include "GUIObject.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "ProjectTabWidget.h"

//! Constructor.
//! @param portName The name of the port
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param iconPath a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(QString portName, qreal xpos, qreal ypos, PortAppearance* pPortAppearance, GUIObject *pParent, GUIRootSystem *pGUIRootSystem)
    : QGraphicsSvgItem(pPortAppearance->iconPath, pParent)
{
    mpParentGraphicsView = pParent->mpParentGraphicsView;
    mpParentGuiObject = pParent;
    mpPortAppearance = pPortAppearance;
    mpGUIRootSystem = pGUIRootSystem; //Use this to indicate system port

    this->name = portName;

    //setTransformOriginPoint(boundingRect().width()/2,boundingRect().height()/2);
    setTransformOriginPoint(boundingRect().center());

    mXpos = xpos;
    mYpos = ypos;

    updatePosition();

//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    //QBrush brush(Qt::green);
    //this->setBrush(brush);

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

    mMag = 1.6180339887;
    mIsMag = false;
    isConnected = false;

    MainWindow *pMainWindow = mpParentGuiObject->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
    connect(pMainWindow->hidePortsAction,SIGNAL(triggered(bool)),this, SLOT(hideIfNotConnected(bool)));
    //connect(pMainWindow->showPortsAction,SIGNAL(triggered()),this, SLOT(showIfNotConnected()));
}


GUIPort::~GUIPort()
{
}


//! Magnify the port with a class mebmer factor 'mMag'. Is used i.e. at hovering over disconnected port.
//! @param blowup says if the port should be magnified or not.
void GUIPort::magnify(bool blowup)
{
    if ((!blowup) && (mIsMag))
    {
        this->moveBy((mMag-1)*boundingRect().width()/2, (mMag-1)*boundingRect().height()/2);
        this->scale(1/mMag,1/mMag);
        this->mpPortLabel->scale(mMag,mMag);
        mIsMag = false;
    }
    else if ((blowup) && (!mIsMag))
    {
        this->scale(mMag, mMag);
        this->moveBy(-(mMag-1)*boundingRect().width()/2, -(mMag-1)*boundingRect().height()/2);
        this->mpPortLabel->scale(1/mMag,1/mMag);
        mIsMag = true;
    }
}


void GUIPort::setVisible(bool visible)
{
    QGraphicsSvgItem::setVisible(visible);
    mpPortLabel->setVisible(false);
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsSvgItem::hoverEnterEvent(event);

    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    std::cout << "GUIPort.cpp: " << "hovering over port" << std::endl;
    magnify(true);

    mpPortLabel->setRotation(-this->mpParentGuiObject->rotation()-this->rotation());
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
    QGraphicsSvgItem::hoverLeaveEvent(event);

    QBrush brush(Qt::green);
    //this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
    magnify(false);

    mpPortLabel->hide();
    this->setZValue(0.0);
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
}


void GUIPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    std::cout << "GUIPort.cpp: " << "contextMenuEvent" << std::endl;

    if ((!this->isConnected) || (this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getTimeVector(getGUIComponentName(), this->getName()).empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;
        //! @todo this bellow is complete madness. hardcoded hopsan specific stuff, must be rewritten, not sure if it should even be here
        //! @todo we do not need hardcoded stuff we can ask the core about what variables are available
        if (this->getNodeType() =="NodeHydraulic")
        {
            QAction *plotPressureAction = menu.addAction("Plot pressure");
            QAction *plotFlowAction = menu.addAction("Plot flow");
            QAction *selectedAction = menu.exec(event->screenPos());

            if (selectedAction == plotFlowAction)
            {
                plot("MassFlow");
            }
            if (selectedAction == plotPressureAction)
            {
                plot("Pressure");
            }
        }
        if (this->getNodeType() =="NodeMechanic")
        {
            QAction *plotVelocityAction = menu.addAction("Plot velocity");
            QAction *plotForceAction = menu.addAction("Plot force");
            QAction *plotPositionAction = menu.addAction("Plot position");
            QAction *selectedAction = menu.exec(event->screenPos());

            if (selectedAction == plotVelocityAction)
            {
                plot("Velocity");
            }
            if (selectedAction == plotForceAction)
            {
                plot("Force");
            }
            if (selectedAction == plotPositionAction)
            {
                plot("Position");
            }
        }
        if (this->getNodeType() =="NodeSignal")
        {
            QAction *plotSignalAction = menu.addAction("Plot signal value");
            QAction *selectedAction = menu.exec(event->screenPos());

            if (selectedAction == plotSignalAction)
            {
                plot("Value");
            }
        }
    }
}


//! Returns a pointer to the GraphicsView that the port belongs to.
GraphicsView *GUIPort::getParentView()
{
    return mpParentGraphicsView;
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
    std::cout << "GUIPort.cpp: " << "Plot()" << std::endl;

    QVector<double> time = QVector<double>::fromStdVector(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getTimeVector(getGUIComponentName(), this->getName()));
    QVector<double> y;
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPlotData(getGUIComponentName(), this->getName(), dataName, y);

    //qDebug() << "Time size: " << time.size() << " last time: " << *time.end() << " " << "y.size(): " << y.size();
    //qDebug() << "time[0]: " << time[0] << " time[last-1]: " << time[time.size()-2] << " time[last]: " << time[time.size()-1];

    for (int i = 0; i<time.size(); ++i)
    {
        //qDebug() << time[i];
    }

    //qDebug() << "y[0]: " << y[0] << " y[last-1]: " << y[y.size()-2] << " y[last]: " << y[y.size()-1];

    QString title;
    QString xlabel;
    QString ylabel;

    title.append(dataName);
    ylabel.append(dataName + ", [" + dataUnit + "]");

    //! @todo need to comment this out  for now  fix later
    //title.append(" at component: ").append(QString::fromStdString(mpParentComponent->mpCoreComponent->getName())).append(", port: ").append(QString::fromStdString(mpCorePort->getPortName()));
    xlabel.append("Time, [s]");

    //PlotWidget *newPlot = new PlotWidget(time,y,mpParentGuiObject->mpParentGraphicsView);
    PlotWidget *newPlot = new PlotWidget(time,y,mpParentGuiObject->mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow);

    //newPlot->mpVariablePlot->setTitle(title);
    newPlot->mpCurve->setTitle(title);
    newPlot->mpVariablePlot->setAxisTitle(VariablePlot::yLeft, ylabel);
    newPlot->mpVariablePlot->setAxisTitle(VariablePlot::xBottom, xlabel);
    newPlot->mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);

    newPlot->show();
}


//! Returns the number of the port by calling the equivalent function in the parent component.
int GUIPort::getPortNumber()
{
    return this->getGuiObject()->getPortNumber(this);
}


//! Wrapper for the Core getPortTypeString() function
QString GUIPort::getPortType()
{
    return this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPortType(getGUIComponentName(), this->getName());
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getNodeType(getGUIComponentName(), this->getName());
}


PortAppearance::portDirectionType GUIPort::getPortDirection()
{
    return mpPortAppearance->direction;
}


void GUIPort::setPortDirection(PortAppearance::portDirectionType direction)
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
        return mpGUIRootSystem->getName();
    }
}


//! Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param justDoIt is true if ports shall be hidden, otherwise false.
void GUIPort::hideIfNotConnected(bool justDoIt)
{
    if(mpParentGraphicsView->mpParentProjectTab == mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->getCurrentTab())
    {
        if(!isConnected and justDoIt)
        {
            this->hide();
        }
        else if(!isConnected and !justDoIt)
        {
            this->show();
        }
    }
}
