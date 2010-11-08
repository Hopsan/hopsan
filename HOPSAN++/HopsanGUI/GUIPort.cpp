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
#include "GUIUtilities.h"

using namespace std;

//! Constructor.
//! @param portName The name of the port
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param QString(ICONPATH) a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(QString portName, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParentGUIModelObject, CoreSystemAccess *pGUIRootSystem)
    : QGraphicsSvgItem(pPortAppearance->mIconPath, pParentGUIModelObject)
{
    qDebug() << "parentType: " << pParentGUIModelObject->type() << " GUISYSTEM=" << GUISYSTEM;
    qDebug() << "======================= parentName: " << pParentGUIModelObject->getName();

    if ( pParentGUIModelObject->mpParentSystem != 0 )
    {
        mpParentSystem = pParentGUIModelObject->mpParentSystem;
    }
    else if ( pParentGUIModelObject->type() == GUISYSTEM )
    {
        //Assume that parentGuiObject (which is a system) is the root system
        //! @todo not sure that this is really 100% correct
        mpParentSystem = qobject_cast<GUISystem*>(pParentGUIModelObject);
    }
    else
    {
        qDebug() << "This should not happen";
        assert(false);
    }

    mpParentGuiModelObject = pParentGUIModelObject;
    mpPortAppearance = pPortAppearance;
    mpGUIRootSystem = pGUIRootSystem; //Use this to indicate system port

    this->name = portName;

    updatePosition(xpos, ypos);
    //All rotaion and other transformation should be aplied around the port center
    setTransformOriginPoint(boundingRect().center());


//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

//    mpPortLabel = new QGraphicsTextItem(this);
//    QString label("<p><span style=\"background-color:lightyellow\">");
//    label.append(this->getName()).append("</span></p>");
//    mpPortLabel->setHtml(label);
//    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
//    mpPortLabel->setPos(7.0,7.0);
//    mpPortLabel->hide();

    //Setup port label and overlay (if it exists)
    this->addPortGraphicsOverlay(pPortAppearance->mIconOverlayPath);
//    //! @todo this kind of harcoded stuff should not be here, fix the problem in some other way
//    if(this->getPortType() == "POWERPORT")
//    {
//        this->setRotation(0.0);
//        mpPortLabel->setRotation(0.0);
//    }
//    else
//    {
        this->setRotation(mpPortAppearance->rot);
        //mpPortLabel->setRotation(-mpPortAppearance->rot);
//        if (mpPortGraphicsOverlay != 0)
//        {
//            mpPortGraphicsOverlay->setRotation(-mpPortAppearance->rot);
//        }


        this->refreshPortOverlayRotation();
//    }

    mMag = GOLDENRATIO;
    mIsMag = false;
    mIsConnected = false;

    MainWindow *pMainWindow = mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    GraphicsView *pView = mpParentSystem->mpParentProjectTab->mpGraphicsView;

    connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentSystem(),SLOT(createConnector(GUIPort*)));
    connect(pMainWindow->hidePortsAction,SIGNAL(triggered(bool)),this, SLOT(hideIfNotConnected(bool)));

    //Connect the view zoom change signal to the port overlay scale slot
    connect(pView, SIGNAL(zoomChange(qreal)), this, SLOT(setPortOverlayScale(qreal)));


    //connect(pMainWindow->showPortsAction,SIGNAL(triggered()),this, SLOT(showIfNotConnected()));
}


