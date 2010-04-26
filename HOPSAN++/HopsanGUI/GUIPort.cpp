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
GUIPort::GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, Port::PORTTYPE type, portDirectionType portDirection, GUIComponent *parent)
    : QGraphicsSvgItem(iconPath,parent)
{


    //Core interaction
    mpCorePort = corePort;
    //

    mpParentView = parent->mpParentGraphicsView;
    mpParentComponent = parent;

    //mType = type;
    mPortDirection = portDirection;

    setTransformOriginPoint(boundingRect().width()/2,boundingRect().height()/2);

    mX = x;
    mY = y;

    updatePosition();

    if(this->getPortType() == Port::POWERPORT)
        this->setRotation(0.0);
    else
        this->setRotation(rot);

//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    //QBrush brush(Qt::green);
    //this->setBrush(brush);

    mMag = 1.6180339887;
    mIsMag = false;
    isConnected = false;

    MainWindow *pMainWindow = mpParentComponent->mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;
    connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
    connect(pMainWindow->hidePortsAction,SIGNAL(triggered()),this, SLOT(hideIfNotConnected()));
    connect(pMainWindow->showPortsAction,SIGNAL(triggered()),this, SLOT(showIfNotConnected()));
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
        mIsMag = false;
    }
    else if ((blowup) && (!mIsMag))
    {
        this->scale(mMag, mMag);
        this->moveBy(-(mMag-1)*boundingRect().width()/2, -(mMag-1)*boundingRect().height()/2);
        mIsMag = true;
    }
}


//! Defines what happens when mouse cursor begins to hover a port.
//! @param *event defines the mouse event.
void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    std::cout << "GUIPort.cpp: " << "hovering over port" << std::endl;
    magnify(true);
}


void GUIPort::updatePosition()
{
    if(mpParentComponent->rotation() == 0)
        setPos(mX-this->boundingRect().width()/2,mY-this->boundingRect().height()/2);
    else if(mpParentComponent->rotation() == 90)
        setPos(mX-this->boundingRect().width()/2,mY+this->boundingRect().height()/2);
    else if(mpParentComponent->rotation() == 180)
        setPos(mX+this->boundingRect().width()/2,mY+this->boundingRect().height()/2);
    else
        setPos(mX+this->boundingRect().width()/2,mY-this->boundingRect().height()/2);
}

//! Defines what happens when mouse cursor stops hovering a port.
//! @param *event defines the mouse event.
void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QBrush brush(Qt::green);
    //this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
    magnify(false);
 //   QGraphicsSvgItem::hoverLeaveEvent(event);
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

    if ((!(this->mpCorePort->isConnected())) || (this->mpCorePort->getTimeVectorPtr()->empty()))
    {
        event->ignore();
    }
    else
    {
        QMenu menu;
        if (mpCorePort->getNodeType() =="NodeHydraulic")
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
        if (mpCorePort->getNodeType() =="NodeMechanic")
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
        if (mpCorePort->getNodeType() =="NodeSignal")
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
QGraphicsView *GUIPort::getParentView()
{
    return mpParentView;
}


//! Returns a pointer to the GUIComponent the port belongs to.
GUIComponent *GUIPort::getComponent()
{
    return mpParentComponent;
}


//! Plots the varable number 'nVar' in the node the port is connected to.
//! @param nVar tells which variable to plot.
void GUIPort::plot(size_t nVar) //En del vansinne i denna metoden...
{
    std::cout << "GUIPort.cpp: " << "Plot()" << std::endl;

    size_t dataLength = this->mpCorePort->getTimeVectorPtr()->size();

    QVector<double> time = QVector<double>::fromStdVector(*(this->mpCorePort->getTimeVectorPtr())); //Inte lampligt att skyffla data pa detta viset
    QVector<double> y(dataLength);// = QVector<double>::fromStdVector((this->mpCorePort->getDataVectorPtr()->at(1)));

    qDebug() << "Time size: " << time.size() << " last time: " << *time.end() << " datalength: " << dataLength << "y.size(): " << y.size();
    qDebug() << "time[0]: " << time[0] << " time[last-1]: " << time[time.size()-2] << " time[last]: " << time[time.size()-1];

    for (size_t i = 0; i<time.size(); ++i)
    {
        //qDebug() << time[i];
    }
    
    for (size_t i = 0; i<dataLength; ++i) //Denna loop ar inte klok
    {
        //timeq[i] = this->mpCorePort->getTimeVectorPtr()->at(i);
        y[i] = (this->mpCorePort->getDataVectorPtr()->at(i)).at(nVar);
    }

    qDebug() << "y[0]: " << y[0] << " y[last-1]: " << y[y.size()-2] << " y[last]: " << y[y.size()-1];

    QString title;
    QString xlabel;
    QString ylabel;

//    if (mpCorePort->getNodeType() == "NodeHydraulic")
//    {
//        if (nVar == 0)
//        {
//            title.append("Flow");
//            ylabel.append("Flow, [m^3/s]");
//        }
//        else if (nVar == 1)
//        {
//            title.append("Pressure");
//            ylabel.append("Pressure, [Pa]");
//        }
//    }
//    if (mpCorePort->getNodeType() == "NodeMechanic")
//    {
//        string name, unit;
//        mpCorePort->getNodeDataNameAndUnit(nVar, name, unit);
//        title.append(QString::fromStdString(name));
//        ylabel.append(QString::fromStdString(name) + ", [" + QString::fromStdString(unit) + "]");
//    }
//    else if (mpCorePort->getNodeType() == "NodeSignal")
//    {
//            title.append("Signal value");
//            ylabel.append("Value, [-]");
//    }
    string name, unit;
    mpCorePort->getNodeDataNameAndUnit(nVar, name, unit);
    title.append(QString::fromStdString(name));
    ylabel.append(QString::fromStdString(name) + ", [" + QString::fromStdString(unit) + "]");

    title.append(" at component: ").append(QString::fromStdString(mpParentComponent->mpCoreComponent->getName())).append(", port: ").append(QString::fromStdString(mpCorePort->getPortName()));
    xlabel.append("Time, [s]");

    PlotWidget *newPlot = new PlotWidget(time,y,mpParentComponent->mpParentGraphicsView);

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
    return this->getComponent()->getPortNumber(this);
}

//! Wrapper for the Core getPortType() function
Port::PORTTYPE GUIPort::getPortType()
{
    return mpCorePort->getPortType();
}


GUIPort::portDirectionType GUIPort::getPortDirection()
{
    return this->mPortDirection;
}

void GUIPort::setPortDirection(GUIPort::portDirectionType direction)
{
    this->mPortDirection = direction;
}



void GUIPort::hideIfNotConnected()
{
    if(!isConnected)
    {
        this->hide();
    }
}


void GUIPort::showIfNotConnected()
{
    if(!isConnected)
    {
        this->show();
    }
}
