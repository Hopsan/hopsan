//$Id$

#include <iostream>
#include <ostream>
#include <assert.h>
#include <vector>
#include <math.h>

#include <QDebug>
#include <QtGui>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsSceneMoveEvent>

#include "HopsanCore.h"
#include "GUIComponent.h"
#include "ParameterDialog.h"
#include "GUIPort.h"
#include "GUIConnector.h"

GUIComponent::GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentGraphicsScene = scene;
    mpParentGraphicsScene->addItem(this);
    mpParentGraphicsView = mpParentGraphicsScene->mpParentProjectTab->mpGraphicsView;

    mTextOffset = 5.0;

    mComponentTypeName = parameterData.at(0);
    QString fileName = parameterData.at(1);
    size_t nPorts = parameterData.at(2).toInt();

    //Core interaction
    mpCoreComponent = hopsan->CreateComponent(mComponentTypeName.toStdString());
    //

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    //setFocusPolicy(Qt::StrongFocus);
    this->setAcceptHoverEvents(true);

    this->setZValue(10);
    mpIcon = new QGraphicsSvgItem(fileName,this);

    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << "GUIcomponent: " << mComponentTypeName.toStdString() << std::endl;

    setGeometry(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height());

    mpNameText = new GUIComponentNameTextItem(this);
    refreshName(); //Make sure name window is correct size for center positioning
    //mpNameText->setPos(QPointF(mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2, mTextOffset*mpIcon->boundingRect().height()));
    mNameTextPos = 0;
    this->setNameTextPos(mNameTextPos);

    mpSelectionBox = new GUIComponentSelectionBox(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height(),
                                                  QPen(QColor("red"),2*1.6180339887499), QPen(QColor("darkRed"),2*1.6180339887499),this);
    mpSelectionBox->setVisible(false);

    //Sets the ports
    GUIPort::portType type;
    for (size_t i = 0; i < nPorts; ++i)
    {
        double x = parameterData.at(3+3*i).toDouble();
        double y = parameterData.at(4+3*i).toDouble();
        double rot = parameterData.at(5+3*i).toDouble();

        QString iconPath("../../HopsanGUI/porticons/");
        if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeSignal")
        {
            iconPath.append("SignalPort");
            if (mpCoreComponent->getPortPtrVector().at(i)->getPortType() == "ReadPort")
            {
                iconPath.append("_read");
                type = GUIPort::READ;
            }
            else
            {
                iconPath.append("_write");
                type = GUIPort::WRITE;
            }
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeMechanic")
        {
            type = GUIPort::POWER;
            iconPath.append("MechanicPort");
            if (mpCoreComponent->getTypeCQS() == "C")
                iconPath.append("C");
            else if (mpCoreComponent->getTypeCQS() == "Q")
                iconPath.append("Q");
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeHydraulic")
        {
            type = GUIPort::POWER;
            iconPath.append("HydraulicPort");
            if (mpCoreComponent->getTypeCQS() == "C")
                iconPath.append("C");
            else if (mpCoreComponent->getTypeCQS() == "Q")
                iconPath.append("Q");
        }
        else
        {
            assert(false);
        }
        iconPath.append(".svg");

        mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(i), x*mpIcon->sceneBoundingRect().width(),y*mpIcon->sceneBoundingRect().height(),rot,iconPath,type,this));//mpIcon));

    }

    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    //connect(this->mpParentGraphicsView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));

    //setPos(position-QPoint(mpIcon->boundingRect().width()/2, mpIcon->boundingRect().height()/2));
    setPos(position.x()-mpIcon->boundingRect().width()/2,position.y()-mpIcon->boundingRect().height()/2);
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


