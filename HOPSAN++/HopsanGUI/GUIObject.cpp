//$Id$

#include <iostream>
#include <ostream>
#include <assert.h>
#include <vector>
#include <math.h>

#include <QObject>
#include <QDebug>
#include <QtGui>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneMoveEvent>

#include "HopsanCore.h"
#include "GUIObject.h"
#include "mainwindow.h"
#include "ParameterDialog.h"
#include "GUIPort.h"
#include "GUIConnector.h"


double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

GUIObject::GUIObject(QPoint position, QString iconPath, GraphicsScene *scene, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentGraphicsScene = scene;
    mpParentGraphicsScene->addItem(this);
    mpParentGraphicsView = mpParentGraphicsScene->mpParentProjectTab->mpGraphicsView;

    mTextOffset = 5.0;

    this->hasIsoIcon = false;

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    //setFocusPolicy(Qt::StrongFocus);
    this->setAcceptHoverEvents(true);

    this->setZValue(10);
    mpIcon = new QGraphicsSvgItem(iconPath,this);
    mIconPath = iconPath;

    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;

    setGeometry(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height());

    mpNameText = new GUIObjectDisplayName(this);
    //mpNameText->setPos(QPointF(mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2, mTextOffset*mpIcon->boundingRect().height()));
    mNameTextPos = 0;
    this->setNameTextPos(mNameTextPos);

    mpSelectionBox = new GUIObjectSelectionBox(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height(),
                                                  QPen(QColor("red"),2*1.6180339887499), QPen(QColor("darkRed"),2*1.6180339887499),this);
    mpSelectionBox->setVisible(false);

    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    //connect(this->mpParentGraphicsView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));

    //setPos(position-QPoint(mpIcon->boundingRect().width()/2, mpIcon->boundingRect().height()/2));
    setPos(position.x()-mpIcon->boundingRect().width()/2,position.y()-mpIcon->boundingRect().height()/2);
}


GUIObject::~GUIObject()
{
    //delete widget;
    emit componentDeleted();
}


//GUIComponent::GUIComponent(HopsanEssentials *hopsan, const QString &fileName, QString componentTypeName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent)
//        : QGraphicsWidget(parent)
//{
//    //Core interaction
//    mpCoreComponent = hopsan->CreateComponent(componentTypeName.toStdString());
//    //
//
//    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
//    this->setAcceptHoverEvents(true);
//
//    //widget = new QWidget;
//
//    mpParentView = parentView;
//
//    this->setZValue(10);
//    icon = new QGraphicsSvgItem(fileName,this);
////    icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
//    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
//    std::cout << "GUIcomponent: " << componentTypeName.toStdString() << std::endl;
//
//    //setWindowFlags(Qt::SplashScreen);//just to see the geometry
//    setGeometry(0,0,icon->boundingRect().width(),icon->boundingRect().height());
//
//    mpNameText = new GUIComponentNameTextItem(mpCoreComponent, this);
//    mpNameText->setPos(QPointF(icon->boundingRect().width()/2-mpNameText->boundingRect().width()/2, icon->boundingRect().height()));
//
//    //UGLY UGLY HARD CODED PORT CONNECTION TO CORE...
//    mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(0), icon->sceneBoundingRect().width()-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon));
//    mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(1),-5,icon->sceneBoundingRect().height()/2-5,10.0,10.0,this->getParentView(),this,icon));
//
//    this->showPorts(false);
//
//    //icon->setPos(QPointF(-icon->boundingRect().width()/2, -icon->boundingRect().height()/2));
//
//   // rectR->boundingRegion();
//
//    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
//    connect(this->mpParentView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));
//
//    setPos(position-QPoint(icon->boundingRect().width()/2, icon->boundingRect().height()/2));
//
//    mpSelectionBox = new GUIComponentSelectionBox(0,0,icon->boundingRect().width(),icon->boundingRect().height(),
//                                                  QPen(QColor("red"),3), QPen(QColor("darkRed"),2),this);
//    mpSelectionBox->setVisible(false);
//}


