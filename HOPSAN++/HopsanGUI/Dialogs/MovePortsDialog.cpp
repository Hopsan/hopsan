#include "MovePortsDialog.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIPortAppearance.h"
#include <algorithm>
#include <sstream>


using namespace std;

MovePortsDialog::MovePortsDialog(Component *pGUIComponent, QWidget *parent)
    : QDialog(parent)
{
    mpView = new QGraphicsView(this);
    mpView->setScene(new QGraphicsScene());

    mpComponent = pGUIComponent;

    mpCompAppearance = pGUIComponent->getAppearanceData();
    mpSVGComponent = new QGraphicsSvgItem(mpCompAppearance->getIconPath(pGUIComponent->getParentContainerObject()->getGfxType(), ABSOLUTE));
    mpView->scene()->addRect(mpSVGComponent->boundingRect(), QPen(Qt::DashLine));
    mpView->scene()->addItem(mpSVGComponent);

    mPortAppearanceMap = mpCompAppearance->getPortAppearanceMap();

    PortAppearanceMapT::Iterator it;
    for(it=mPortAppearanceMap.begin(); it != mPortAppearanceMap.end(); ++it)
    {
        DragPort *pPort = new DragPort(it->mMainIconPath);
        mvSVGPorts.append(pPort);
        pPort->setPosOnComponent(mpSVGComponent, it->x, it->y, it->rot);
        mpView->scene()->addItem(pPort);
    }

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
    mpCancelButton = new QPushButton("Cancel");
    mpZoomSlider = new QSlider(Qt::Vertical, this);
    mpZoomSlider->setRange(10, 200);
    mpZoomSlider->setSliderPosition(mViewScale*10);

    QHBoxLayout *buttons = new QHBoxLayout();

    buttons->addWidget(mpOkButton);
    buttons->addWidget(mpCancelButton);

    mpMainLayout->addLayout(buttons, 1, 0);
    mpMainLayout->addWidget(mpZoomSlider, 0, 1, 2, 1);

    connect(mpOkButton, SIGNAL(clicked()), this, SLOT(okButtonPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPressed()));
    connect(mpZoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateZoom()));

    this->setModal(true);

    show();
}


void MovePortsDialog::updateZoom()
{
   // mpView->setUpdatesEnabled(false);
  //  mpView->scale(1/mViewScale, 1/mViewScale);
  //  mpView->setUpdatesEnabled(true);
    mpView->scale(mpZoomSlider->sliderPosition()/10/mViewScale, mpZoomSlider->sliderPosition()/10/mViewScale);
    mViewScale = mpZoomSlider->sliderPosition()/10;
}


bool MovePortsDialog::okButtonPressed()
{
    stringstream ss;
    PortAppearanceMapT::Iterator it;
    int i = 0;
    for(it=mPortAppearanceMap.begin(); it != mPortAppearanceMap.end(); ++it)
    {
        QPointF p = mvSVGPorts.at(i)->getPosOnComponent(mpSVGComponent);
        ss << it.key().toStdString() << " - x: " << p.x() << "   y: " << p.y() << "\n";
        ++i;

        Port *port = mpComponent->getPort(it.key());
        port->setCenterPosByFraction(p.x(), p.y());
    }
    qDebug() << ss;
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(ss.str()));

    msgBox.exec();

    return close();
}


bool MovePortsDialog::cancelButtonPressed()
{
    return close();
}


DragPort::DragPort(QString path)
    : QGraphicsWidget()
{
    mpSvg = new QGraphicsSvgItem(path, this);
    setGeometry(mpSvg->boundingRect());
    setAcceptDrops(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}


void DragPort::setPosOnComponent(QGraphicsItem *component, double x, double y, double rot)
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    mpSvg->setTransformOriginPoint(mpSvg->boundingRect().center());
    mpSvg->setRotation(rot);

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
