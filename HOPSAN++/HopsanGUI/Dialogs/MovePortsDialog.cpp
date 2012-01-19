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


MovePortsDialog::MovePortsDialog(ModelObjectAppearance *pComponentAppearance, graphicsType gfxType, QWidget *parent)
    : QDialog(parent)
{
    mpView = new QGraphicsView(this);
    mpView->setScene(new QGraphicsScene());

    mpCompAppearance = pComponentAppearance;
    mpSVGComponent = new QGraphicsSvgItem(mpCompAppearance->getIconPath(gfxType, ABSOLUTE));
    mpView->scene()->addRect(mpSVGComponent->boundingRect(), QPen(Qt::DashLine));
    mpView->scene()->addItem(mpSVGComponent);

    mpPortAppearanceMap = &(mpCompAppearance->getPortAppearanceMap());

    PortAppearanceMapT::Iterator it;
    for(it=mpPortAppearanceMap->begin(); it != mpPortAppearanceMap->end(); ++it)
    {
        DragPort *pPort = new DragPort(&(*it), it.key(), mpSVGComponent);
        mvSVGPorts.append(pPort);
        pPort->setPosOnComponent(it->x, it->y, it->rot);
        mpView->scene()->addItem(pPort);
        mDragPortMap.insert(it.key(), pPort);

        connect(pPort, SIGNAL(activePort(QString,QString,QString)), this, SLOT(updatePortInfo(QString,QString,QString)), Qt::UniqueConnection);
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
    mpSelectedPortLabel = new QLabel("Selected port: ", this);
    mpPortNameLabel = new QLabel(this);
    mpPortNameLabel->setMinimumWidth(50);
    mpSelectedPortXLabel = new QLabel("X: ", this);
    mpPortXLineEdit = new QLineEdit(this);
    mpPortXLineEdit->setValidator(new QDoubleValidator);
    mpSelectedPortYLabel = new QLabel("Y: ", this);
    mpPortYLineEdit = new QLineEdit(this);
    mpPortYLineEdit->setValidator(new QDoubleValidator);

    QHBoxLayout *buttons = new QHBoxLayout();

    buttons->addWidget(mpOkButton);
    buttons->addWidget(mpCancelButton);

    mpMainLayout->addLayout(buttons, 2, 0);
    mpMainLayout->addWidget(mpZoomSlider, 0, 1, 2, 1);
    QHBoxLayout *pPortInfoLayout = new QHBoxLayout();
    pPortInfoLayout->addWidget(mpSelectedPortLabel);
    pPortInfoLayout->addWidget(mpPortNameLabel);
    pPortInfoLayout->addWidget(mpSelectedPortXLabel);
    pPortInfoLayout->addWidget(mpPortXLineEdit);
    pPortInfoLayout->addWidget(mpSelectedPortYLabel);
    pPortInfoLayout->addWidget(mpPortYLineEdit);
    pPortInfoLayout->addStretch(1);
    mpMainLayout->addLayout(pPortInfoLayout,1,0);

    connect(mpOkButton, SIGNAL(clicked()), this, SLOT(okButtonPressed()));
    connect(mpCancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonPressed()));
    connect(mpZoomSlider, SIGNAL(sliderMoved(int)), this, SLOT(updateZoom()));
    connect(mpPortXLineEdit, SIGNAL(textChanged(QString)), this, SLOT(updatePortXPos(QString)));
    connect(mpPortYLineEdit, SIGNAL(textChanged(QString)), this, SLOT(updatePortYPos(QString)));

    this->setModal(true);

    show();
}


void MovePortsDialog::updatePortXPos(QString x)
{
    PortAppearance app = *(mpPortAppearanceMap->find(mpPortNameLabel->text()));
    (*mDragPortMap.find(mpPortNameLabel->text()))->setPosOnComponent(x.toDouble(), app.y, app.rot);
}


void MovePortsDialog::updatePortYPos(QString y)
{
    PortAppearance app = *(mpPortAppearanceMap->find(mpPortNameLabel->text()));
    (*mDragPortMap.find(mpPortNameLabel->text()))->setPosOnComponent(app.x, y.toDouble(), app.rot);
}


void MovePortsDialog::updatePortInfo(QString portName, QString x, QString y)
{
    mpPortNameLabel->setText(portName);
    mpPortXLineEdit->setText(x);
    mpPortYLineEdit->setText(y);
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
    for(it=mpPortAppearanceMap->begin(); it != mpPortAppearanceMap->end(); ++it)
    {
        QPointF p = mvSVGPorts.at(i)->getPosOnComponent();
        ss << it.key().toStdString() << " - x: " << p.x() << "   y: " << p.y() << "\n";
        ++i;

        it.value().x = p.x();
        it.value().y = p.y();

        //Port *port = mpComponent->getPort(it.key());
        //port->setCenterPosByFraction(p.x(), p.y());
    }
    qDebug() << ss;
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(ss.str()));

    msgBox.exec();

    //mpComponent->redrawConnectors();

    emit finished();

    return close();
}


bool MovePortsDialog::cancelButtonPressed()
{
    return close();
}


DragPort::DragPort(PortAppearance *appearance, QString name, QGraphicsItem *parentComponent)
    : QGraphicsWidget()
{
    mpParentComponent = parentComponent;
    mpSvg = new QGraphicsSvgItem(appearance->mMainIconPath, this);
    mpName = new QGraphicsTextItem(name, this);
    QFont font = QFont();
    font.setPointSize(6);
    mpName->setFont(font);
    setGeometry(mpSvg->boundingRect());
    setAcceptDrops(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}


void DragPort::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    portMoved();
    QGraphicsWidget::mouseMoveEvent(event);
}


void DragPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    portMoved();
    QGraphicsWidget::mousePressEvent(event);
}


void DragPort::portMoved()
{
    QPointF p = getPosOnComponent();
    QString x,y;
    x.setNum(p.x());
    y.setNum(p.y());
    emit activePort(mpName->toPlainText(), x, y);
}


void DragPort::setPosOnComponent(double x, double y, double rot)
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    mpSvg->setTransformOriginPoint(mpSvg->boundingRect().center());
    mpSvg->setRotation(rot);

    setPos(QPointF(mpParentComponent->boundingRect().topLeft().x()+mpParentComponent->boundingRect().width()*x-ox, mpParentComponent->boundingRect().topLeft().y()+mpParentComponent->boundingRect().height()*y-oy));
}


QPointF DragPort::getPosOnComponent()
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    double dx = (this->mapToScene(boundingRect().topLeft()).x()+ox) - mpParentComponent->mapToScene(mpParentComponent->boundingRect().topLeft()).x();
    double dy = (this->mapToScene(boundingRect().topLeft()).y()+oy) - mpParentComponent->mapToScene(mpParentComponent->boundingRect().topLeft()).y();

    return QPointF(max(0.0, min(1.0, dx/mpParentComponent->boundingRect().width())), max(0.0, min(1.0, dy/mpParentComponent->boundingRect().height())));
}
