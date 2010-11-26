//$Id: GUIObject.cpp 2032 2010-10-27 15:50:41Z bjoer $

//! @todo clean up this include and forward declaration mess
#include "GUIGroup.h"

#include <QVector>
#include <QtGui> //!< @todo maybe only need qtfile dialog

#include "common.h"

//#include "GUIObject.h"
#include "GUISystem.h"
#include "GUIContainerObject.h"
#include "GUIComponent.h"
#include "../ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../Dialogs/ComponentPropertiesDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../MessageWidget.h"
#include "../GraphicsView.h"
#include "../LibraryWidget.h"
#include "../loadObjects.h"
#include "../CopyStack.h"

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

//! @brief Returns the core access ptr in the parent system, groups are GUI only objects
CoreSystemAccess* GUIGroup::getCoreSystemAccessPtr()
{
    return this->mpParentContainerObject->getCoreSystemAccessPtr();
}

//! Constructor.
//! @param compList is a list for the QGraphicsItems that should be in the group.
//! @param appearanceData defines the appearance for the group.
//! @param scene is the scene which should contain the group.
//! @param parent is the parent QGraphicsItem for the group, default = 0.
GUIGroup::GUIGroup(QPoint position, qreal rotation, const GUIModelObjectAppearance *pAppearanceData, GUIContainerObject *system, QGraphicsItem *parent)
    :   GUIContainerObject(position, rotation, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)
{
    //Set the hmf save tag name
    mHmfTagName = HMF_GROUPTAG;

    this->setDisplayName(QString("Grupp_test"));

    //Add this item to the parent container scene, no we do this outside
    //this->mpParentContainerObject->getContainedScenePtr()->addItem(this);

    //! @todo this is not good all mpParentProjectTab should be set in one common place not in guigroup and guisystem
    this->mpParentProjectTab = system->mpParentProjectTab;
}


GUIGroup::~GUIGroup()
{
//    qDebug() << "GUIGroup destructor";
//    GUISystem::GUIModelObjectMapT::iterator itm;
//    for(itm = mpParentContainerObject->mGUIModelObjectMap.begin(); itm != mpParentContainerObject->mGUIModelObjectMap.end(); ++itm)
//    {
//        qDebug() << "GUIObjectMap: " << itm.key();
//    }


//    QList<QGraphicsItem*> objectsInScenePtrs = getContainedScenePtr()->items();
//    QList<QGraphicsItem*>::iterator it;
//    for(it=objectsInScenePtrs.begin(); it != objectsInScenePtrs.end(); ++it)
//    {
//        //! @todo Will cause crash when closing program if the GUIObject has already been deleted by the scene.
//        mpParentContainerObject->deleteGUIModelObject(this->getName());
//        GUIComponent *pGUIComponent = qgraphicsitem_cast<GUIComponent*>(*it);
//        getContainedScenePtr()->removeItem((*it));

//        if(pGUIComponent)
//        {
//            qDebug() << "Add this to parent scene: " << pGUIComponent->getName();
//            //mpParentScene->addItem(pGUIComponent);
//            this->mpParentContainerObject->getContainedScenePtr()->addItem(pGUIComponent);
//        }
//        //mpParentScene->addItem((*it));
//    }
//    qDebug() << "mpParentSystem->deleteGUIModelObject(this->getName()), getName:" << this->getName();
//    //delete getContainedScenePtr();
}

