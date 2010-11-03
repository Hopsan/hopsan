//$Id: GUIObject.cpp 2032 2010-10-27 15:50:41Z bjoer $

//! @todo clean up this include and forward declaration mess
#include "GUIGroup.h"

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "common.h"

#include "GUIObject.h"
#include "GUISystem.h"
#include "GUIComponent.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "ParameterDialog.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "GUIUtilities.h"
#include "UndoStack.h"
#include "MessageWidget.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "LibraryWidget.h"
#include "loadObjects.h"

using namespace std;

//! @class GUIGroup
//! @brief The GUIGroup class implement a class to group components graphically
//!
//! The grouping is an alternative to make subcomponents. A group is a graphical operation ONLY, no changes in the core.
//!


int GUIGroup::type() const
{
    return Type;
}


QString GUIGroup::getTypeName()
{
    return "";
}


//! Constructor.
//! @param compList is a list for the QGraphicsItems that should be in the group.
//! @param appearanceData defines the appearance for the group.
//! @param scene is the scene which should contain the group.
//! @param parent is the parent QGraphicsItem for the group, default = 0.
GUIGroup::GUIGroup(QList<QGraphicsItem*> compList, AppearanceData* pAppearanceData, GUISystem *system, QGraphicsItem *parent)
    :   GUIContainerObject(QPoint(0.0,0.0), 0, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)
{
    mpParentScene = system->mpScene;

    //Set the hmf save tag name
    mHmfTagName = HMF_GROUPTAG;

    this->setDisplayName(QString("Grupp_test"));

    MessageWidget *pMessageWidget = mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget;
    pMessageWidget->printGUIMessage("Group selected components (implementing in progress...) Selected components: ");

    for (int i=0; i < compList.size(); ++i)
    {
        GUIComponent *pComponent = qgraphicsitem_cast<GUIComponent*>(compList.at(i));
        if (pComponent)
        {
            //Adds the component pComponent to a list of components whose make up the group
            mGUICompList.append(pComponent);
            pMessageWidget->printGUIMessage(pComponent->getName());

            QList<GUIConnector*> GUIConnectorPtrs = pComponent->getGUIConnectorPtrs();
            for(int i = 0; i != GUIConnectorPtrs.size(); ++i)
            {
                //Loop trough the GUIConnectors that are connected to pComponent
                if((GUIConnectorPtrs[i]->getStartComponentName() == pComponent->getName()) ||
                   (GUIConnectorPtrs[i]->getEndComponentName() == pComponent->getName()))
                {
                    if((compList.contains(GUIConnectorPtrs[i]->getStartPort()->getGuiModelObject())) &&
                       (compList.contains(GUIConnectorPtrs[i]->getEndPort()->getGuiModelObject())))
                    {
                        //Add the connections which have both ends among selected components
                        //for grouping in a list for connections
                        mGUIConnList.append(GUIConnectorPtrs[i]);
                    }
                    else
                    {
                        //Add the connections that go trough the group boundary to a list
                        mGUITransitConnList.append(GUIConnectorPtrs[i]);
                    }
                }
                if(GUIConnectorPtrs.empty())
                {
                    break;
                }
            }
        }
    }

    //Constructs a new scene for the group
    mpGroupScene = new GraphicsScene(mpParentSystem->mpParentProjectTab);

    double xMin = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
           xMax = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
           yMin = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0,
           yMax = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0;
    for (int i=0; i < mGUICompList.size(); ++i)
    {
        //Add the components in the group to the group scene
        mpGroupScene->addItem(mGUICompList.at(i));

        //Find the rect for the selscted items (the group)
        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 < xMin)
            xMin = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 > xMax)
            xMax = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 < yMin)
            yMin = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 > yMax)
            yMax = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
    }
    //Fix the position for the group item
    this->setPos((xMax+xMin)/2.0-this->rect().width()/2.0,(yMax+yMin)/2.0-this->rect().height()/2.0);

    for (int i=0; i < mGUIConnList.size(); ++i)
    {
        //Add the connections in the group to the group scene
        mpGroupScene->addItem(mGUIConnList.at(i));
    }

    mpGroupScene->setSceneRect(0,0,0,0); //Dirty(?) fix to re-calculate the correct scenerect
    QPointF sceneCenterPointF = mpGroupScene->sceneRect().center();

    //Draw a cross in the center of the scene (just for debugging)