int GUIObject::type() const
{
    return Type;
}


void GUIObject::fixTextPosition(QPointF pos)
{
    double x1,x2,y1,y2;

    if(this->rotation() == 0)
    {
        x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
        x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
    }
    else if(this->rotation() == 180)
    {
        x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
        x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y2 = -mTextOffset;
    }
    else if(this->rotation() == 90)
    {
        x1 = -mpNameText->boundingRect().height() - mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        x2 = mpIcon->boundingRect().width() + mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
    }
    else if(this->rotation() == 270)
    {
        x1 = mpIcon->boundingRect().width() + mpNameText->boundingRect().height() + mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
        x2 = -mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
    }

    double x = mpNameText->mapToParent(pos).x();
    double y = mpNameText->mapToParent(pos).y();

    if (dist(x,y, x1,y1) > dist(x,y, x2,y2))
    {
        mpNameText->setPos(x2,y2);
        mNameTextPos = 0;
    }
    else
    {
        mpNameText->setPos(x1,y1);
        mNameTextPos = 1;
    }

    std::cout << "GUIComponent::fixTextPosition, x: " << x << " y: " << y << std::endl;

}


//QGraphicsView *GUIComponent::getParentView()
//{
//    return mpParentView;
//}

void GUIObject::addConnector(GUIConnector *item)
{
    connect(this, SIGNAL(componentMoved()), item, SLOT(updatePos()));
}

//! This function refreshes the displayed name (HopsanCore may have changed it)
void GUIObject::refreshName()
{
    mpNameText->setPlainText(getName());
    //Adjust the position of the text
    this->fixTextPosition(this->mpNameText->pos());
}


//! This function returns the current component name
QString GUIObject::getName()
{
    return this->mpNameText->toPlainText();
}

void GUIObject::deleteInHopsanCore()
{
    cout << "Virtual dummy function" << endl;
    assert(false);
}

void GUIObject::setName(QString newName, bool doOnlyCoreRename)
{
    mpNameText->setPlainText(newName);
}


//GUIObject::setIcon(bool useIso)
//{
//    //if(useIso)
//    //    this->mpIcon->
//}

//! Returns the port with the specified number.
//! @see getPortNumber(GUIPort *port)
GUIPort *GUIObject::getPort(int number)
{
    return this->mPortListPtrs[number];
}

Component* GUIObject::getHopsanCoreComponentPtr()
{
    cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
}

ComponentSystem* GUIObject::getHopsanCoreSystemComponentPtr()
{
    cout << "This function should only be available in GUISubsystem" << endl;
    assert(false);
}


//! Event when mouse cursor enters component icon.
void GUIObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setHovered();
        //this->mpSelectionBox->setVisible(true);
    }
    this->showPorts(true);
}


//! Event when mouse cursor leaves component icon.
void GUIObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setPassive();
    }
    this->showPorts(false);
}

//! Handles item change events.
QVariant GUIObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsWidget::itemChange(change, value);

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            this->mpSelectionBox->setActive();
            connect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressR()), this, SLOT(rotate()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressUp()), this, SLOT(moveUp()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressDown()), this, SLOT(moveDown()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressLeft()), this, SLOT(moveLeft()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressRight()), this, SLOT(moveRight()));
        }
        else
        {
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressR()), this, SLOT(rotate()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressUp()), this, SLOT(moveUp()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDown()), this, SLOT(moveDown()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressLeft()), this, SLOT(moveLeft()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressRight()), this, SLOT(moveRight()));
            this->mpSelectionBox->setPassive();
        }
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit componentMoved();
    }
    return value;
}


