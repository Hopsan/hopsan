/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#include <QGraphicsView>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>


#include "MovePortsDialog.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIPortAppearance.h"
#include <algorithm>

//! @brief Constructor for the move port dialog
//! @param[in] pModelObject Pointer to the model object which has the ports
//! @param[in] gfxType USER or ISO graphics for the port
//! @param[in] parent Pointer to the parent widget
MovePortsDialog::MovePortsDialog(ModelObject *pModelObject, GraphicsTypeEnumT gfxType, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(QString("Move ports on component: %1").arg(pModelObject->getName()));

    mpView = new QGraphicsView(this);
    mpView->setScene(new QGraphicsScene());

    ModelObjectAppearance *pComponentAppearance = pModelObject->getAppearanceData();
    const SharedModelObjectAppearanceT pLibraryComponentAppearance = pModelObject->getLibraryAppearanceData();
    auto* pSVGComponent = new QGraphicsSvgItem(pComponentAppearance->getFullAvailableIconPath(gfxType));
    mpView->scene()->addRect(pSVGComponent->boundingRect(), QPen(Qt::DashLine));
    mpView->scene()->addItem(pSVGComponent);

    mpActualPortAppearanceMap = &(pComponentAppearance->getPortAppearanceMap());

    auto* pPortEnableLayout = new QGridLayout();

    PortAppearanceMapT::Iterator it;
    for(it=mpActualPortAppearanceMap->begin(); it != mpActualPortAppearanceMap->end(); ++it)
    {
        const QString& rPortName = it.key();
        const SharedPortAppearanceT& rpPortAppearance = it.value();
        const auto *pPort = pModelObject->getPort(rPortName);
        const bool portEnabled = rpPortAppearance->mEnabled;
        const bool portConnected = pPort && pPort->isConnected();

        const SharedPortAppearanceT portLibraryAppearance = pLibraryComponentAppearance ? pLibraryComponentAppearance->getPortAppearance(rPortName) : nullptr;

        DragPort *pDragPort = new DragPort(rPortName, rpPortAppearance, portLibraryAppearance, pSVGComponent);

        pDragPort->setPosOnComponent(rpPortAppearance->x, rpPortAppearance->y, rpPortAppearance->rot);
        mpView->scene()->addItem(pDragPort);
        mDragPortMap.insert(rPortName, pDragPort);

        pDragPort->setVisible(portEnabled);
        QCheckBox *pEnabledCb = new QCheckBox(rPortName, this);
        pEnabledCb->setChecked(portEnabled);
        pEnabledCb->setDisabled(portConnected);
        pPortEnableLayout->addWidget(pEnabledCb);

        connect(pEnabledCb, SIGNAL(stateChanged(int)), pDragPort, SLOT(setEnable(int)), Qt::UniqueConnection);
        connect(pDragPort, SIGNAL(portMoved()), this, SLOT(updatePortInfo()), Qt::UniqueConnection);
        connect(pDragPort, SIGNAL(portSelected()), this, SLOT(setSelectPort()), Qt::UniqueConnection);
        connect(pDragPort, SIGNAL(portDisabled()), this, SLOT(disabledPort()), Qt::UniqueConnection);
    }

    //mpView->setSceneRect(mpComponent->boundingRect());
    mViewScale = 5;
    mpView->scale(mViewScale, mViewScale);
    //mpView->adjustSize();

    auto *pMainLayout = new QGridLayout(this);
    pMainLayout->addWidget(mpView);
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

    mpPortNameLabel = new QLabel(this);
    mpPortNameLabel->setMinimumWidth(50);

    mpPortXLineEdit = new QLineEdit(this);
    mpPortXLineEdit->setValidator(new QDoubleValidator(-0.1, 1.1, 3));
    mpPortYLineEdit = new QLineEdit(this);
    mpPortYLineEdit->setValidator(new QDoubleValidator(-0.1, 1.1, 3));
    mpPortALineEdit = new QLineEdit(this);
    mpPortALineEdit->setValidator(new QDoubleValidator(0, 360, 2));
    mpPortAutoCheckBox = new QCheckBox(this);
    mpResetButton = new QPushButton("Reset");


    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addWidget(mpOkButton);
    buttons->addWidget(mpCancelButton);

    pMainLayout->addLayout(buttons, 2, 0);
    pMainLayout->addWidget(mpZoomSlider, 0, 1, 2, 1);

    QHBoxLayout *pPortInfoLayout = new QHBoxLayout();
    pPortInfoLayout->addWidget(new QLabel("Selected port: ", this));
    pPortInfoLayout->addWidget(mpPortNameLabel);
    pPortInfoLayout->addWidget(new QLabel("X: ", this));
    pPortInfoLayout->addWidget(mpPortXLineEdit);
    pPortInfoLayout->addWidget(new QLabel("Y: ", this));
    pPortInfoLayout->addWidget(mpPortYLineEdit);
    pPortInfoLayout->addWidget(new QLabel("Ang: ", this));
    pPortInfoLayout->addWidget(mpPortALineEdit);
    pPortInfoLayout->addWidget(new QLabel("Auto: ", this));
    pPortInfoLayout->addWidget(mpPortAutoCheckBox);
    pPortInfoLayout->addWidget(mpResetButton);

    pPortInfoLayout->addStretch(1);
    pMainLayout->addLayout(pPortInfoLayout,1,0);
    pMainLayout->addLayout(pPortEnableLayout, 0, 2, 1, 1);

    connect(mpOkButton,      SIGNAL(clicked()),           this, SLOT(okButtonPressed()));
    connect(mpCancelButton,  SIGNAL(clicked()),           this, SLOT(cancelButtonPressed()));
    connect(mpZoomSlider,    SIGNAL(sliderMoved(int)),    this, SLOT(updateZoom()));

    clearPortInfo();
    this->setModal(true);
    show();
}