double dist(double x1,double y1, double x2, double y2)
{
    return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

void GUIComponent::fixTextPosition(QPointF pos)
{
    double x1,x2,y1,y2;

    if(this->rotation() == 0)
    {
        qDebug() << "Debug 1, rotation = " << this->rotation();
        x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
        x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
    }
    else if(this->rotation() == 180)
    {
        qDebug() << "Debug 2, rotation = " << this->rotation();
        x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
        x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y2 = -mTextOffset;
    }
    else if(this->rotation() == 90)
    {
        qDebug() << "Debug 3, rotation = " << this->rotation();
        x1 = -mpNameText->boundingRect().height() - mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        x2 = mpIcon->boundingRect().width() + mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
    }
    else if(this->rotation() == 270)
    {
        qDebug() << "Debug 4, rotation = " << this->rotation();
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


//void GUIComponent::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
//{
//    //qDebug() << "GUIComponent: " << "mouseReleaseEvent";
//    QGraphicsItem::mouseReleaseEvent(event);
//}


GUIComponent::~GUIComponent()
{
    //delete widget;
    emit componentDeleted();
}


//QGraphicsView *GUIComponent::getParentView()
//{
//    return mpParentView;
//}

void GUIComponent::addConnector(GUIConnector *item)
{
    connect(this, SIGNAL(componentMoved()), item, SLOT(updatePos()));
}

//! This function refreshes the displayed name (HopsanCore may have changed it)
void GUIComponent::refreshName()
{
    mpNameText->setPlainText(getName());
    //Adjust the position of the text
}

//! This function returns the current component name
QString GUIComponent::getName()
{
    return QString::fromStdString(mpCoreComponent->getName());
}

//! This function sets the desired component name
void GUIComponent::setName(QString newName)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //If the old name has not already been changed, let it decide our new name
        //Need this to prevent risk of loop between rename and this function (rename cals this one again)
        if (mpParentGraphicsView->haveComponent(oldName))
        {
            //Rename
            mpParentGraphicsView->renameComponent(oldName, newName);
        }
        else
        {
            //Set name for real
            mpCoreComponent->setName(newName.toStdString());
            refreshName();
        }
    }
}

//! Returns the port with the specified number.
//! @see getPortNumber(GUIPort *port)
GUIPort *GUIComponent::getPort(int number)
{
    return this->mPortListPtrs[number];
}


//! Tells the component to ask its parent to delete it.
void GUIComponent::deleteMe()
{
    qDebug() << "GUIComponent:: inside delete component";
//    if(this->isSelected())
//    {
//        emit componentDeleted();
//        this->scene()->removeItem(this);
//        delete(this);
//    }
    mpParentGraphicsView->deleteComponent(this->getName());
}


//! Returns a string with the component type.
QString GUIComponent::getTypeName()
{
    return this->mComponentTypeName;
}

//! Event when mouse cursor enters component icon.
void GUIComponent::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setHovered();
        //this->mpSelectionBox->setVisible(true);
    }
    this->showPorts(true);
}


//! Event when mouse cursor leaves component icon.
void GUIComponent::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setPassive();
    }
    this->showPorts(false);
}


void GUIComponent::openParameterDialog()
{
    vector<CompParameter>::iterator it;

    vector<CompParameter> paramVector = this->mpCoreComponent->getParameterVector();

    qDebug() << "This component has the following Parameters: ";
    for ( it=paramVector.begin() ; it !=paramVector.end(); it++ )
        qDebug() << QString::fromStdString(it->getName()) << ": " << it->getValue();

    ParameterDialog *dialog = new ParameterDialog(mpCoreComponent,mpParentGraphicsView);
    dialog->exec();
}


//! Event when double clicking on component icon.
void GUIComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    std::cout << "GUIComponent.cpp: " << "mouseDoubleClickEvent " << std::endl;

    openParameterDialog();

}



void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction = menu.addAction(tr("Group components"));
        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == parameterAction)
        {
            openParameterDialog();
        }
        else if (selectedAction == groupAction)
        {
            qDebug() << "Group selected components (to be implemented...)";
        }


}

//void GUIComponent::keyPressEvent( QKeyEvent *event )
//{
//    if (event->key() == Qt::Key_Delete)
//    {
//        //please delete me
//        mpParentGraphicsView->deleteComponent(this->getName());
//    }
//}


