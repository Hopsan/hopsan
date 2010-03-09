//$Id$

//#include "HopsanCore.h"
#include "GUIPort.h"
#include "plotwidget.h"

#include <QtGui>


//! Constructor.
//! @param corePort a pointer to the corresponing port in Core.
//! @param x the x-coord. of where the port should be placed.
//! @param y the y-coord. of where the port should be placed.
//! @param rot how the port should be rotated.
//! @param iconPath a string with the path to the svg-figure representing the port.
//! @param parent the port's parent, the component it is a part of.
GUIPort::GUIPort(Port *corePort, qreal x, qreal y, qreal rot, QString iconPath, GUIComponent *parent)
    : QGraphicsSvgItem(iconPath,parent)
{
    //Core interaction
    mpCorePort = corePort;
    //

    mpParentView = parent->mpParentGraphicsView;
    mpParentComponent = parent;

    setTransformOriginPoint(boundingRect().width()/2,boundingRect().height()/2);

    setPos(x-this->boundingRect().width()/2,y-this->boundingRect().height()/2);
    this->setRotation(rot);

//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    //QBrush brush(Qt::green);
    //this->setBrush(brush);

    mMag = 1.6180339887;
    mIsMag = false;

    QObject::connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
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
    
    for (int i = 0; i<dataLength; ++i) //Denna loop ar inte klok
    {
        //timeq[i] = this->mpCorePort->getTimeVectorPtr()->at(i);
        y[i] = (this->mpCorePort->getDataVectorPtr()->at(i)).at(nVar);
    }

    QString title;
    QString xlabel;
    QString ylabel;

    if (mpCorePort->getNodeType() == "NodeHydraulic")
    {
        if (nVar == 0)
        {
            title.append("Flow");
            ylabel.append("Flow, [m^3/s]");
        }
        else if (nVar == 1)
        {
            title.append("Pressure");
            ylabel.append("Pressure, [Pa]");
        }
    }
    else if (mpCorePort->getNodeType() == "NodeSignal")
    {
            title.append("Signal value");
            ylabel.append("Value, [-]");
    }

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
