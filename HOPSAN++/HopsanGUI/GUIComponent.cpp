//$Id$

#include "HopsanCore.h"
#include "GUIComponent.h"
#include <iostream>
#include "GUIPort.h"
#include "GUIConnector.h"
#include <ostream>
#include <assert.h>
#include <QDebug>

#include <QGraphicsWidget>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QWidget>
#include <QGraphicsView>
#include <vector>
#include <QGraphicsItem>

#include <QGraphicsSceneMoveEvent>

#include <math.h>
#include "ParameterDialog.h"

GUIComponent::GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent)
        : QGraphicsWidget(parent)
{
    mpParentGraphicsScene = scene;
    mpParentGraphicsScene->addItem(this);
    mpParentGraphicsView = mpParentGraphicsScene->mpParentProjectTab->mpGraphicsView;

    QString componentTypeName = parameterData.at(0);
    QString fileName = parameterData.at(1);
    size_t nPorts = parameterData.at(2).toInt();

    //Core interaction
    mpCoreComponent = hopsan->CreateComponent(componentTypeName.toStdString());
    //

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemUsesExtendedStyleOption);
    this->setAcceptHoverEvents(true);

    this->setZValue(10);
    icon = new QGraphicsSvgItem(fileName,this);

    std::cout << "GUIcomponent: " << "x=" << this->pos().x() << "  " << "y=" << this->pos().y() << std::endl;
    std::cout << "GUIcomponent: " << componentTypeName.toStdString() << std::endl;

    setGeometry(0,0,icon->boundingRect().width(),icon->boundingRect().height());

    mpNameText = new GUIComponentNameTextItem(mpCoreComponent, this);
    mpNameText->setPos(QPointF(icon->boundingRect().width()/2-mpNameText->boundingRect().width()/2, icon->boundingRect().height()));

    //Sets the ports
    for (size_t i = 0; i < nPorts; ++i)
    {
        double x = parameterData.at(3+2*i).toDouble();
        double y = parameterData.at(4+2*i).toDouble();
        mPortListPtrs.append(new GUIPort(mpCoreComponent->getPortPtrVector().at(i), x*icon->sceneBoundingRect().width()-5,y*icon->sceneBoundingRect().height()-5,10.0,10.0,mpParentGraphicsView,this,icon));
    }

    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(fixTextPosition(QPointF)));
    connect(this->mpParentGraphicsView,SIGNAL(keyPressDelete()),this,SLOT(deleteComponent()));

    setPos(position-QPoint(icon->boundingRect().width()/2, icon->boundingRect().height()/2));

    mpSelectionBox = new GUIComponentSelectionBox(0,0,icon->boundingRect().width(),icon->boundingRect().height(),
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
    double x1 = icon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
    double y1 = icon->boundingRect().height();

    double x2 = icon->boundingRect().width()/2-mpNameText->boundingRect().width()/2;
    double y2 = -mpNameText->boundingRect().height();

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


void GUIComponent::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    qDebug() << "GUIComponent: " << "mouseReleaseEvent";
    QGraphicsItem::mouseReleaseEvent(event);
}


GUIComponent::~GUIComponent()
{
    //delete widget;
}


//QGraphicsView *GUIComponent::getParentView()
//{
//    return mpParentView;
//}

void GUIComponent::addConnector(GUIConnector *item)
{
    connect(this,SIGNAL(componentMoved()),item,SLOT(updatePos()));
}

void GUIComponent::refreshName()
{
    mpNameText->refreshName();
}

GUIPort *GUIComponent::getPort(int number)
{
    return this->mPortListPtrs[number];
}

void GUIComponent::deleteComponent()
{
    qDebug() << "GUIComponent:: inside delete component\n";
    if(this->isSelected())
    {
        emit componentDeleted();
        this->scene()->removeItem(this);
        delete(this);
    }
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


QVariant GUIComponent::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        qDebug() << "Component selected status = " << this->isSelected();
        if (!this->isSelected())
        {
            this->mpSelectionBox->setActive();
        }
        else
        {
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
            (*i)->hide();
        }
}



GUIComponentNameTextItem::GUIComponentNameTextItem(Component* pCoreComponent, QGraphicsItem *parent)
    :   QGraphicsTextItem(QString::fromStdString(pCoreComponent->getName()), parent)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setTextInteractionFlags(Qt::TextEditorInteraction);
    mpCoreComponent = pCoreComponent;
}

void GUIComponentNameTextItem::refreshName()
{
    setPlainText(QString::fromStdString(mpCoreComponent->getName()));
    //Adjust the position of the text
    emit textMoved(pos());
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
    mpCoreComponent->setName(toPlainText().toStdString());
    //refresh the display name (it may be different from the one you wanted)
    refreshName();

    QGraphicsTextItem::focusOutEvent(event);
}

//void GUIComponentTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    qDebug() << "GUIComponentTextItem: " << "mousePressEvent";
//    //QGraphicsItem::setFocus ();
//    //QGraphicsItem::grabKeyboard ();
//    QGraphicsTextItem::mousePressEvent(event);
//}