//! Handles item change events.
QVariant GUIComponent::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsWidget::itemChange(change, value);

    qDebug() << "Component selected status = " << this->isSelected();
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            this->mpSelectionBox->setActive();
            connect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            connect(this->mpParentGraphicsView, SIGNAL(keyPressR()), this, SLOT(rotate()));
        }
        else
        {
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressR()), this, SLOT(rotate()));
            this->mpSelectionBox->setPassive();
        }
    }
    else if (change == QGraphicsItem::ItemPositionChange)
    {
        emit componentMoved();
    }
    return value;
}


//! Shows or hides the port, depending on the input boolean and whether or not they are connected.
void GUIComponent::showPorts(bool visible)
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
int GUIComponent::getPortNumber(GUIPort *port)
{
    for (int i = 0; i != mPortListPtrs.size(); ++i)
    {
        qDebug() << "Checking port " << i;
        if(port == mPortListPtrs.value(i))
        {
            qDebug() << "Match!";
            return i;
        }
    }
    assert(false);      //Todo: Cast exception
}


//! Rotates a component 90 degrees clockwise, and tells the connectors that the component has moved.
void GUIComponent::rotate()
{
    int temNameTextPos = mNameTextPos;
    this->setTransformOriginPoint(this->boundingRect().center());
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
        if (mPortListPtrs.value(i)->getPortType() == GUIPort::POWER)
            mPortListPtrs.value(i)->setRotation(-this->rotation());
    }

    emit componentMoved();
}


//! Returns an integer that describes the position of the component name text.
//! @see setNameTextPos(int textPos)
//! @see fixTextPosition(QPointF pos)
int GUIComponent::getNameTextPos()
{
    return mNameTextPos;
}\


//! Updates the name text position, and moves the text to the correct position.
//! @see getNameTextPos()
//! @see fixTextPosition(QPointF pos)
void GUIComponent::setNameTextPos(int textPos)
{
    mNameTextPos = textPos;

    double x1,x2,y1,y2;

    if(this->rotation() == 0)
    {
        qDebug() << "Debug 1, rotation = " << this->rotation();
        x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y1 = -mpNameText->boundingRect().height() - mTextOffset;  //mTextOffset*
        x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
        y2 = mpIcon->boundingRect().height() + mTextOffset;// - mpNameText->boundingRect().height()/2;
    }
    else if(this->rotation() == 180)
    {
        qDebug() << "Debug 2, rotation = " << this->rotation();
        x1 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y1 = mpIcon->boundingRect().height() + mpNameText->boundingRect().height() + mTextOffset;
        x2 = mpIcon->boundingRect().width()/2+mpNameText->boundingRect().width()/2;
        y2 = -mTextOffset;
    }
    else if(this->rotation() == 90)
    {
        qDebug() << "Debug 3, rotation = " << this->rotation();
        x1 = -mpNameText->boundingRect().height() - mTextOffset;
        y1 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
        x2 = mpIcon->boundingRect().width() + mTextOffset;
        y2 = mpIcon->boundingRect().height()/2 + mpNameText->boundingRect().width()/2;
    }
    else if(this->rotation() == 270)
    {
        qDebug() << "Debug 4, rotation = " << this->rotation();
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

GUIComponentNameTextItem::GUIComponentNameTextItem(GUIComponent *pParent)
    :   QGraphicsTextItem(pParent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    //setTextInteractionFlags(Qt::TextEditorInteraction);
    mpParentGUIComponent = pParent;
}


void GUIComponentNameTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
}


void GUIComponentNameTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "GUIComponentTextItem: " << "mouseReleaseEvent";
    emit textMoved(event->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}

void GUIComponentNameTextItem::focusOutEvent(QFocusEvent *event)
{
    qDebug() << "GUIComponentTextItem: " << "focusOutEvent";
    //Try to set the new name
    mpParentGUIComponent->setName(toPlainText());
    //refresh the display name (it may be different from the one you wanted)
    mpParentGUIComponent->refreshName();
    emit textMoved(pos());

    setTextInteractionFlags(Qt::NoTextInteraction);

    QGraphicsTextItem::focusOutEvent(event);
}
