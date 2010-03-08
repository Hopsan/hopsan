//$Id$

#include <iostream>
#include <ostream>
#include <assert.h>
#include <vector>
#include <math.h>

#include <QDebug>
#include <QGraphicsWidget>
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

    mTextOffset = 1.1;

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
    mpNameText->setPos(QPointF(mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2, mTextOffset*mpIcon->boundingRect().height()));

    //Sets the ports
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
                iconPath.append("_read");
            else
                iconPath.append("_write");
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeMechanic")
        {
            iconPath.append("MechanicPort");
            if (mpCoreComponent->getTypeCQS() == "C")
                iconPath.append("C");
            else if (mpCoreComponent->getTypeCQS() == "Q")
                iconPath.append("Q");
        }
        else if (mpCoreComponent->getPortPtrVector().at(i)->getNodeType() == "NodeHydraulic")
        {
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

        mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(i), x*mpIcon->sceneBoundingRect().width(),y*mpIcon->sceneBoundingRect().height(),rot,iconPath,this));//mpIcon));

    }

    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    //connect(this->mpParentGraphicsView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));

    setPos(position-QPoint(mpIcon->boundingRect().width()/2, mpIcon->boundingRect().height()/2));

    mpSelectionBox = new GUIComponentSelectionBox(0,0,mpIcon->boundingRect().width(),mpIcon->boundingRect().height(),
                                                  QPen(QColor("red"),3), QPen(QColor("darkRed"),2),this);
    mpSelectionBox->setVisible(false);
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
    double x1 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
    double y1 = mTextOffset*mpIcon->boundingRect().height();

    double x2 = mpIcon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
    double y2 = -mTextOffset*mpNameText->boundingRect().height();

    double x = mpNameText->mapToParent(pos).x();
    double y = mpNameText->mapToParent(pos).y();

    if (dist(x,y, x1,y1) > dist(x,y, x2,y2))
    {
        mpNameText->setPos(x2,y2);
    }
    else
    {
        mpNameText->setPos(x1,y1);
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
void GUIComponent::setName(QString name)
{
    mpCoreComponent->setName(name.toStdString());
    refreshName();
}

GUIPort *GUIComponent::getPort(int number)
{
    return this->mPortListPtrs[number];
}

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


QString GUIComponent::getTypeName()
{
    return this->mComponentTypeName;
}


void GUIComponent::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setHovered();
        //this->mpSelectionBox->setVisible(true);
    }
    this->showPorts(true);
}


void GUIComponent::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!this->isSelected())
    {
        this->mpSelectionBox->setPassive();
    }
    this->showPorts(false);
}


void GUIComponent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    std::cout << "GUIComponent.cpp: " << "contextMenuEvent " << std::endl;

    vector<CompParameter>::iterator it;

    vector<CompParameter> paramVector = this->mpCoreComponent->getParameterVector();

    qDebug() << "This component has the following Parameters: ";
    for ( it=paramVector.begin() ; it !=paramVector.end(); it++ )
        qDebug() << QString::fromStdString(it->getName()) << ": " << it->getValue();

    ParameterDialog *dialog = new ParameterDialog(mpCoreComponent,mpParentGraphicsView);
    dialog->exec();
}

//void GUIComponent::keyPressEvent( QKeyEvent *event )
//{
//    if (event->key() == Qt::Key_Delete)
//    {
//        //please delete me
//        mpParentGraphicsView->deleteComponent(this->getName());
//    }
//}


QVariant GUIComponent::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        qDebug() << "Component selected status = " << this->isSelected();
        if (this->isSelected())
        {
            this->mpSelectionBox->setActive();
            connect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
        }
        else
        {
            disconnect(this->mpParentGraphicsView, SIGNAL(keyPressDelete()), this, SLOT(deleteMe()));
            this->mpSelectionBox->setPassive();
        }
    }
    else if (change == QGraphicsItem::ItemPositionChange)
    {
        emit componentMoved();
    }
    return value;
}


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