//! @todo Add group contents from a copy/paste object
void GUIGroup::setContents(CopyStack *pCopyStack)
{
    MessageWidget *pMessageWidget = gpMainWindow->mpMessageWidget;
    //QDomElement *copyRoot = pCopyStack->getCopyRoot();

    this->paste(pCopyStack);

//    pMessageWidget->printGUIMessage("Group selected components (implementing in progress...) Selected components: ");

//    for (int i=0; i < compList.size(); ++i)
//    {
//        GUIModelObject *pComponent = qgraphicsitem_cast<GUIComponent*>(compList.at(i));
//        if (pComponent)
//        {
//            //Adds the component pComponent to a list of components whose make up the group
//            mGUICompList.append(pComponent);
//            pMessageWidget->printGUIMessage(pComponent->getName());

//            QList<GUIConnector*> GUIConnectorPtrs = pComponent->getGUIConnectorPtrs();
//            for(int i = 0; i != GUIConnectorPtrs.size(); ++i)
//            {
//                //Loop trough the GUIConnectors that are connected to pComponent
//                if((GUIConnectorPtrs[i]->getStartComponentName() == pComponent->getName()) ||
//                   (GUIConnectorPtrs[i]->getEndComponentName() == pComponent->getName()))
//                {
//                    if((compList.contains(GUIConnectorPtrs[i]->getStartPort()->getGuiModelObject())) &&
//                       (compList.contains(GUIConnectorPtrs[i]->getEndPort()->getGuiModelObject())))
//                    {
//                        //Add the connections which have both ends among selected components
//                        //for grouping in a list for connections
//                        mGUIConnList.append(GUIConnectorPtrs[i]);
//                    }
//                    else
//                    {
//                        //Add the connections that go trough the group boundary to a list
//                        mGUITransitConnList.append(GUIConnectorPtrs[i]);
//                    }
//                }
//                if(GUIConnectorPtrs.empty())
//                {
//                    break;
//                }
//            }
//        }
//    }

//    //Constructs a new scene for the group
//    //getContainedScenePtr() = new QGraphicsScene(mpParentContainerObject->mpParentProjectTab);

//    double xMin = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
//           xMax = mGUICompList.at(0)->x()+mGUICompList.at(0)->rect().width()/2.0,
//           yMin = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0,
//           yMax = mGUICompList.at(0)->y()+mGUICompList.at(0)->rect().height()/2.0;
//    for (int i=0; i < mGUICompList.size(); ++i)
//    {
//        //Add the components in the group to the group scene
//        getContainedScenePtr()->addItem(mGUICompList.at(i));

//        //Find the rect for the selscted items (the group)
//        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 < xMin)
//            xMin = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
//        if (mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0 > xMax)
//            xMax = mGUICompList.at(i)->x()+mGUICompList.at(i)->rect().width()/2.0;
//        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 < yMin)
//            yMin = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
//        if (mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0 > yMax)
//            yMax = mGUICompList.at(i)->y()+mGUICompList.at(i)->rect().height()/2.0;
//    }
//    //Fix the position for the group item
//    this->setPos((xMax+xMin)/2.0-this->rect().width()/2.0,(yMax+yMin)/2.0-this->rect().height()/2.0);

//    for (int i=0; i < mGUIConnList.size(); ++i)
//    {
//        //Add the connections in the group to the group scene
//        getContainedScenePtr()->addItem(mGUIConnList.at(i));
//    }

//    getContainedScenePtr()->setSceneRect(0,0,0,0); //Dirty(?) fix to re-calculate the correct scenerect
//    QPointF sceneCenterPointF = getContainedScenePtr()->sceneRect().center();

//    //Draw a cross in the center of the scene (just for debugging)
////    getContainedScenePtr()->addLine(-10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), 10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
////    getContainedScenePtr()->addLine(10+sceneCenterPointF.x(), -10+sceneCenterPointF.y(), -10+sceneCenterPointF.x(), 10+sceneCenterPointF.y());
////    qDebug() << "Center: " << sceneCenterPointF << getContainedScenePtr()->sceneRect();

//    //Adjusts the size of the group component icon
//    double scale = 1.0;//.75*min(getContainedScenePtr()->sceneRect().width()/this->boundingRect().width(),getContainedScenePtr()->sceneRect().height()/this->boundingRect().height());
//    this->setTransformOriginPoint(this->boundingRect().center());
//    this->setScale(scale);

//    //Take care of the boundary connections of the group
//    for(int i=0; i < mGUITransitConnList.size(); ++i)
//    {
//        GUIConnector *pTransitConnector = mGUITransitConnList[i];

//        //Get the right appearance data for the group port
//        GUIModelObjectAppearance appData;
//        appData = *(gpMainWindow->mpLibrary->getAppearanceData("SystemPort"));
//        appData.setName("aPaApA-port");

//        GUIGroupPort *pGroupPortComponent;

//        GUIModelObject *startComp;
//        GUIModelObject *endComp;
//        startComp = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getStartPort()->getGuiModelObject());
//        endComp   = qgraphicsitem_cast<GUIComponent*>(pTransitConnector->getEndPort()->getGuiModelObject());

//        QPoint groupPortPoint;
//        GUIPort *pPortBoundaryInside; //Inside the group
//        GUIPort *pPortBoundaryOutside; //Outside the group
//        if((startComp) && (mGUICompList.contains(startComp)))
//        {
//            //Find the right point for the group boundary port (in this case the boundary is at the connector start point)
//            pPortBoundaryInside = pTransitConnector->getStartPort();
//            pPortBoundaryOutside = pTransitConnector->getEndPort();
//        }
//        if((endComp) && (mGUICompList.contains(endComp)))
//        {
//            //Find the right point for the group boundary port (in this case the boundary is at the connector end point)
//            pPortBoundaryInside = pTransitConnector->getEndPort();
//            pPortBoundaryOutside = pTransitConnector->getStartPort();
//        }
//        groupPortPoint = getOffsetPointfromPort(pPortBoundaryInside).toPoint();
//        groupPortPoint += QPoint(2.0*groupPortPoint.x(), 2.0*groupPortPoint.y());
//        groupPortPoint += pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()).toPoint();

//        //Add a new group port for the boundary at the boundary connector
//        pGroupPortComponent = new GUIGroupPort(&appData, groupPortPoint, mpParentContainerObject);
//        GUIPort *pPort = pGroupPortComponent->getPort("sysp");
//        QString portName;
//        if(pPort)
//        {
//            pGroupPortComponent->setOuterGuiPort(pPortBoundaryOutside);
//            portName = pTransitConnector->getStartPortName();

//            QVector<QPointF> points;
//            points.append(pPortBoundaryInside->mapToScene(pPortBoundaryInside->boundingRect().center()));
//            points.append(pPort->mapToScene(pPort->boundingRect().center())); //! @todo GUIConnector should handle any number of points e.g. 0, 1 or 2
//            points.append(pPort->mapToScene(pPort->boundingRect().center()));
//            GUIConnector *pInsideConnector = new GUIConnector(pPortBoundaryInside, pPort, points, mpParentContainerObject);
//            getContainedScenePtr()->addItem(pInsideConnector);

////            pGroupPortComponent->addConnector(pInsideConnector);
//            getContainedScenePtr()->addItem(pGroupPortComponent);
//            pGroupPortComponent->showPorts(false);

//        }

//        //A line from center to port, used to determine the angle
//        QLineF line(QPointF(sceneCenterPointF.x(), sceneCenterPointF.y()), QPointF(groupPortPoint.x(), groupPortPoint.y()));
////        getContainedScenePtr()->addLine(line); //(just for debugging)
//        //Determine the placement of the ports on the group icon
//        double vinkel=line.angle()*3.141592/180.0;
//        double b = mpIcon->boundingRect().width()/2.0;
//        double h = mpIcon->boundingRect().height()/2.0;
//        double x, y;
//        calcSubsystemPortPosition(b, h, vinkel, x, y);

//        qDebug() << portName << " vinkel: " << tan(vinkel) << " x: " << x << " ber x: " << h/tan(vinkel) << " b: " << b << " y: " << y << " ber y: " << b*tan(vinkel) << " h: " << h;
//        //Make ports on the group system icon
//        GUIPortAppearance portAppearance;
//        portAppearance.selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)
//        //We supply ptr to rootsystem to indicate that this is a systemport
//        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
//        GUIPort *pGuiPort = new GUIPort(pPortBoundaryInside->getGuiModelObjectName().append(", ").append(portName),
//                                        mpIcon->boundingRect().center().x()+x,
//                                        mpIcon->boundingRect().center().y()-y,
//                                        &(portAppearance),
//                                        this);
//        mPortListPtrs.append(pGuiPort);

//        //Make connectors to the group component
//        GUIConnector *tmpConnector = new GUIConnector(pGuiPort, pPortBoundaryOutside,pTransitConnector->getPointsVector(), mpParentContainerObject);
//        //mpParentScene->addItem(tmpConnector);
//        this->mpParentContainerObject->getContainedScenePtr()->addItem(tmpConnector);
//        this->showPorts(false);
//        tmpConnector->drawConnector();

//        //Delete the old connector
//        delete pTransitConnector;
//    }



//    //Draw a cross in the center of the group component icon (debug)
////    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,-10), this->rect().center()-QPointF(10,10)),this);
////    new QGraphicsLineItem(QLineF(this->rect().center()-QPointF(-10,10), this->rect().center()-QPointF(10,-10)),this);

//    //Scale up the ports and so on
//    //! @todo Add a method to mpSelectionBox so the lines could be scaled
//    QList<GUIPort*>::iterator it;
//    for (it = mPortListPtrs.begin(); it != mPortListPtrs.end(); ++it)
//    {
//        (*it)->setTransformOriginPoint((*it)->boundingRect().center());
//        (*it)->setScale(1.0/scale);
//    }

}

