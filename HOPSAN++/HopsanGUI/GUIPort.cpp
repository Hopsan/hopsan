//$Id$

//#include "HopsanCore.h"
#include "GUIPort.h"
#include "plotwidget.h"

#include <QtGui>

GUIPort::GUIPort(Port *corePort, qreal x, qreal y, QString iconPath,GUIComponent *parent)
    : QGraphicsSvgItem(iconPath,parent)
{
    //Core interaction
    mpCorePort = corePort;
    //

    this->

    mpParentView = parent->mpParentGraphicsView;
    mpParentComponent = parent;
    setPos(x-this->boundingRect().width()/2,y-this->boundingRect().height()/2);
//    pRectParent = parent;
    this->setAcceptHoverEvents(true);

    //QBrush brush(Qt::green);
    //this->setBrush(brush);

    mMag = 4.0;

    QObject::connect(this,SIGNAL(portClicked(GUIPort*)),this->getParentView(),SLOT(addConnector(GUIPort*)));
}

GUIPort::~GUIPort()
{
}

void GUIPort::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    this->setCursor(Qt::CrossCursor);
    QBrush brush(Qt::blue);
    //this->setBrush(brush);
    std::cout << "GUIPort.cpp: " << "hovering over port" << std::endl;
    this->scale(mMag, mMag);
    this->moveBy(-(mMag-1)*boundingRect().width()/2, -(mMag-1)*boundingRect().height()/2);
}

void GUIPort::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QBrush brush(Qt::green);
    //this->setBrush(brush);
    this->setCursor(Qt::ArrowCursor);
    this->moveBy((mMag-1)*boundingRect().width()/2, (mMag-1)*boundingRect().height()/2);
    this->scale(1/mMag,1/mMag);
}


QGraphicsView *GUIPort::getParentView()
{
    return mpParentView;
}

GUIComponent *GUIPort::getComponent()
{
    return mpParentComponent;
}


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

    PlotWidget *newPlot = new PlotWidget(time,y,mpParentComponent->mpParentGraphicsView);
    newPlot->show();

}