//    mpGroupScene->addLine(-10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), 10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
//    mpGroupScene->addLine(10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), -10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
//    qDebug() << "Center: " << sceneCenterPointF << mpGroupScene->sceneRect();

    //Adjusts the size of the group component icon
    double scale = 1.0;//.75*min(mpGroupScene->sceneRect().width()/this->boundingRect().width(),mpGroupScene->sceneRect().height()/this->boundingRect().height());
    this->setTransformOriginPoint(this->boundingRect().center());
    this->setScale(scale);

    //Take care of the boundary connections of the group
    for(int i=0; i < mGUITransitConnList.size(); ++i)
    {
        GUIConnector *pTransitConnector = mGUITransitConnList[i];

        //Get the right appearance data for the group port
        AppearanceData appData;
        appData = *(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary->getAppearanceData("SystemPort"));
        appData.setName("aPaApA-port");

        GUIGroupPort *pGroupPortComponent;

        GUIComponent *startComp;
        GUIComponent *endComp;
        startComp = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getStartPort()->getGuiModelObject());
        endComp   = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getEndPort()->getGuiModelObject());

        QPoint groupPortPoint;
        GUIPort *pPortBoundaryInside; //Inside the group
        GUIPort *pPortBoundaryOutside; //Outside the group
        if((startComp) && (mGUICompList.contains(startComp)))
        {
            //Find the right point for the group boundary port (in this case the boundary is at the connector start point)
            pPortBoundaryInside = pTransitConnector->getStartPort();
            pPortBoundaryOutside = pTransitConnector->getEndPort();
        }
        if((endComp) && (mGUICompList.contains(endComp)))
        {
            //Find the right point for the group boundary port (in this case the boundary is at the connector end point)
            pPortBoundaryInside = pTransitConnector->getEndPort();
            pPortBoundaryOutside = pTransitConnector->getStartPort();
        }
        groupPortPoint = getOffsetPointfromPort(pPortBoundaryInside).toPoint();
        groupPortPoint += QPoint(2.0*groupPortPoint.x(), 2.0*groupPortPoint.y());
        groupPortPoint += pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()).toPoint();

        //Add a new group port for the boundary at the boundary connector
        pGroupPortComponent = new GUIGroupPort(&appData, groupPortPoint, mpParentSystem);
        GUIPort *pPort = pGroupPortComponent->getPort("sysp");
        QString portName;
        if(pPort)
        {
            pGroupPortComponent->setOuterGuiPort(pPortBoundaryOutside);
            portName = pTransitConnector->getStartPortName();

            QVector<QPointF> points;
            points.append(pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()));
            points.append(pPort->mapToScene(pPort->boundingRect().center())); //! @todo GUIConnector should handle any number of points e.g. 0, 1 or 2
            points.append(pPort->mapToScene(pPort->boundingRect().center()));
            GUIConnector *pInsideConnector = new GUIConnector(pPortBoundaryInside, pPort, points, mpParentSystem);
            mpGroupScene->addItem(pInsideConnector);

//            pGroupPortComponent->addConnector(pInsideConnector);
            mpGroupScene->addItem(pGroupPortComponent);
            pGroupPortComponent->showPorts(false);

        }

        //A line from center to port, used to determine the angle
        QLineF line(QPointF(sceneCenterPointF.x(), sceneCenterPointF.y()), QPointF(groupPortPoint.x(), groupPortPoint.y()));
//        mpGroupScene->addLine(line); //(just for debugging)
        //Determine the placement of the ports on the group icon
        double vinkel=line.angle()*3.141592/180.0;
        double b = mpIcon->boundingRect().width()/2.0;
        double h = mpIcon->boundingRect().height()/2.0;
        double x, y;
        calcSubsystemPortPosition(b, h, vinkel, x, y);

        qDebug() << portName << " vinkel: " << tan(vinkel) << " x: " << x << " ber x: " << h/tan(vinkel) << " b: " << b << " y: " << y << " ber y: " << b*tan(vinkel) << " h: " << h;
        //Make ports on the group system icon
        GUIPortAppearance portAppearance;
        portAppearance.selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)
        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        GUIPort *pGuiPort = new GUIPort(pPortBoundaryInside->getGUIComponentName().append(", ").append(portName),
                                        mpIcon->boundingRect().center().x()+x,
                                        mpIcon->boundingRect().center().y()-y,
                                        &(portAppearance),
                                        this);
        mPortListPtrs.append(pGuiPort);

        //Make connectors to the group component
        GUIConnector *tmpConnector = new GUIConnector(pGuiPort, pPortBoundaryOutside,pTransitConnector->getPointsVector(), mpParentSystem);
        mpParentScene->addItem(tmpConnector);
        this->showPorts(false);
        tmpConnector->drawConnector();

        //Delete the old connector
        delete pTransitConnector;
    }

    //Show this scene
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpGroupScene);
    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    //Draw a cross in the center of the group component icon (debug)
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,-10), this->rect().center()-QPointF(10,10)),this);
//    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,10), this->rect().center()-QPointF(10,-10)),this);

    //Scale up the ports and so on
    //! @todo Add a method to mpSelectionBox so the lines could be scaled
    QList<GUIPort*>::iterator it;
    for (it = mPortListPtrs.begin(); it != mPortListPtrs.end(); ++it)
    {
        (*it)->setTransformOriginPoint((*it)->boundingRect().center());
        (*it)->setScale(1.0/scale);
    }

    connect(this->mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}