//! Shows or hides the port, depending on the input boolean and whether or not they are connected.
void GUIObject::showPorts(bool visible)
{
    QList<GUIPort*>::iterator i;
    if(visible)
    {
        for (i = mPortListPtrs.begin(); i != mPortListPtrs.end(); ++i)
        {
            (*i)->show();
        }
    }
    else
        for (i = mPortListPtrs.begin(); i != mPortListPtrs.end(); ++i)
        {
        if ((*i)->mpCorePort->isConnected())
            (*i)->hide();
        }
}


//! Figures out the number of a component port by using a pointer to the port.
//! @see getPort(int number)
int GUIObject::getPortNumber(GUIPort *port)
{
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(port == mPortListPtrs.value(i))
        {
            return i;
        }
    }
    assert(false);      /// @todo: Cast exception
}


//! Rotates a component 90 degrees clockwise, and tells the connectors that the component has moved.
void GUIObject::rotate()
{
    int temNameTextPos = mNameTextPos;
    this->setTransformOriginPoint(this->mpIcon->boundingRect().center());
    this->setRotation(this->rotation()+90);
    if (this->rotation() == 360)
    {
        this->setRotation(0);
    }
    this->mpNameText->setRotation(-this->rotation());
    this->fixTextPosition(this->mpNameText->pos());
    this->setNameTextPos(temNameTextPos);
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        if(mPortListPtrs.value(i)->getPortDirection() == GUIPort::VERTICAL)
            mPortListPtrs.value(i)->setPortDirection(GUIPort::HORIZONTAL);
        else
            mPortListPtrs.value(i)->setPortDirection(GUIPort::VERTICAL);
        if (mPortListPtrs.value(i)->getPortType() == Port::POWERPORT)
            mPortListPtrs.value(i)->setRotation(-this->rotation());
    }
    if(!this->mIconRotation)
    {
        this->mpIcon->setRotation(-this->rotation());
        if(this->rotation() == 0)
        {
            this->mpIcon->setPos(0,0);
        }
        else if(this->rotation() == 90)
        {
            this->mpIcon->setPos(0,this->boundingRect().height());
        }
        else if(this->rotation() == 180)
        {
            this->mpIcon->setPos(this->boundingRect().width(),this->boundingRect().height());
        }
        else if(this->rotation() == 270)
        {
            this->mpIcon->setPos(this->boundingRect().width(),0);
        }

        //this->mpIcon->setPos(this->boundingRect().center());
    }
    emit componentMoved();
}


//! Slot that moves component one pixel upwards
//! @see moveDown()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveUp()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()-1);
}


//! Slot that moves component one pixel downwards
//! @see moveUp()
//! @see moveLeft()
//! @see moveRight()
void GUIObject::moveDown()
{
    this->setPos(this->pos().x(), this->mapFromScene(this->mapToScene(this->pos())).y()+1);
}


//! Slot that moves component one pixel leftwards
//! @see moveUp()
//! @see moveDown()
//! @see moveRight()
void GUIObject::moveLeft()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()-1, this->pos().y());
}


//! Slot that moves component one pixel rightwards
//! @see moveUp()
//! @see moveDown()
//! @see moveLeft()
void GUIObject::moveRight()
{
    this->setPos(this->mapFromScene(this->mapToScene(this->pos())).x()+1, this->pos().y());
}


//! Returns an integer that describes the position of the component name text.
//! @see setNameTextPos(int textPos)
//! @see fixTextPosition(QPointF pos)
int GUIObject::getNameTextPos()
{
    return mNameTextPos;
}\


//! Updates the name text position, and moves the text to the correct position.
//! @see getNameTextPos()
//! @see fixTextPosition(QPointF pos)
void GUIObject::setNameTextPos(int textPos)
{
    mNameTextPos = textPos;

    double x1,x2,y1,y2;

    if(this->rotation() == 0)
    {
        x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
        x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
    }
    else if(this->rotation() == 180)
    {
        x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
        x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y2 = -mTextOffset;
    }
    else if(this->rotation() == 90)
    {
        x1 = -mpNameText->boundingRect().height() - mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        x2 = mpIcon->boundingRect().width() + mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
    }
    else if(this->rotation() == 270)
    {
        x1 = mpIcon->boundingRect().width() + mpNameText->boundingRect().height() + mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
        x2 = -mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 - mpNameText->boundingRect().width()/2;
    }

    switch(textPos)
    {
    case 0:
        mpNameText->setPos(x2,y2);
        break;
    case 1:
        mpNameText->setPos(x1,y1);
        break;
    }
}