void GUIGroup::enterContainer()
{
    //Show this scene
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(getContainedScenePtr());
    connect(gpMainWindow->mpBackButton, SIGNAL(clicked()), this, SLOT(exitContainer()));
    gpMainWindow->mpBackButton->show();

}

//! @brief Exit a container and shows the parents scene instead
void GUIGroup::exitContainer()
{
    gpMainWindow->mpBackButton->hide();
    disconnect(gpMainWindow->mpBackButton, SIGNAL(clicked()), this, SLOT(exitContainer()));
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(this->mpParentContainerObject->getContainedScenePtr());

}


//! Shows the parent scene. Should be called to exit a group.
void GUIGroup::showParent()
{
//    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(this->mpParentContainerObject->getContainedScenePtr());

//    disconnect(gpMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

//    gpMainWindow->mpBackButton->hide();

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
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(getContainedScenePtr());

    gpMainWindow->mpBackButton->show();

    connect(gpMainWindow->mpBackButton,SIGNAL(clicked()),this,SLOT(showParent()));

}

//QGraphicsColorizeEffect *graphicsColor = new QGraphicsColorizeEffect;
//graphicsColor ->setColor(Qt::red);
//graphicsColor->setEnabled(true);
//mpIcon->setGraphicsEffect(graphicsColor);