//! @brief Updates the x-position of a port in the move port dialog window
//! @param[in] x The new x-position
void DragPort::setPortXPos(QString x)
{
    mPortAppearance.x = x.toDouble();
    setPosOnComponent(mPortAppearance.x, mPortAppearance.y, mPortAppearance.rot);
    setPortAutoPlaced(false);
    mPortAppearance.mPoseModified = true;
}

//! @brief Updates the y-position of a port in the move port dialog window
//! @param[in] y The new y-position
void DragPort::setPortYPos(QString y)
{
    mPortAppearance.y = y.toDouble();
    setPosOnComponent(mPortAppearance.x, mPortAppearance.y, mPortAppearance.rot);
    setPortAutoPlaced(false);
    mPortAppearance.mPoseModified = true;
}

void DragPort::setPortRotation(QString a)
{
    mPortAppearance.rot = a.toDouble();
    setPosOnComponent(mPortAppearance.x, mPortAppearance.y, mPortAppearance.rot);
    setPortAutoPlaced(false);
    mPortAppearance.mPoseModified = true;
}

void DragPort::setPortAutoPlaced(bool ap)
{
    mPortAppearance.mAutoPlaced = ap;
    //! @todo should have reset position function also (but not here)
}

void MovePortsDialog::updatePortInfo(DragPort *pDragPort)
{
    if (!pDragPort)
    {
        pDragPort = qobject_cast<DragPort*>(sender());
    }

    QPointF p = pDragPort->getPosOnComponent();
    mpPortNameLabel->setText(pDragPort->getName());
    mpPortXLineEdit->setText(QString("%1").arg(p.x(),0,'g',3));
    mpPortYLineEdit->setText(QString("%1").arg(p.y(),0,'g',3));
    mpPortALineEdit->setText(QString("%1").arg(pDragPort->getPortRotation(),0,'f',0));
    mpPortAutoCheckBox->setChecked(pDragPort->getPortAppearance().mAutoPlaced);
    mpResetButton->setEnabled(pDragPort->getOriginalPortAppearance()!=0);
}