GUIGroup::~GUIGroup()
{
    qDebug() << "GUIGroup destructor";
    GUISystem::GUIModelObjectMapT::iterator itm;
    for(itm = mpParentSystem->mGUIModelObjectMap.begin(); itm != mpParentSystem->mGUIModelObjectMap.end(); ++itm)
    {
        qDebug() << "GUIObjectMap: " << itm.key();
    }


    QList<QGraphicsItem*> objectsInScenePtrs = mpGroupScene->items();
    QList<QGraphicsItem*>::iterator it;
    for(it=objectsInScenePtrs.begin(); it != objectsInScenePtrs.end(); ++it)
    {
        //! @todo Will cause crash when closing program if the GUIObject has already been deleted by the scene.
        mpParentSystem->deleteGUIObject(this->getName());
        GUIComponent *pGUIComponent = qgraphicsitem_cast<GUIComponent*>(*it);
        mpGroupScene->removeItem((*it));

        if(pGUIComponent)
        {
            qDebug() << "Add this to parent scene: " << pGUIComponent->getName();
            mpParentScene->addItem(pGUIComponent);
        }
        //mpParentScene->addItem((*it));
    }
    qDebug() << "mpParentSystem->deleteGUIObject(this->getName()), getName:" << this->getName();
    //delete mpGroupScene;
}


//! Shows the parent scene. Should be called to exit a group.
void GUIGroup::showParent()
{
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpParentScene);

    disconnect(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->hide();

}


void GUIGroup::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;

        groupAction = menu.addAction(tr("Un-group components"));

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction == groupAction)
        {
            delete this;
        }
}


//! A slot that makes an entrance into a group at double clicks.
//! @param event contain information of the doubleclick event.
void GUIGroup::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    mpParentSystem->mpParentProjectTab->mpGraphicsView->setScene(mpGroupScene);

    mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton->show();

    connect(mpParentSystem->mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}

//QGraphicsColorizeEffect *graphicsColor = new QGraphicsColorizeEffect;
//graphicsColor ->setColor(Qt::red);
//graphicsColor->setEnabled(true);
//mpIcon->setGraphicsEffect(graphicsColor);


GUIGroupPort::GUIGroupPort(AppearanceData* pAppearanceData, QPoint position, GUISystem *system, QGraphicsItem *parent)
    : GUIModelObject(position, 0, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)

{
    //Sets the ports
    //! @todo Only one port in group ports could simplify this
    PortAppearanceMapT::iterator i;
    for (i = mAppearanceData.getPortAppearanceMap().begin(); i != mAppearanceData.getPortAppearanceMap().end(); ++i)
    {
        qDebug() << "DEBUG: " << i.key();
        qreal x = i.value().x;
        qreal y = i.value().y;

        i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

//        mAppearanceData.setName(mpParentSystem->mpParentProjectTab->mGUIRootSystem.addSystemPort(i.key()));
        mAppearanceData.setName(i.key());

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mAppearanceData.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
        mpOuterGuiPort = 0;
        mPortListPtrs.append(mpGuiPort);
    }
}


void GUIGroupPort::setOuterGuiPort(GUIPort *pPort)
{
    mpOuterGuiPort = pPort;
}


//! Returns a string with the GUIObject type.
QString GUIGroupPort::getTypeName()
{
    return "GroupPort";
}

//! Set the name of a group port
void GUIGroupPort::setName(QString newName)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        //Rename
        mpParentSystem->renameGUIObject(oldName, newName);
    }
}


int GUIGroupPort::type() const
{
    return Type;
}