GUIGroupPort::GUIGroupPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, GUIContainerObject *system, QGraphicsItem *parent)
    : GUIModelObject(position, 0, pAppearanceData, DESELECTED, USERGRAPHICS, system, parent)

{
    //Sets the ports
    //! @todo Only one port in group ports could simplify this
    PortAppearanceMapT::iterator i;
    for (i = mGUIModelObjectAppearance.getPortAppearanceMap().begin(); i != mGUIModelObjectAppearance.getPortAppearanceMap().end(); ++i)
    {
        qDebug() << "DEBUG: " << i.key();
        qreal x = i.value().x;
        qreal y = i.value().y;

        i.value().selectPortIcon("", "", "Undefined"); //Dont realy need to write undefined here, could be empty, (just to make it clear)

//        mGUIModelObjectAppearance.setName(mpParentSystem->mpParentProjectTab->mGUIRootSystem.addSystemPort(i.key()));
        mGUIModelObjectAppearance.setName(i.key());

        //We supply ptr to rootsystem to indicate that this is a systemport
        //! @todo this is a very bad way of doing this (ptr to rootsystem for systemport), really need to figure out some better way
        mpGuiPort = new GUIPort(mGUIModelObjectAppearance.getName(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(i.value()), this);
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
        mpParentContainerObject->renameGUIModelObject(oldName, newName);
    }
}


int GUIGroupPort::type() const
{
    return Type;
}