void MovePortsDialog::setSelectPort()
{
    disconnect(mpPortXLineEdit, 0, 0, 0);
    disconnect(mpPortYLineEdit, 0, 0, 0);
    disconnect(mpPortALineEdit, 0, 0, 0);
    disconnect(mpPortAutoCheckBox, 0, 0, 0);
    disconnect(mpResetButton, 0, 0, 0);

    DragPort *pDragPort = qobject_cast<DragPort*>(sender());
    updatePortInfo(pDragPort);

    connect(mpPortXLineEdit, SIGNAL(textEdited(QString)), pDragPort, SLOT(setPortXPos(QString)), Qt::UniqueConnection);
    connect(mpPortYLineEdit, SIGNAL(textEdited(QString)), pDragPort, SLOT(setPortYPos(QString)), Qt::UniqueConnection);
    connect(mpPortALineEdit, SIGNAL(textEdited(QString)), pDragPort, SLOT(setPortRotation(QString)), Qt::UniqueConnection);
    connect(mpPortAutoCheckBox, SIGNAL(clicked(bool)), pDragPort, SLOT(setPortAutoPlaced(bool)), Qt::UniqueConnection);
    connect(mpResetButton, SIGNAL(clicked()), pDragPort, SLOT(reset()), Qt::UniqueConnection);
}

void MovePortsDialog::disabledPort()
{
    DragPort *pDragPort = qobject_cast<DragPort*>(sender());
    disconnect(mpPortXLineEdit, 0, pDragPort, 0);
    disconnect(mpPortYLineEdit, 0, pDragPort, 0);
    disconnect(mpPortALineEdit, 0, pDragPort, 0);
    disconnect(mpPortAutoCheckBox, 0, pDragPort, 0);
    disconnect(mpResetButton, 0, pDragPort, 0);
    clearPortInfo();
}


//! @brief Updates the zoom of the component and ports
void MovePortsDialog::updateZoom()
{
   // mpView->setUpdatesEnabled(false);
  //  mpView->scale(1/mViewScale, 1/mViewScale);
  //  mpView->setUpdatesEnabled(true);
    mpView->scale(mpZoomSlider->sliderPosition()/10/mViewScale, mpZoomSlider->sliderPosition()/10/mViewScale);
    mViewScale = mpZoomSlider->sliderPosition()/10;
}


//! @brief Ok button is pressed
bool MovePortsDialog::okButtonPressed()
{
    QList<DragPort*> ports = mDragPortMap.values();
    for (int i=0; i<ports.size(); ++i)
    {
        PortAppearanceMapT::iterator it = mpActualPortAppearanceMap->find(ports[i]->getName());
        if (it != mpActualPortAppearanceMap->end())
        {
            it.value()->x = ports[i]->getPortAppearance().x;
            it.value()->y = ports[i]->getPortAppearance().y;
            it.value()->rot = ports[i]->getPortAppearance().rot;
            it.value()->mAutoPlaced = ports[i]->getPortAppearance().mAutoPlaced;
            it.value()->mEnabled = ports[i]->getPortAppearance().mEnabled;
            it.value()->mPoseModified = ports[i]->getPortAppearance().mPoseModified;
        }
        //! @todo enabled and autoplaced are set immediately so even if you select cancel they will be set
    }

    emit finished();
    return close();
}


//! @brief Cancel button pressed
bool MovePortsDialog::cancelButtonPressed()
{
    return close();
}


