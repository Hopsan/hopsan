//$Id$

#include <QtGui>

#include "common.h"

#include "GUIPort.h"
#include "Widgets/PlotWidget.h"
#include "MainWindow.h"
#include "CoreAccess.h"
#include "GUIObjects/GUIModelObject.h"
#include "GraphicsView.h"
#include "Widgets/ProjectTabWidget.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"

using namespace std;

QPointF getOffsetPointfromPort(GUIPort *pPort)
{
    QPointF point;

    if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).x() > pPort->scenePos().x()))
    {
        point.setX(-20);
    }
    else if((pPort->getPortDirection() == LEFTRIGHT) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).x() < pPort->scenePos().x()))
    {
        point.setX(20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).y() > pPort->scenePos().y()))
    {
        point.setY(-20);
    }
    else if((pPort->getPortDirection() == TOPBOTTOM) && (pPort->getGuiModelObject()->mapToScene(pPort->getGuiModelObject()->boundingRect().center()).y() < pPort->scenePos().y()))
    {
        point.setY(20);
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
    : QGraphicsSvgItem(pPortAppearance->mIconPath, pParentGUIModelObject)
{
//    qDebug() << "parentType: " << pParentGUIModelObject->type() << " GUISYSTEM=" << GUISYSTEM;
//    qDebug() << "======================= parentName: " << pParentGUIModelObject->getName();

    //Here we try to figure out what to set the parent system pointer to
    if ( pParentGUIModelObject->mpParentContainerObject != 0 )
    {
        //This is the normal case, our objects parentsystem
        mpParentContainerObject = pParentGUIModelObject->mpParentContainerObject;
    }
    else if ( pParentGUIModelObject->type() == GUISYSTEM )
    {
        //In this case, our parentobect is a root system (that is it has no parent)
        //this should only happen in systemports in the root system
        mpParentContainerObject = qobject_cast<GUISystem*>(pParentGUIModelObject);
    }
    else
    {
        qDebug() << "This should not happen";
        assert(false);
    }

    mpParentGuiModelObject = pParentGUIModelObject;
    mpPortAppearance = pPortAppearance;

    this->mName = portName;

    updatePosition(xpos, ypos);
    //All rotaion and other transformation should be aplied around the port center
    setTransformOriginPoint(boundingRect().center());

    this->setAcceptHoverEvents(true);

    //Setup port label and overlay (if it exists)
    this->addPortGraphicsOverlay(pPortAppearance->mIconOverlayPath);
    this->setRotation(mpPortAppearance->rot);
    this->refreshPortOverlayPosition();

    mMag = GOLDENRATIO;
    mIsMagnified = false;
    mIsConnected = false;

    GraphicsView *pView = mpParentContainerObject->mpParentProjectTab->mpGraphicsView;

    connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentContainerObjectPtr(),SLOT(createConnector(GUIPort*)));
    connect(gpMainWindow->hidePortsAction,SIGNAL(triggered(bool)),this, SLOT(hideIfNotConnected(bool)));

    //Connect the view zoom change signal to the port overlay scale slot
    connect(pView, SIGNAL(zoomChange(qreal)), this, SLOT(setPortOverlayScale(qreal)));
}


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


void GUIPort::setVisible(bool value)
{
    QGraphicsSvgItem::setVisible(value);
    mpPortLabel->setVisible(false);
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //qDebug() << "hovering over port beloning to: " << mpParentGuiModelObject->getName();
    QGraphicsSvgItem::hoverEnterEvent(event);

    this->setCursor(Qt::CrossCursor);

    magnify(true);
    this->setZValue(1.0);
    mpPortLabel->show();
}

//! @brief Updates the gui port position on its parent object, taking coordinates in parent coordinate system
//! @todo is it necessary to be able to update orientation also?
void GUIPort::updatePosition(const qreal x, const qreal y)
{
    //Place the guiport with center in x and y, assume x and y in parent local coordinates
    this->setPos(x-boundingRect().width()/2.0, y-boundingRect().height()/2.0);
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
    //QGraphicsSvgItem::mousePressEvent(event); //Don't work if this is called
    if (event->button() == Qt::LeftButton)
    {
        //std::cout << "GUIPort.cpp: " << "portClick emitted\n";
        emit portClicked(this);
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

    if ((!this->isConnected()) || (mpParentContainerObject->getCoreSystemAccessPtr()->getTimeVector(getGuiModelObjectName(), this->getName()).empty()))
    {
        event->ignore();
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
    mpParentGuiModelObject->mpParentContainerObject->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(mpParentGuiModelObject->getName(), this->getName(), parameterNames, parameterUnits);

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
            plot(parameterNames[i], parameterUnits[i]);
        }
    }
}


