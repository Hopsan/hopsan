#include "MovePortsDialog.h"
#include <algorithm>
#include <sstream>


using namespace std;

MovePortsDialog::MovePortsDialog(QWidget *parent)
    : QDialog(parent)
{
    mpView = new QGraphicsView(this);
    mpView->setScene(new QGraphicsScene());

    mpComponent = new QGraphicsSvgItem("../HopsanGUI/componentData/hydraulic/restrictors/checkvalve_iso.svg");
    mpView->scene()->addRect(mpComponent->boundingRect(), QPen(Qt::DashLine));
    mpView->scene()->addItem(mpComponent);

    mpPort = new DragPort("../HopsanGUI/graphics/porticons/HydraulicPort.svg");
    mpPort->setPosOnComponent(mpComponent, 0.0, .5);
    mpView->scene()->addItem(mpPort);

    //mpView->setSceneRect(mpComponent->boundingRect());
    mViewScale = 5;
    mpView->scale(mViewScale, mViewScale);
    //mpView->adjustSize();

    mpMainLayout = new QGridLayout(this);
    mpMainLayout->addWidget(mpView);
    //layout()->setSizeConstraint(QLayout::SetFixedSize);

    mpView->setDragMode(QGraphicsView::RubberBandDrag);
    mpView->setInteractive(true);
    mpView->setEnabled(true);
    mpView->setAcceptDrops(true);

    mpOkButton = new QPushButton("Ok");
    mpZoomSlider = new QSlider(Qt::Vertical, this);
    mpZoomSlider->setRange(10, 200);
    mpZoomSlider->setSliderPosition(mViewScale*10);

    QHBoxLayout *buttons = new QHBoxLayout();

    buttons->addWidget(mpOkButton);

    mpMainLayout->addLayout(buttons, 1, 0);
    mpMainLayout->addWidget(mpZoomSlider, 0, 1, 2, 1);

    connect(mpOkButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(mpZoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateZoom()));
    show();
}


void MovePortsDialog::updateZoom()
{
    mpView->setUpdatesEnabled(false);
    mpView->scale(1/mViewScale, 1/mViewScale);
    mpView->setUpdatesEnabled(true);
    mpView->scale(mpZoomSlider->sliderPosition()/10, mpZoomSlider->sliderPosition()/10);
    mViewScale = mpZoomSlider->sliderPosition()/10;
}


bool MovePortsDialog::close()
{
    QPointF p = mpPort->getPosOnComponent(mpComponent);
    stringstream ss;
    ss << "x: " << p.x() << "   y: " << p.y();
    qDebug() << ss;
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(ss.str()));
    msgBox.exec();

    QWidget::close();
}


DragPort::DragPort(QString path)
    : QGraphicsSvgItem(path)
{
    setAcceptDrops(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}


void DragPort::setPosOnComponent(QGraphicsItem *component, double x, double y)
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    setPos(QPointF(component->boundingRect().topLeft().x()+component->boundingRect().width()*x-ox, component->boundingRect().topLeft().y()+component->boundingRect().height()*y-oy));
}


QPointF DragPort::getPosOnComponent(QGraphicsItem *component)
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    double dx = (this->mapToScene(boundingRect().topLeft()).x()+ox) - component->mapToScene(component->boundingRect().topLeft()).x();
    double dy = (this->mapToScene(boundingRect().topLeft()).y()+oy) - component->mapToScene(component->boundingRect().topLeft()).y();

    return QPointF(max(0.0, min(1.0, dx/component->boundingRect().width())), max(0.0, min(1.0, dy/component->boundingRect().height())));
}