//! @brief Constructor for the port that can be dragged around
//! @param[in] pAppearance Pointer to the port appearance data of the port that should be used
//! @param[in] pOriginalAppearance The original port appearance, for reset (optional)
//! @param[in] pParentComponent Pointer to the parent component graphics item on which the port is placed
DragPort::DragPort(QString name, const SharedPortAppearanceT pAppearance, const SharedPortAppearanceT pOriginalAppearance, QGraphicsItem *pParentComponent)
    : QGraphicsWidget(pParentComponent), mpOriginalPortAppearance(pOriginalAppearance)
{
    mPortAppearance = *pAppearance;
    mpParentComponent = pParentComponent;
    mpSvg = new QGraphicsSvgItem(mPortAppearance.mMainIconPath, this);
    setGeometry(mpSvg->boundingRect());
    mpName = new QGraphicsTextItem(name, this);
    //! @todo need help object representing svgitem (or graphics in general) with centered overlayed text
    //mpName->setPos(-mpName->boundingRect().width()/2.0, -mpName->boundingRect().height()/2.0);
    QFont font = QFont();
    font.setPointSize(6);
    mpName->setFont(font);
    setAcceptDrops(true);
    setFlags(QGraphicsItem::ItemIsMovable);
}


//! @brief Re-implemented event function when mouse is moved
//! @param[in] event The mouse event
void DragPort::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseMoveEvent(event);
    setPortAutoPlaced(false);
    refreshLocalAppearanceData();
    mPortAppearance.mPoseModified = true;
    emit portMoved();
}


//! @brief Re-implemented event function when mouse is pressed
//! @param[in] event The mouse event
void DragPort::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mousePressEvent(event);
    emit portSelected();
}

void DragPort::refreshLocalAppearanceData()
{
    QPointF p = getPosOnComponent();
    mPortAppearance.x = p.x();
    mPortAppearance.y = p.y();
    mPortAppearance.rot = getPortRotation();
}

//! @brief Moves the position and rotation of the port in the dialog window
//! @param[in] x The new x-position
//! @param[in] y The new y-position
//! @param[in] rot The new rot-position
void DragPort::setPosOnComponent(double x, double y, double rot)
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    mpSvg->setTransformOriginPoint(mpSvg->boundingRect().center());
    mpSvg->setRotation(rot);

    setPos(QPointF(mpParentComponent->boundingRect().topLeft().x()+mpParentComponent->boundingRect().width()*x-ox,
                   mpParentComponent->boundingRect().topLeft().y()+mpParentComponent->boundingRect().height()*y-oy));
}


//! @brief Calculates the position of the port on its parent component in scene coordinates
//! @return QPoint The resulting point
QPointF DragPort::getPosOnComponent()
{
    double ox = this->boundingRect().width()/2.0;
    double oy = this->boundingRect().height()/2.0;

    double dx = (this->mapToScene(boundingRect().topLeft()).x()+ox) - mpParentComponent->mapToScene(mpParentComponent->boundingRect().topLeft()).x();
    double dy = (this->mapToScene(boundingRect().topLeft()).y()+oy) - mpParentComponent->mapToScene(mpParentComponent->boundingRect().topLeft()).y();

    return QPointF(qMax(0.0, qMin(1.0, dx/mpParentComponent->boundingRect().width())), qMax(0.0, qMin(1.0, dy/mpParentComponent->boundingRect().height())));
}

//! @brief Returns the rotation of the portgraphics
double DragPort::getPortRotation()
{
    return mpSvg->rotation();
}

QString DragPort::getName()
{
    return mpName->toPlainText();
}

const PortAppearance &DragPort::getPortAppearance() const
{
    return mPortAppearance;
}

const SharedPortAppearanceT DragPort::getOriginalPortAppearance() const
{
    return mpOriginalPortAppearance;
}

void DragPort::reset()
{
    if (mpOriginalPortAppearance)
    {
        mPortAppearance = *mpOriginalPortAppearance;
        this->setPosOnComponent(mPortAppearance.x, mPortAppearance.y, mPortAppearance.rot);
        emit portMoved();
    }
}


void DragPort::setEnable(int state)
{
    if(1 > state)
    {
        mPortAppearance.mEnabled = false;
        setVisible(false);
        emit portDisabled();
    }
    else
    {
        mPortAppearance.mEnabled = true;
        setVisible(true);
    }
}


void MovePortsDialog::clearPortInfo()
{
    mpPortNameLabel->setText("-");
    mpPortXLineEdit->clear();
    mpPortYLineEdit->clear();
    mpPortALineEdit->clear();
}
