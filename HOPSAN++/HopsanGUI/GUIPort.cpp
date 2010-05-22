//$Id$

//#include "HopsanCore.h"
#include "GUIPort.h"
#include "plotwidget.h"
#include "mainwindow.h"

#include <QtGui>


//! Constructor.
//! @param corePort a pointer to the corresponing port in Core.
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param iconPath a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(Port *pCorePort, qreal xpos, qreal ypos, PortAppearance* pPortAppearance, GUIObject *pParent)
    : QGraphicsSvgItem(pPortAppearance->iconPath, pParent)
{
    //*****Core Interaction*****
    mpCorePort = pCorePort;
    //**************************

    mpParentGraphicsView = pParent->mpParentGraphicsView;
    mpParentGuiObject = pParent;
    mpPortAppearance = pPortAppearance;

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

    if ((!this->isConnected) || (this->mpCorePort->getTimeVectorPtr()->empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;

        if (this->getNodeType() =="NodeHydraulic")
        {
            QAction *plotPressureAction = menu.addAction("Plot pressure");
            QAction *plotFlowAction = menu.addAction("Plot flow");
            QAction *selectedAction = menu.exec(event->screenPos());

            if (selectedAction == plotFlowAction)
            {
                plot(0);
            }
            if (selectedAction == plotPressureAction)
            {
                plot(1);
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
                plot(0);
            }
            if (selectedAction == plotForceAction)
            {
                plot(1);
            }
            if (selectedAction == plotPositionAction)
            {
                plot(2);
            }
        }
        if (this->getNodeType() =="NodeSignal")
        {
            QAction *plotSignalAction = menu.addAction("Plot signal value");
            QAction *selectedAction = menu.exec(event->screenPos());

            if (selectedAction == plotSignalAction)
            {
                plot(0);
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
    //! @todo Thiss will crash if not GUIComponent should be GUI object may need to change code elsewhere
//    GUIComponent* ptr = qobject_cast<GUIComponent*>(mpParentGuiObject);
//    return ptr;
    return mpParentGuiObject;
}


//! Plots the varable number 'nVar' in the node the port is connected to.
//! @param nVar tells which variable to plot.
void GUIPort::plot(size_t nVar) //En del vansinne i denna metoden...
{
    std::cout << "GUIPort.cpp: " << "Plot()" << std::endl;

    //*****Core Interaction*****
    size_t dataLength = this->mpCorePort->getTimeVectorPtr()->size();

    QVector<double> time = QVector<double>::fromStdVector(*(this->mpCorePort->getTimeVectorPtr())); //Inte lampligt att skyffla data pa detta viset
    QVector<double> y(dataLength);// = QVector<double>::fromStdVector((this->mpCorePort->getDataVectorPtr()->at(1)));
    //**************************

    qDebug() << "Time size: " << time.size() << " last time: " << *time.end() << " datalength: " << dataLength << "y.size(): " << y.size();
    qDebug() << "time[0]: " << time[0] << " time[last-1]: " << time[time.size()-2] << " time[last]: " << time[time.size()-1];

    for (int i = 0; i<time.size(); ++i)
    {
        //qDebug() << time[i];
    }
    
    for (size_t i = 0; i<dataLength; ++i) //Denna loop ar inte klok
    {
        //*****Core Interaction*****
        //timeq[i] = this->mpCorePort->getTimeVectorPtr()->at(i);
        y[i] = (this->mpCorePort->getDataVectorPtr()->at(i)).at(nVar);
        //**************************
    }

    qDebug() << "y[0]: " << y[0] << " y[last-1]: " << y[y.size()-2] << " y[last]: " << y[y.size()-1];

    QString title;
    QString xlabel;
    QString ylabel;

    string name, unit;
    //*****Core Interaction*****
    mpCorePort->getNodeDataNameAndUnit(nVar, name, unit);
    //**************************
    title.append(QString::fromStdString(name));
    ylabel.append(QString::fromStdString(name) + ", [" + QString::fromStdString(unit) + "]");

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
    return this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPortType(mpParentGuiObject->getName(), this->getName());
}


//! Wrapper for the Core getNodeType() function
QString GUIPort::getNodeType()
{
    return this->mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getNodeType(mpParentGuiObject->getName(), this->getName());
}


PortAppearance::portDirectionType GUIPort::getPortDirection()
{
    return mpPortAppearance->direction;
}

void GUIPort::setPortDirection(PortAppearance::portDirectionType direction)
{
    mpPortAppearance->direction = direction;
}

QString GUIPort::getName()
{
    //*****Core Interaction*****
    return QString::fromStdString(mpCorePort->getPortName());       //This must change so that ports now their own names
    //**************************
}

QString GUIPort::getGUIComponentName()
{
    return this->mpParentGuiObject->getName();
}


void GUIPort::hideIfNotConnected(bool justDoIt)
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