void GUIObject::hideName()
{
    this->mpNameText->setVisible(false);
}

void GUIObject::showName()
{
    this->mpNameText->setVisible(true);
}


//! Dummy
QString GUIObject::getTypeName()
{

    assert(false);
}


GUIObjectDisplayName::GUIObjectDisplayName(GUIObject *pParent)
    :   QGraphicsTextItem(pParent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
//    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    //setTextInteractionFlags(Qt::TextEditorInteraction);
    mpParentGUIComponent = pParent;
}


void GUIObjectDisplayName::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
}


void GUIObjectDisplayName::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit textMoved(event->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}

void GUIObjectDisplayName::focusOutEvent(QFocusEvent *event)
{
    //Try to set the new name, the rename function in parent view will be called
    mpParentGUIComponent->setName(toPlainText());
    //refresh the display name (it may be different from the one you wanted)
    mpParentGUIComponent->refreshName();
    emit textMoved(pos());

    setTextInteractionFlags(Qt::NoTextInteraction);

    QGraphicsTextItem::focusOutEvent(event);
}




//! Constructor.
//! @param x1 is the x-coordinate of the top left corner of the parent component.
//! @param y1 is the y-coordinate of the top left corner of the parent component.
//! @param x2 is the x-coordinate of the bottom right corner of the parent component.
//! @param y2 is the y-coordinate of the bottom right corner of the parent component.
//! @param activePen defines the width and color of the box when the parent component is selected.
//! @param hoverPen defines the width and color of the box when the parent component is hovered by the mouse cursor.
//! @param *parent defines the parent object.
GUIObjectSelectionBox::GUIObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, GUIObject *parent)
        : QGraphicsItemGroup(parent)
{
    mpParentGUIObject = parent;
    qreal b = 5;
    qreal a = b*mpParentGUIObject->boundingRect().width()/mpParentGUIObject->boundingRect().height();
    x1 = x1-3;
    y1 = y1-3;
    x2 = x2+3;
    y2 = y2+3;

    this->mActivePen = activePen;
    this->mHoverPen = hoverPen;

    QGraphicsLineItem *tempLine = new QGraphicsLineItem(x1,y1+b,x1,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y1,x1+a,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2-a,y1,x2,y1,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y1,x2,y1+b,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1+a,y2,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x1,y2-b,x1,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2-b,x2,y2,this);
    mLines.push_back(tempLine);
    tempLine = new QGraphicsLineItem(x2,y2,x2-a,y2,this);
    mLines.push_back(tempLine);
}


//! Destructor.
GUIObjectSelectionBox::~GUIObjectSelectionBox()
{
}


//! Tells the box to become visible and use active style.
//! @see setPassive();
//! @see setHovered();
void GUIObjectSelectionBox::setActive()
{

    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mActivePen);
    }
}


//! Tells the box to become invisible.
//! @see setActive();
//! @see setHovered();
void GUIObjectSelectionBox::setPassive()
{
    this->setVisible(false);
}


//! Tells the box to become visible and use hovered style.
//! @see setActive();
//! @see setPassive();
void GUIObjectSelectionBox::setHovered()
{
    this->setVisible(true);
    for(std::size_t i=0;i!=mLines.size();++i)
    {
        mLines[i]->setPen(this->mHoverPen);
    }
}

//! Tells the component to ask its parent to delete it.
void GUIObject::deleteMe()
{
    mpParentGraphicsView->deleteGUIObject(this->getName());
}