//QVariant GUIPort::itemChange( GraphicsItemChange change, const QVariant & value )
//{
//    return QGraphicsItem::itemChange(change, value);
//}


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
    mpPortLabel = new QGraphicsTextItem(this);
    mpPortLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    mpPortLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    mpPortLabel->hide();

    //Port label must exist and be set up before we run setDisplayName
    this->setDisplayName(this->getName());
}


//! @brief Refreshes the port overlay position and makes sure that the overlay allways have rotation zero (to be readable)
void GUIPort::refreshPortOverlayPosition()
{
    QTransform transf;
    QPointF pt1, pt2, pt3;

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
    this->mpPortLabel->setPos(pt3-pt2+QPoint(10, 10)); //! @todo This is little messy, GUIPort::magnify fucks the pos for the label a bit

    //Refresh the port overlay graphics
    if (this->mpPortGraphicsOverlay != 0)
    {
        transf.reset();

        pt1 = this->mpPortGraphicsOverlay->boundingRect().center();
        transf.rotate(-(this->mpPortGraphicsOverlay->rotation() + this->rotation() + this->mpParentGuiModelObject->rotation()));
        if (this->mpParentGuiModelObject->isFlipped())
        {
            transf.scale(-1.0,1.0);
        }
        pt2 =  transf * pt1;
        this->mpPortGraphicsOverlay->setPos(pt3-pt2);
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

    if (this->mpPortGraphicsOverlay != 0)
    {
        this->mpPortGraphicsOverlay->setScale(mOverlaySetScale * overlayExtraScaleFactor);
    }
}

//! Returns a pointer to the GraphicsView that the port belongs to.
GUIContainerObject *GUIPort::getParentContainerObjectPtr()
{
    return mpParentContainerObject;
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
            dataUnit = this->mpParentContainerObject->getCoreSystemAccessPtr()->getPlotDataUnit(this->getGuiModelObjectName(),this->getName(),dataName);

        if(gpMainWindow->mpPlotWidget == 0)
        {
            gpMainWindow->mpPlotWidget = new PlotWidget(gpMainWindow);
        }

        if(gpMainWindow->mpPlotWidget->mpPlotParameterTree->createPlotWindow(mpParentGuiModelObject->getName(), this->getName(), dataName, dataUnit))
            success = true;
    }

    return success;
}


//! Wrapper for the Core getPortTypeString() function
QString GUIPort::getPortType()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getPortType(getGuiModelObjectName(), this->getName());
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getNodeType(getGuiModelObjectName(), this->getName());
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    mpParentContainerObject->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getName(), rNames, rValues, rUnits);
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    mpParentContainerObject->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getName(), rNames, rValuesTxt, rUnits);
}


void GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<double> values)
{
    mpParentContainerObject->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getName(), names, values);
}

void GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt)
{
    mpParentContainerObject->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getName(), names, valuesTxt);
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



void GUIPort::hide()
{
    mpPortLabel->hide();
    QGraphicsSvgItem::hide();
}


QString GUIPort::getName()
{
    return this->mName;
}

QString GUIPort::getGuiModelObjectName()
{
    return this->mpParentGuiModelObject->getName();
}


void GUIPort::setDisplayName(const QString name)
{
    this->mName = name;
    QString label("<p><span style=\"background-color:lightyellow\">");
    label.append(this->mName).append("</span></p>");
    mpPortLabel->setHtml(label);
}


bool GUIPort::getLastNodeData(QString dataName, double& rData)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getLastNodeData(getGuiModelObjectName(), this->getName(), dataName, rData);
}


//! Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param hidePortsActionTriggered is true if ports shall be hidden, otherwise false.
void GUIPort::hideIfNotConnected(bool hidePortsActionTriggered)
{
    if(mpParentContainerObject->mpParentProjectTab == mpParentContainerObject->mpParentProjectTab->mpParentProjectTabWidget->getCurrentTab())
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