//! Magnify the port with a class mebmer factor 'mMag'. Is used i.e. at hovering over disconnected port.
//! @param blowup says if the port should be magnified or not.
void GUIPort::magnify(bool blowup)
{
    if ((!blowup) && (mIsMag))
    {
        this->moveBy((mMag-1.0)*boundingRect().width()/2.0, (mMag-1.0)*boundingRect().height()/2.0);
        this->scale(1.0/mMag,1.0/mMag);

        this->scalePortOverlay(1.0/mMag);
        mIsMag = false;
    }
    else if ((blowup) && (!mIsMag))
    {
        this->scale(mMag, mMag);
        this->moveBy(-(mMag-1.0)*boundingRect().width()/2.0, -(mMag-1.0)*boundingRect().height()/2.0);

        this->scalePortOverlay(mMag);
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
    qDebug() << "hovering over port beloning to: " << mpParentGuiModelObject->getName();
    magnify(true);

    //mpPortLabel->setRotation(-mpParentGuiModelObject->rotation()-this->rotation());
    //qDebug() << "label: " << mpPortLabel->rotation() << " this: " << this->rotation();
    this->setZValue(1.0);
    mpPortLabel->show();
}

//! @brief Updates the gui port position on its parent object, taking coordinates in aprent coordinate system
//! @todo it is necessary to be able to update orientation also
void GUIPort::updatePosition(const qreal x, const qreal y)
{
    qDebug() << "Update Port Position in: " << this->getName();
    qDebug() << "x,y: " << x << " " << y;
    qDebug() << "Parent bounding rect: " << mpParentGuiModelObject->boundingRect();
    qDebug() << "Parentrotation: " << mpParentGuiModelObject->rotation();
    qDebug() << "This bounding rect: " << this->boundingRect();
    qDebug() << "This bounding rect center: " << this->boundingRect().center();
    qDebug() << "This transform origin: " << this->transformOriginPoint();


//    qreal mXpos = x;
//    qreal mYpos = y;
//    Original code----
//    if(mpParentGuiObject->rotation() == 0)
//        setPos(mXpos-this->boundingRect().width()/2.0, mYpos-this->boundingRect().height()/2.0);
//    else if(mpParentGuiObject->rotation() == 90)
//        setPos(mXpos-this->boundingRect().width()/2.0, mYpos+this->boundingRect().height()/2.0);
//    else if(mpParentGuiObject->rotation() == 180)
//        setPos(mXpos+this->boundingRect().width()/2.0, mYpos+this->boundingRect().height()/2.0);
//    else
//        setPos(mXpos+this->boundingRect().width()/2.0, mYpos-this->boundingRect().height()/2.0);
//--------------------------

    //Place the guiport with center in x and y, assume x and y in parent local coordinates
    this->setPos(x-boundingRect().width()/2.0, y-boundingRect().height()/2.0);
    //Now rotate backwards around center to always keep the object horizontal
    //this->setTransformOriginPoint(boundingRect().width()/2.0, boundingRect().height()/2.0);
    //this->setRotation(mpParentGuiObject->rotation());
    //Reset transformation (rotation) point
    //this->setTransformOriginPoint(0, 0);

    qDebug() << "Resulting pos: " << this->pos();

}

//! Conveniance function when fraction positions are known
void GUIPort::updatePositionByFraction(qreal x, qreal y)
{
    qDebug() << "Fraction position: " << x << " " << y;
    qDebug() << "parent bounding rect: " << mpParentGuiModelObject->boundingRect();
    //qDebug() << "parent icon scene bounding rect" << mpParentGuiObject->mpIcon->sceneBoundingRect();

    //! @todo maybe use only boundingrect instead (if they are same)
    //! @todo for now root systems may not have an icon, if icon is empty ports will end up in zero, which is OK, maybe we should always force a default icon
    //this->updatePosition(x*mpParentGuiObject->mpIcon->sceneBoundingRect().width(), y*mpParentGuiObject->mpIcon->sceneBoundingRect().height());
    this->updatePosition(x*mpParentGuiModelObject->boundingRect().width(), y*mpParentGuiModelObject->boundingRect().height());
}


QPointF GUIPort::getCenterPos()
{
    return this->pos() + QPointF( this->boundingRect().width()/2.0, this->boundingRect().height()/2.0);
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
        //std::cout << "GUIPort.cpp: " << "portClick emitted\n";
        emit portClicked(this);
    }
    else if (event->button() == Qt::RightButton)
    {
        //std::cout << "GUIPort.cpp: " << "RightClick" << std::endl;
    }
    magnify(false);
}