//void GUIObject::groupComponents(QList<QGraphicsItem*> compList) //Inte alls klart
//{
//    //Borde nog ligga i projecttab så man kan rodda med scenerna
//
//    QList<GUIComponent*> GUICompList;
//    QList<GUIConnector*> GUIConnList;
//
//    MessageWidget *pMessageWidget = mpParentGraphicsScene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;
//    pMessageWidget->printGUIMessage("Group selected components (implementing in progress...) Selected components: ");
//    for (int i=0; i < compList.size(); ++i)
//    {
//        GUIComponent *pComponent = qgraphicsitem_cast<GUIComponent*>(compList.at(i));
//        if (pComponent)
//        {
//            GUICompList.append(pComponent);
//
//            QMap<QString, GUIConnector *>::iterator it;
//            for(it = this->mpParentGraphicsView->mConnectionMap.begin(); it!=this->mpParentGraphicsView->mConnectionMap.end(); ++it)
//            {
//                if(it.key().contains(pComponent->getName()))
//                    if((compList.contains(it.value()->getStartPort()->getComponent())) && (compList.contains(it.value()->getEndPort()->getComponent())))
//                        GUIConnList.append(it.value());
//
//                if(this->mpParentGraphicsView->mConnectionMap.empty())
//                    break;
//            }
//        }
//    }
//
//    GraphicsScene *pSubScene = new GraphicsScene(this->mpParentGraphicsScene->mpParentProjectTab);
//    for (int i=0; i < GUICompList.size(); ++i)
//    {
//        pSubScene->addItem(GUICompList.at(i));
//    }
//    for (int i=0; i < GUIConnList.size(); ++i)
//    {
//        pSubScene->addItem(GUIConnList.at(i));
//    }
//    this->mpParentGraphicsView->setScene(pSubScene);
//}

int GUIComponent::type() const
{
    return Type;
}


//void GUIComponent::keyPressEvent( QKeyEvent *event )
//{
//    if (event->key() == Qt::Key_Delete)
//    {
//        //please delete me
//        mpParentGraphicsView->deleteComponent(this->getName());
//    }
//}




GUIComponent::GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
    : GUIObject(position, parameterData.at(1),scene, parent)
{
    mComponentTypeName = parameterData.at(0);
    //QString fileName = parameterData.at(1);
    QString iconRotationBehaviour = parameterData.at(2);
    if(iconRotationBehaviour == "ON")
        this->mIconRotation = true;
    else
        this->mIconRotation = false;
    size_t nPorts = parameterData.at(3).toInt();

    //Core interaction
    mpCoreComponent = hopsan->CreateComponent(mComponentTypeName.toStdString());
    //

    //Sets the ports
    //GUIPort::portType type;
    Port::PORTTYPE porttype;
    for (size_t i = 0; i < nPorts; ++i)
    {
        double x = parameterData.at(4+3*i).toDouble();
        double y = parameterData.at(5+3*i).toDouble();
        double rot = parameterData.at(6+3*i).toDouble();

        porttype = mpCoreComponent->getPortPtrVector().at(i)->getPortType();

        QString iconPath("../../HopsanGUI/porticons/");
        if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeSignal")
        {
            iconPath.append("SignalPort");
            if ( porttype == Port::READPORT)
            {
                iconPath.append("_read");
            }
            else
            {
                iconPath.append("_write");
            }
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeMechanic")
        {
            iconPath.append("MechanicPort");
            if (mpCoreComponent->getTypeCQS() == Component::C)
                iconPath.append("C");
            else if (mpCoreComponent->getTypeCQS() == Component::Q)
                iconPath.append("Q");
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeHydraulic")
        {
            iconPath.append("HydraulicPort");
            if (mpCoreComponent->getTypeCQS() == Component::C)
                iconPath.append("C");
            else if (mpCoreComponent->getTypeCQS() == Component::Q)
                iconPath.append("Q");
        }
        else
        {
            assert(false);
        }
        iconPath.append(".svg");

        GUIPort::portDirectionType direction;
        if((rot == 0) | (rot == 180))
            direction = GUIPort::HORIZONTAL;
        else
            direction = GUIPort::VERTICAL;
        mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(i), x*mpIcon->sceneBoundingRect().width(),y*mpIcon->sceneBoundingRect().height(),rot,iconPath,porttype,direction,this));//mpIcon));
    }

    refreshName(); //Make sure name window is correct size for center positioning

    std::cout << "GUIcomponent: " << mComponentTypeName.toStdString() << std::endl;

}


