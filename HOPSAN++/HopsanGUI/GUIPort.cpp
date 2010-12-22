//$Id$

#include "stdlib.h"

#include <QtGui>
#include <QSvgRenderer>

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


//using namespace std;

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
    : QGraphicsSvgItem(pPortAppearance->mIconPath, pParentGUIModelObject)
{
//    qDebug() << "parentType: " << pParentGUIModelObject->type() << " GUISYSTEM=" << GUISYSTEM << " GUICONTAINER=" << GUICONTAINEROBJECT;
//    qDebug() << "======================= parentName: " << pParentGUIModelObject->getName();

//    //Here we try to figure out what to set the parent container pointer to
//    if ( pParentGUIModelObject->mpParentContainerObject != 0 )
//    {
//        //This is the normal case, our objects parentsystem
//        mpParentContainerObject = pParentGUIModelObject->mpParentContainerObject;
//        //qDebug() << "This seems to be a normal port, setting parentContainer for this port to system: " << mpParentContainerObject->getName();
//    }
//    else if ( pParentGUIModelObject->type() == GUISYSTEM )
//    {
//        //In this case, our parentobject is a root system (that is it has no parent)
//        //this should only happen for external systemports in the root system
//        mpParentContainerObject = qobject_cast<GUISystem*>(pParentGUIModelObject);
//        qDebug() << "This seems to be a Root system and a systemport, ptr: " << mpParentContainerObject;
//        qDebug() << "port ParentContainerName: " << mpParentContainerObject->getName();
//    }
//    else
//    {
//        qDebug() << "This should not happen";
//        assert(false);
//    }

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
    mOverlaySetScale = 1.0;
    mIsMagnified = false;
    mIsConnected = false;

    //connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentContainerObjectPtr(),SLOT(createConnector(GUIPort*)));
    connect(gpMainWindow->hidePortsAction,SIGNAL(triggered(bool)),this, SLOT(hideIfNotConnected(bool)));

    //Connect the view zoom change signal to the port overlay scale slot
    GraphicsView *pView = mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab->mpGraphicsView; //! @todo need to be able to access this in some nicer way then ptr madness
    connect(pView, SIGNAL(zoomChange(qreal)), this, SLOT(setPortOverlayScale(qreal)));
}

void GUIPort::refreshParentContainerConnection()
{
    //! @todo cant we solve this in some other way to avoid the need to refresh connections when moving into groups, OK fior now though
    disconnect(this, SIGNAL(portClicked(GUIPort*)), 0, 0);
    connect(this, SIGNAL(portClicked(GUIPort*)), this->getParentContainerObjectPtr(), SLOT(createConnector(GUIPort*)));
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

    if ((!this->isConnected()) || (mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getTimeVector(getGuiModelObjectName(), this->getName()).empty()))
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

void GUIPort::refreshPortGraphics(QString cqsType, QString portType, QString nodeType)
{
    mpPortAppearance->selectPortIcon(cqsType, portType, nodeType);
    refreshPortGraphics();
}


//! @brief recreate the port graphics overlay
//! @todo This needs to be synced and clean up with addPortOverlayGraphics, right now duplicate work, also should not change if icon same as before
void GUIPort::refreshPortGraphics()
{
    //! @todo the port graphics is not updated when you return to an unconnected systemport, this is probably a problem in core, maybe nodetype is nod reset, need to fix

    //! @todo this seems to load new graphics in old scale, need to fix this
    //If we have an icon, change graphics, and redraw by calling hide and then show
    if (this->renderer()->load(this->mpPortAppearance->mIconPath))
    {
        this->hide();
        this->show();
    }
    else
    {
        qDebug() << "failed to swap icon to: " << this->mpPortAppearance->mIconPath;
    }

    //! @todo instead of delete we should use swap graphics trick like above, but we need to fix the scale problem first
    //Setup port graphics overlay
    if (mpPortGraphicsOverlay!=0)
    {
        delete mpPortGraphicsOverlay;
    }

    if (!this->mpPortAppearance->mIconOverlayPath.isEmpty())
    {
        //! @todo check if file exist
        mpPortGraphicsOverlay = new QGraphicsSvgItem(this->mpPortAppearance->mIconOverlayPath, this);
        mpPortGraphicsOverlay->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    }
    else
    {
        mpPortGraphicsOverlay = 0;
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
bool GUIPort::plot(QString dataName, QString dataUnit) //En del vansinne i denna metoden...
{
    bool success = false;
    if(this->isConnected())
    {
        if(dataUnit.isEmpty())
            dataUnit = mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getPlotDataUnit(this->getGuiModelObjectName(),this->getName(),dataName);

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
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(getGuiModelObjectName(), this->getName());
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getNodeType(getGuiModelObjectName(), this->getName());
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getName(), rNames, rValues, rUnits);
}


void GUIPort::getStartValueDataNamesValuesAndUnits(QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getStartValueDataNamesValuesAndUnits(getGuiModelObjectName(), this->getName(), rNames, rValuesTxt, rUnits);
}


//void GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<double> values)
//{
//    mpParentContainerObject->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getName(), names, values);
//}

bool GUIPort::setStartValueDataByNames(QVector<QString> names, QVector<QString> valuesTxt)
{
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->setStartValueDataByNames(getGuiModelObjectName(), this->getName(), names, valuesTxt);
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
    this->magnify(false);
    mpPortLabel->hide();
    QGraphicsSvgItem::hide();
}

void GUIPort::show()
{
    QGraphicsSvgItem::show();
    mpPortLabel->hide();
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
    return mpParentGuiModelObject->getParentContainerObject()->getCoreSystemAccessPtr()->getLastNodeData(getGuiModelObjectName(), this->getName(), dataName, rData);
}


//! Slot that hides the port if "hide ports" setting is enabled, but only if the project tab is opened.
//! @param hidePortsActionTriggered is true if ports shall be hidden, otherwise false.
void GUIPort::hideIfNotConnected(bool hidePortsActionTriggered)
{
    if(mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab == mpParentGuiModelObject->getParentContainerObject()->mpParentProjectTab->mpParentProjectTabWidget->getCurrentTab())
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


GroupPort::GroupPort(QString name, qreal xpos, qreal ypos, GUIPortAppearance* pPortAppearance, GUIModelObject *pParent)
    : GUIPort(name, xpos, ypos, pPortAppearance, pParent)
{
    //Nothing extra yet
}

//! Overloaded as groups laks core connection
QString GroupPort::getPortType()
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