//! Defines what happens when double clicking on a port. Nothing should happen.
//! @param *event defines the mouse event.
void GUIPort::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    //Nothing to do, reimplemented just to do nothing
}


void GUIPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    std::cout << "GUIPort.cpp: " << "contextMenuEvent" << std::endl;

    if ((!this->isConnected()) || (mpParentSystem->mpCoreSystemAccess->getTimeVector(getGUIComponentName(), this->getName()).empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;

        QVector<QString> parameterNames;
        QVector<QString> parameterUnits;
        mpParentGuiModelObject->mpParentSystem->mpCoreSystemAccess->getPlotDataNamesAndUnits(mpParentGuiModelObject->getName(), this->getName(), parameterNames, parameterUnits);

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

QVariant GUIPort::itemChange( GraphicsItemChange change, const QVariant & value )
{
//    qDebug() << "GuiPort item has changed: " << change;
//    if (change == QGraphicsItem::ItemTransformHasChanged)
//    {
//        //Update the port overaly position and the label position
//        qDebug() << "Refreshing port overlay position";

//    }


    return QGraphicsItem::itemChange(change, value);
}


void GUIPort::addPortGraphicsOverlay(QString filepath)
{
    //Setup port graphics overlay
    if (!filepath.isEmpty())
    {
        //! @todo check if file exist
        mpPortGraphicsOverlay = new QGraphicsSvgItem(filepath, this);
        mpPortGraphicsOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    }
    else
    {
        mpPortGraphicsOverlay = 0;
    }

    //Setup port label (ports allways have lables)
    QString label("<p><span style=\"background-color:lightyellow\">");
    label.append(this->getName()).append("</span></p>");

    mpPortLabel = new QGraphicsTextItem(this);
    mpPortLabel->setHtml(label);
    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mpPortLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QPointF portcenter = this->boundingRect().center();
    //mpPortLabel->setPos(portcenter*2.0);
    mpPortLabel->hide();
}


//! @brief Refreshes the port overlay rotation and makes sure that the overlay allways have rotation zero (to be readable)
void GUIPort::refreshPortOverlayRotation()
{
    QTransform transf;
    QPointF pt1, pt2, pt3;

//    qDebug() << "before, overlay pos: " << mpPortLabel->pos();
//    qDebug() << "overlay local angle: " << mpPortLabel->rotation();
//    qDebug() << "overlay local+port angle: " << mpPortLabel->rotation() + this->rotation();
//    qDebug() << "overlay local+port+parent angle: " << mpPortLabel->rotation() + this->rotation() +this->mpParentGuiModelObject->rotation();

    //Refresh the port label position and orientation
    //! @todo something wierd with the portlable it seems to be bigger than what you can see in the gui
    pt1 = this->mpPortLabel->boundingRect().center();
    transf.rotate(-(this->mpPortLabel->rotation() + this->rotation() + this->mpParentGuiModelObject->rotation()));
    if (this->mpParentGuiModelObject->isFlipped())
    {
        transf.scale(-1.0,1.0);
    }
    pt2 =  transf * pt1;
    pt3 = this->boundingRect().center();
    this->mpPortLabel->setPos(pt3-pt2);

//    qDebug() << "pt1: " << pt1;
//    qDebug() << "pt2: " << pt2;
//    qDebug() << "pt3: " << pt3;
//    qDebug() << "after, overlay pos: " << mpPortLabel->pos();
//    qDebug() << "\n";

    //Refresh the port overlay graphics
    if (this->mpPortGraphicsOverlay != 0)
    {
        transf.reset();
//        qDebug() << "before, overlay pos: " << this->mpPortGraphicsOverlay->pos();
//        qDebug() << "overlay local angle: " << this->mpPortGraphicsOverlay->rotation();
//        qDebug() << "overlay local+port angle: " << this->mpPortGraphicsOverlay->rotation() + this->rotation();
//        qDebug() << "overlay local+port+parent angle: " << this->mpPortGraphicsOverlay->rotation() + this->rotation() +this->mpParentGuiModelObject->rotation();

        pt1 = this->mpPortGraphicsOverlay->boundingRect().center();
        transf.rotate(-(this->mpPortGraphicsOverlay->rotation() + this->rotation() + this->mpParentGuiModelObject->rotation()));
        if (this->mpParentGuiModelObject->isFlipped())
        {
            transf.scale(-1.0,1.0);
        }
        pt2 =  transf * pt1;
        this->mpPortGraphicsOverlay->setPos(pt3-pt2);

//        qDebug() << "pt1: " << pt1;
//        qDebug() << "pt2: " << pt2;
//        qDebug() << "pt3: " << pt3;
//        qDebug() << "after, overlay pos: " << this->mpPortGraphicsOverlay->pos();
//        qDebug() << "\n";
    }
}

//! @brief Scales the port overlay graphics and tooltip
void GUIPort::setPortOverlayScale(qreal scale)
{
    this->mpPortLabel->setScale(scale);

    if (this->mpPortGraphicsOverlay != 0)
    {
        this->mpPortGraphicsOverlay->setScale(scale);
    }

}

void GUIPort::scalePortOverlay(qreal scalefactor)
{
    this->mpPortLabel->setScale(mpPortLabel->scale()*1.0/scalefactor);

    if (this->mpPortGraphicsOverlay != 0)
    {
        this->mpPortGraphicsOverlay->setScale(mpPortGraphicsOverlay->scale() * scalefactor);
    }
}


//! Returns a pointer to the GraphicsView that the port belongs to.
GUISystem *GUIPort::getParentSystem()
{
    return mpParentSystem;
}


//! Returns a pointer to the GUIComponent the port belongs to.
GUIModelObject *GUIPort::getGuiModelObject()
{
    return mpParentGuiModelObject;
}


//! Plots the variable with name 'dataName' in the node the port is connected to.
//! @param dataName tells which variable to plot.
//! @param dataUnit sets the unit to show in the plot (has no connection to data, just text).
bool GUIPort::plot(QString dataName, QString dataUnit) //En del vansinne i denna metoden...
{
    bool success = false;
    if(this->isConnected())
    {
        if(dataUnit.isEmpty())
            dataUnit = this->mpParentSystem->mpCoreSystemAccess->getPlotDataUnit(this->getGUIComponentName(),this->getName(),dataName);

        MainWindow *pMainWindow = mpParentGuiModelObject->mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;

        if(pMainWindow->mpPlotWidget == 0)
        {
            pMainWindow->mpPlotWidget = new PlotWidget(pMainWindow);
        }

        if(pMainWindow->mpPlotWidget->mpPlotParameterTree->createPlotWindow(mpParentGuiModelObject->getName(), this->getName(), dataName, dataUnit))
            success = true;
    }

    return success;
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
    qreal scene_angle = this->mpParentGuiModelObject->rotation() + this->rotation();
    if( (scene_angle == 0) || (scene_angle == 180) )
    {
        return LEFTRIGHT;
    }
    else
    {
        return TOPBOTTOM;
    }
}


//! @brief Access method for mIsConnected
//! @param isConnected tells if the port is connected or not.
void GUIPort::setIsConnected(bool isConnected)
{
    //! @todo Maybe should this be handled in core only snd just ask core if connected or not
    mIsConnected = isConnected;
}


//! @brief Access method for mIsConnected
//! @return if the port is connected or not
bool GUIPort::isConnected()
{
    return mIsConnected;
}


//! @todo Do we really need both direction and heading
qreal GUIPort::getPortHeading()
{
    return mpPortAppearance->rot;
}



//void GUIPort::setPortDirection(portDirection direction)
//{
//    mpPortAppearance->direction = direction;
//}


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
        return mpParentGuiModelObject->getName();
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
        if(!isConnected() && hidePortsActionTriggered)
        {
            this->hide();
        }
        else if(!isConnected() && !hidePortsActionTriggered)
        {
            this->show();
        }
    }
}