//! This function returns the current component name
QString GUIComponent::getName()
{
    return QString::fromStdString(mpCoreComponent->getName());
}

//!
//! @brief This function sets the desired component name
//! @param [in] newName The new name
//! @param [in] doOnlyCoreRename  Dont use this if you dont know what you are doing
//!
//! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
//! doOnlyCoreRename is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
//!
void GUIComponent::setName(QString newName, bool doOnlyCoreRename)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //This does not work when we load systems, the default name (oldNAme) may already be in the graphicsView and an incorrect rename will be triggered
//        if (mpParentGraphicsView->haveComponent(oldName))
//        {
//            //Rename
//            mpParentGraphicsView->renameComponent(oldName, newName);
//        }

        //Check if we want to avoid trying to rename in the graphics view map
        if (doOnlyCoreRename)
        {
            //Set name in core component,
            mpCoreComponent->setName(newName.toStdString());
            refreshName();
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
    }
}



//! Event when double clicking on component icon.
void GUIComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    std::cout << "GUIComponent.cpp: " << "mouseDoubleClickEvent " << std::endl;

    openParameterDialog();

}


//! Returns a string with the component type.
QString GUIComponent::getTypeName()
{
    return this->mComponentTypeName;
}

void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;
        if (!mpParentGraphicsScene->selectedItems().empty())
            groupAction = menu.addAction(tr("Group components"));

        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *showNameAction = menu.addAction(tr("Show name"));
        showNameAction->setCheckable(true);
        showNameAction->setChecked(this->mpNameText->isVisible());

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == parameterAction)
        {
            openParameterDialog();
        }
        else if (selectedAction == groupAction)
        {
            //groupComponents(mpParentGraphicsScene->selectedItems());
            GUIGroup *pGroup = new GUIGroup(mpParentGraphicsScene->selectedItems(), mpParentGraphicsScene);
            this->mpParentGraphicsScene->addItem(pGroup);
        }
        else if (selectedAction == showNameAction)
        {
            if(this->mpNameText->isVisible())
            {
                this->hideName();
            }
            else
            {
                this->showName();
            }
        }

}


void GUIComponent::openParameterDialog()
{
    vector<CompParameter>::iterator it;

    vector<CompParameter> paramVector = this->mpCoreComponent->getParameterVector();

    qDebug() << "This component has the following Parameters: ";
    for ( it=paramVector.begin() ; it !=paramVector.end(); it++ )
        qDebug() << QString::fromStdString(it->getName()) << ": " << it->getValue();

    ParameterDialog *dialog = new ParameterDialog(this,mpParentGraphicsView);
    dialog->exec();
}

Component* GUIComponent::getHopsanCoreComponentPtr()
{
    return mpCoreComponent;
}


void GUIComponent::deleteInHopsanCore()
{
    mpCoreComponent->getSystemParent()->removeSubComponent(mpCoreComponent, true);
}


GUISubsystem::GUISubsystem(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : GUIObject(position, parameterData.at(1), scene, parent)
{
    //Core interaction
    mpCoreComponentSystem = hopsan->CreateComponentSystem();
    //mpCoreComponentSystem->setName("unnamed");
    //

//    mComponentTypeName = parameterData.at(0);
//    //QString fileName = parameterData.at(1);
//    QString iconRotationBehaviour = parameterData.at(2);
//    if(iconRotationBehaviour == "ON")
//        this->mIconRotation = true;
//    else
//        this->mIconRotation = false;
//    size_t nPorts = parameterData.at(3).toInt();
//
//
//
//    //Sets the ports
//    //! @todo should break this out and make it reusable
//    //GUIPort::portType type;
//    Port::PORTTYPE porttype;
//    for (size_t i = 0; i < nPorts; ++i)
//    {
//        double x = parameterData.at(4+3*i).toDouble();
//        double y = parameterData.at(5+3*i).toDouble();
//        double rot = parameterData.at(6+3*i).toDouble();
//
//        porttype = mpCoreComponent->getPortPtrVector().at(i)->getPortType();
//
//        QString iconPath("../../HopsanGUI/porticons/");
//        if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeSignal")
//        {
//            iconPath.append("SignalPort");
//            if ( porttype == Port::READPORT)
//            {
//                iconPath.append("_read");
//            }
//            else
//            {
//                iconPath.append("_write");
//            }
//        }
//        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeMechanic")
//        {
//            iconPath.append("MechanicPort");
//            if (mpCoreComponent->getTypeCQS() == Component::C)
//                iconPath.append("C");
//            else if (mpCoreComponent->getTypeCQS() == Component::Q)
//                iconPath.append("Q");
//        }
//        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeHydraulic")
//        {
//            iconPath.append("HydraulicPort");
//            if (mpCoreComponent->getTypeCQS() == Component::C)
//                iconPath.append("C");
//            else if (mpCoreComponent->getTypeCQS() == Component::Q)
//                iconPath.append("Q");
//        }
//        else
//        {
//            assert(false);
//        }
//        iconPath.append(".svg");
//
//        GUIPort::portDirectionType direction;
//        if((rot == 0) | (rot == 180))
//            direction = GUIPort::HORIZONTAL;
//        else
//            direction = GUIPort::VERTICAL;
//        mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(i), x*mpIcon->sceneBoundingRect().width(),y*mpIcon->sceneBoundingRect().height(),rot,iconPath,porttype,direction,this));//mpIcon));
//    }

    refreshName(); //Make sure name window is correct size for center positioning

//    std::cout << "GUISubsystem: " << mComponentTypeName.toStdString() << std::endl;
}

//! This function returns the current subsystem name
QString GUISubsystem::getName()
{
    return QString::fromStdString(mpCoreComponentSystem->getName());
}

//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//! @param [in] doOnlyCoreRename  Dont use this if you dont know what you are doing
//!
//! @todo This function is almost exactly identical to the one for GUIcomponents need to make sure that we dont dublicate functions like this, maybe this should be directly in GUIObject
//!
//! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
//! doOnlyCoreRename is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
//!
void GUISubsystem::setName(QString newName, bool doOnlyCoreRename)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //This does not work when we load systems, the default name (oldNAme) may already be in the graphicsView and an incorrect rename will be triggered
//        if (mpParentGraphicsView->haveComponent(oldName))
//        {
//            //Rename
//            mpParentGraphicsView->renameComponent(oldName, newName);
//        }

        //Check if we want to avoid trying to rename in the graphics view map
        if (doOnlyCoreRename)
        {
            //Set name in core component,
            mpCoreComponentSystem->setName(newName.toStdString());
            refreshName();
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
    }
}


//! Returns a string with the sub system type.
QString GUISubsystem::getTypeName()
{
    //! @todo is this OK should really ask the subsystem but result should be subsystem i think
    return "Subsystem";
}


void GUISubsystem::deleteInHopsanCore()
{
    mpCoreComponentSystem->getSystemParent()->removeSubComponent(mpCoreComponentSystem, true);
}


ComponentSystem* GUISubsystem::getHopsanCoreSystemComponentPtr()
{
    return mpCoreComponentSystem;
}

GUISystemPort::GUISystemPort(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : GUIObject(position, parameterData.at(1), scene, parent)
{
    //Do something nice
}


//! Returns a string with the GUIObject type.
QString GUISystemPort::getTypeName()
{
    return "SystemPort";
}


int GUIGroup::type() const
{
    return Type;
}


GUIGroup::GUIGroup(QList<QGraphicsItem*> compList, GraphicsScene *scene, QGraphicsItem *parent)
    :   GUIObject(QPoint(0.0,0.0), QString("../../HopsanGUI/subsystemtmp.svg"), scene, parent)
{
    QList<GUIComponent*> GUICompList;
    QList<GUIConnector*> GUIConnList;

    this->setName(QString("ApGrupp"));
    this->refreshName();

    MessageWidget *pMessageWidget = scene->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;
    pMessageWidget->printGUIMessage("Group selected components (implementing in progress...) Selected components: ");
    for (int i=0; i < compList.size(); ++i)
    {
        GUIComponent *pComponent = qgraphicsitem_cast<GUIComponent*>(compList.at(i));
        if (pComponent)
        {
            GUICompList.append(pComponent);

            QMap<QString, GUIConnector *>::iterator it;
            for(it = this->mpParentGraphicsView->mConnectionMap.begin(); it!=this->mpParentGraphicsView->mConnectionMap.end(); ++it)
            {
                if(it.key().contains(pComponent->getName()))
                    if((compList.contains(it.value()->getStartPort()->getComponent())) && (compList.contains(it.value()->getEndPort()->getComponent())))
                        GUIConnList.append(it.value());

                if(this->mpParentGraphicsView->mConnectionMap.empty())
                    break;
            }
        }
    }

    double xMin = GUICompList.at(0)->x()+GUICompList.at(0)->rect().width()/2.0,
           xMax = GUICompList.at(0)->x()+GUICompList.at(0)->rect().width()/2.0,
           yMin = GUICompList.at(0)->y()+GUICompList.at(0)->rect().height()/2.0,
           yMax = GUICompList.at(0)->y()+GUICompList.at(0)->rect().height()/2.0;

    mpGroupScene = new GraphicsScene(this->mpParentGraphicsScene->mpParentProjectTab);
    for (int i=0; i < GUICompList.size(); ++i)
    {
        mpGroupScene->addItem(GUICompList.at(i));

        //Find the rect for the selscted items
        if (GUICompList.at(i)->x()+GUICompList.at(i)->rect().width()/2.0 < xMin)
            xMin = GUICompList.at(i)->x()+GUICompList.at(i)->rect().width()/2.0;
        if (GUICompList.at(i)->x()+GUICompList.at(i)->rect().width()/2.0 > xMax)
            xMax = GUICompList.at(i)->x()+GUICompList.at(i)->rect().width()/2.0;
        if (GUICompList.at(i)->y()+GUICompList.at(i)->rect().height()/2.0 < yMin)
            yMin = GUICompList.at(i)->y()+GUICompList.at(i)->rect().height()/2.0;
        if (GUICompList.at(i)->y()+GUICompList.at(i)->rect().height()/2.0 > yMax)
            yMax = GUICompList.at(i)->y()+GUICompList.at(i)->rect().height()/2.0;
    }
    for (int i=0; i < GUIConnList.size(); ++i)
    {
        mpGroupScene->addItem(GUIConnList.at(i));
    }

    //Fix the position for the group item
    this->setPos((xMax+xMin)/2.0-this->rect().width()/2.0,(yMax+yMin)/2.0-this->rect().height()/2.0);

    //this->mpParentGraphicsView->setScene(mpGroupScene);
}


//QGraphicsColorizeEffect *graphicsColor = new QGraphicsColorizeEffect;
//graphicsColor ->setColor(Qt::red);
//graphicsColor->setEnabled(true);
//this->mpIcon->setGraphicsEffect(graphicsColor);
