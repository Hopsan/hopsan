#include "GUIContainerObject.h"

//! @todo clean these up
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../Dialogs/ComponentPropertiesDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../Widgets/MessageWidget.h"
#include "../GraphicsView.h"
#include "../Widgets/LibraryWidget.h"
#include "../loadObjects.h"
#include "../CoreAccess.h"
#include "GUIObject.h"
#include "GUIComponent.h"
#include "GUIGroup.h"
#include "GUISystemPort.h"
#include "GUIWidgets.h"
#include "GUISystem.h"
#include "../CopyStack.h"

#include <QDomElement>


GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUIContainerObject *system, QGraphicsItem *parent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
        //Initialize
    setIsCreatingConnector(false);
    mIsRenamingObject = false;
    mPortsHidden = false;
    mUndoDisabled = false;
    mGfxType = USERGRAPHICS;

    mHighestWidgetIndex = 0;

    mPasteOffset = -30;

    //Create the scene
    mpScene = new QGraphicsScene();

//    //Set the parent project tab pointer
//    this->mpParentProjectTab = system->mpParentProjectTab;
    //mpMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;

    //Create the undastack
    mUndoStack = new UndoStack(this);
    mUndoStack->clear();

    //Establish connections
    //connect(this->systemPortAction, SIGNAL(triggered()), SLOT(addSystemPort()));
    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(gpMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(gpMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(gpMainWindow->mpUndoWidget->getUndoButton(), SIGNAL(pressed()), this, SLOT(undo()));
    connect(gpMainWindow->mpUndoWidget->getRedoButton(), SIGNAL(pressed()), this, SLOT(redo()));
    connect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(pressed()), this, SLOT(clearUndo()));
    connect(gpMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this, SLOT(hidePorts(bool)));


}

void GUIContainerObject::makeRootSystem()
{
    mContainerStatus = ROOT;
}

void GUIContainerObject::updateExternalPortPositions()
{
    //Nothing for now
}

GUIContainerObject::CONTAINERSTATUS GUIContainerObject::getContainerStatus()
{
    return mContainerStatus;
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h heigth of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void GUIContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
{
    //! @todo make common PI declaration, maybe also PIhalf or include math.h and use M_PI
    if(angle>3.1415*3.0/2.0)
    {
        x=-std::max(std::min(h/tan(angle), w), -w);
        y=std::max(std::min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415)
    {
        x=-std::max(std::min(h/tan(angle), w), -w);
        y=-std::max(std::min(w*tan(angle), h), -h);
    }
    else if(angle>3.1415/2.0)
    {
        x=std::max(std::min(h/tan(angle), w), -w);
        y=-std::max(std::min(w*tan(angle), h), -h);
    }
    else
    {
        x=std::max(std::min(h/tan(angle), w), -w);
        y=std::max(std::min(w*tan(angle), h), -h);
    }
}


CoreSystemAccess *GUIContainerObject::getCoreSystemAccessPtr()
{
    //Should be overloaded
    return 0;
}

//! @brief Retunrs a pointer to the contained scene
QGraphicsScene *GUIContainerObject::getContainedScenePtr()
{
    return this->mpScene;
}


void GUIContainerObject::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (in case we reload, but maybe we can discard old system and create new in that case)
    //Create the graphics for the ports but do NOT create new ports, use the system ports within the subsystem
    PortAppearanceMapT::iterator it;
    for (it = mGUIModelObjectAppearance.getPortAppearanceMap().begin(); it != mGUIModelObjectAppearance.getPortAppearanceMap().end(); ++it)
    {
        //! @todo fix this
        //qDebug() << "getNode and portType for " << it.key();
        //SystemPort "Component Name" (GuiModelObjectName) and portname is same
        //One other way would be to ask our parent to find the types of our ports but that would be even more strange and would not work on the absolute root system
        //! @todo to minimaze search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
        QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        qreal x = it.value().x;
        qreal y = it.value().y;

        //qDebug() << "this-type(): " << this->type();
        GUIPort *pNewPort = new GUIPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
        mPortListPtrs.append(pNewPort);
    }
}

//! @brief Temporary addSubSystem functin should be same later on
//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
//! @returns a pointer to the created and added object
//! @todo only modelobjects for now
GUIModelObject* GUIContainerObject::addGUIModelObject(GUIModelObjectAppearance* pAppearanceData, QPoint position, qreal rotation, selectionStatus startSelected, nameVisibility nameStatus, undoStatus undoSettings)
{
        //Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();

    //qDebug()  << "Adding GUIModelObject, typename: " << componentTypeName << " displayname: " << pAppearanceData->getName() << " systemname: " << this->getName();
    QString componentTypeName = pAppearanceData->getTypeName();
    if (componentTypeName == "Subsystem")
    {
        mpTempGUIModelObject= new GUISystem(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }
    else if (componentTypeName == "SystemPort") //!< @todo dont hardcode
    {
        mpTempGUIModelObject = new GUISystemPort(pAppearanceData, position, rotation, this, startSelected, mGfxType);
    }
    else //Assume some standard component type
    {
        mpTempGUIModelObject = new GUIComponent(pAppearanceData, position, rotation, this, startSelected, mGfxType);
    }

    mpScene->addItem(mpTempGUIModelObject);

    emit checkMessages();

    if ( mGUIModelObjectMap.contains(mpTempGUIModelObject->getName()) )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Trying to add component with name: " + mpTempGUIModelObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
        //! @todo Is this check really necessary? Two objects cannot have the same name anyway...
    }
    else
    {
        mGUIModelObjectMap.insert(mpTempGUIModelObject->getName(), mpTempGUIModelObject);
    }

        //Set name visibility status (no undo because it will be registered as an added object anyway)
    if(nameStatus == NAMEVISIBLE)
    {
        mpTempGUIModelObject->showName(NOUNDO);
    }
    else
    {
        mpTempGUIModelObject->hideName(NOUNDO);
    }


    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedObject(mpTempGUIModelObject);
    }

    //this->setFocus();

    return mpTempGUIModelObject;
}



void GUIContainerObject::addTextWidget(QPoint position)
{
    GUITextWidget *tempTextWidget;
    tempTextWidget = new GUITextWidget("Text", position, 0, DESELECTED, this, mHighestWidgetIndex);
    mTextWidgetList.append(tempTextWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempTextWidget);
    ++mHighestWidgetIndex;
}


void GUIContainerObject::addBoxWidget(QPoint position, undoStatus undoSettings)
{
    qDebug() << "Adding box widget, index = " << mHighestWidgetIndex;
    GUIBoxWidget *tempBoxWidget;
    tempBoxWidget = new GUIBoxWidget(position, 0, DESELECTED, this, mHighestWidgetIndex);
    mBoxWidgetList.append(tempBoxWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempBoxWidget);
    ++mHighestWidgetIndex;
    if(undoSettings == UNDO)
    {
        mUndoStack->newPost();
        mUndoStack->registerAddedBoxWidget(tempBoxWidget);
    }
}


//! Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void GUIContainerObject::deleteGUIModelObject(QString objectName, undoStatus undoSettings)
{
    //qDebug() << "deleteGUIModelObject(): " << objectName << " in: " << this->getName() << " coresysname: " << this->getCoreSystemAccessPtr()->getRootSystemName() ;
    GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(objectName);
    GUIModelObject* obj_ptr = it.value();

    QList<GUIConnector *> pConnectorList = obj_ptr->getGUIConnectorPtrs();
    for(int i=0; i<pConnectorList.size(); ++i)
    {
        this->removeConnector(pConnectorList[i], undoSettings);
    }

    if (undoSettings == UNDO)
    {
        //Register removal of connector in undo stack (must be done after removal of connectors or the order of the commands in the undo stack will be wrong!)
        this->mUndoStack->registerDeletedObject(it.value());
        emit componentChanged();
    }

    if (it != mGUIModelObjectMap.end())
    {
        //qDebug() << "Höns från Korea";
        mGUIModelObjectMap.erase(it);
        mSelectedGUIObjectsList.removeOne(obj_ptr);
        mpScene->removeItem(obj_ptr);
        delete(obj_ptr);
        emit checkMessages();
    }
    else
    {
        //qDebug() << "In delete GUIObject: could not find object with name " << objectName;
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Could not delete object with name " + objectName + ", object not found");
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! This function is used to rename a SubGUIObject
void GUIContainerObject::renameGUIModelObject(QString oldName, QString newName, undoStatus undoSettings)
{
    //Avoid work if no change is requested
    if (oldName != newName)
    {
        QString modNewName;
            //First find record with old name
        GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(oldName);
        if (it != mGUIModelObjectMap.end())
        {
                //Make a backup copy
            GUIModelObject* obj_ptr = it.value();
                //Erase old record
            mGUIModelObjectMap.erase(it);
                //Set new name, first in core then in gui object
            //qDebug() << "Renaming: " << oldName << " " << newName << " type: " << obj_ptr->type();
            switch (obj_ptr->type())
            {
            case GUICOMPONENT:
                //qDebug() << "GUICOMPONENT";
            case GUISYSTEM :
                //qDebug() << "GUISYSTEM";
                modNewName = this->getCoreSystemAccessPtr()->renameSubComponent(oldName, newName);
                break;
            case GUISYSTEMPORT :
                //qDebug() << "GUISYSTEMPORT";
                modNewName = this->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName);
                break;
            //default :
                //qDebug() << "default";
                    //No Core rename action
            }
            //qDebug() << "modNewName: " << modNewName;
            obj_ptr->setDisplayName(modNewName);
                //Re insert
            mGUIModelObjectMap.insert(obj_ptr->getName(), obj_ptr);
        }
        else
        {
            //qDebug() << "Old name: " << oldName << " not found";
            //! @todo Maybe we should give the user a message?
        }

        if (undoSettings == UNDO)
        {
            mUndoStack->newPost();
            mUndoStack->registerRenameObject(oldName, modNewName);
            emit componentChanged();
        }
    }
    emit checkMessages();
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GUIContainerObject::hasGUIModelObject(QString name)
{
    return (mGUIModelObjectMap.count(name) > 0);
}


//! Returns a pointer to the component with specified name.
GUIModelObject *GUIContainerObject::getGUIModelObject(QString name)
{
    if(!mGUIModelObjectMap.contains(name))
    {
        qDebug() << "Request for pointer to non-existing component" << endl;
        assert(false);
    }
    return mGUIModelObjectMap.find(name).value();
}


//! @brief Find a connector in the connector vector
GUIConnector* GUIContainerObject::findConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    GUIConnector *item;
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if((mSubConnectorList[i]->getStartComponentName() == startComp) &&
           (mSubConnectorList[i]->getStartPortName() == startPort) &&
           (mSubConnectorList[i]->getEndComponentName() == endComp) &&
           (mSubConnectorList[i]->getEndPortName() == endPort))
        {
            item = mSubConnectorList[i];
            break;
        }
        //Find even if the caller mixed up start and stop
        else if((mSubConnectorList[i]->getStartComponentName() == endComp) &&
                (mSubConnectorList[i]->getStartPortName() == endPort) &&
                (mSubConnectorList[i]->getEndComponentName() == startComp) &&
                (mSubConnectorList[i]->getEndPortName() == startPort))
        {
            item = mSubConnectorList[i];
            break;
        }
    }
    assert(!item == 0);
    return item;
}


//! @brief Tells whether or not there is a connector between two specified ports
bool GUIContainerObject::hasConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if((mSubConnectorList[i]->getStartComponentName() == startComp) &&
           (mSubConnectorList[i]->getStartPortName() == startPort) &&
           (mSubConnectorList[i]->getEndComponentName() == endComp) &&
           (mSubConnectorList[i]->getEndPortName() == endPort))
        {
            return true;
        }
        //Find even if the caller mixed up start and stop
        else if((mSubConnectorList[i]->getStartComponentName() == endComp) &&
                (mSubConnectorList[i]->getStartPortName() == endPort) &&
                (mSubConnectorList[i]->getEndComponentName() == startComp) &&
                (mSubConnectorList[i]->getEndPortName() == startPort))
        {
            return true;
        }
    }
    return false;
}


//! Removes the connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GUIContainerObject::removeConnector(GUIConnector* pConnector, undoStatus undoSettings)
{
    bool doDelete = false;
    bool startPortHasMoreConnections = false;
    bool endPortWasConnected = false;
    bool endPortHasMoreConnections = false;
    int i;

    //qDebug() << "Svampar i min diskho";

    if(undoSettings == UNDO)
    {
        mUndoStack->registerDeletedConnector(pConnector);
    }

    for(i = 0; i < mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i] == pConnector)
        {
             //! @todo some error handling both ports must exist and be connected to each other
             if(pConnector->isConnected())
             {
                 GUIPort *pStartP = pConnector->getStartPort();
                 GUIPort *pEndP = pConnector->getEndPort();
                 this->getCoreSystemAccessPtr()->disconnect(pStartP->getGuiModelObjectName(), pStartP->getName(), pEndP->getGuiModelObjectName(), pEndP->getName());
                 emit checkMessages();
                 endPortWasConnected = true;
             }
             doDelete = true;
        }
        else if( (pConnector->getStartPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getStartPort() == mSubConnectorList[i]->getEndPort()) )
        {
            startPortHasMoreConnections = true;
        }
        else if( (pConnector->getEndPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getEndPort() == mSubConnectorList[i]->getEndPort()) )
        {
            endPortHasMoreConnections = true;
        }
        if(mSubConnectorList.empty())
        {
            break;
        }
    }

    if(endPortWasConnected && !endPortHasMoreConnections)
    {
        pConnector->getEndPort()->setVisible(!mPortsHidden);
        pConnector->getEndPort()->setIsConnected(false);
    }

    if(!startPortHasMoreConnections)
    {
        pConnector->getStartPort()->setVisible(!mPortsHidden);
        pConnector->getStartPort()->setIsConnected(false);
    }
    else if(startPortHasMoreConnections && !endPortWasConnected)
    {
        pConnector->getStartPort()->setVisible(false);
        pConnector->getStartPort()->setIsConnected(true);
    }

    if(doDelete)
    {
        mSubConnectorList.removeAll(pConnector);
        mSelectedSubConnectorsList.removeAll(pConnector);
        mpScene->removeItem(pConnector);
        delete pConnector;
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}



//! Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
void GUIContainerObject::createConnector(GUIPort *pPort, undoStatus undoSettings)
{
    qDebug() << "mIsCreatingConnector: " << getIsCreatingConnector();
        //When clicking start port
    if (!getIsCreatingConnector())
    {
        qDebug() << "CreatingConnector in: " << this->getName() << " startPortName: " << pPort->getName();
        //GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(pPort->getPortType(), mpParentProjectTab->setGfxType);
        mpTempConnector = new GUIConnector(pPort, this);
        emit deselectAllGUIObjects();
        emit deselectAllGUIConnectors();
        setIsCreatingConnector(true);
        mpTempConnector->drawConnector();
    }
        //When clicking end port
    else
    {
        qDebug() << "clicking end port: " << pPort->getName();
        GUIPort *pStartPort = mpTempConnector->getStartPort();

        bool success = this->getCoreSystemAccessPtr()->connect(pStartPort->getGuiModelObjectName(), pStartPort->getName(), pPort->getGuiModelObjectName(), pPort->getName() );
        qDebug() << "GUI Connect: " << success;
        if (success)
        {
            setIsCreatingConnector(false);
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->updateEndPoint(newPos);
            pPort->getGuiModelObject()->rememberConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

                //Hide ports; connected ports shall not be visible
            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            mSubConnectorList.append(mpTempConnector);

            mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
            if(undoSettings == UNDO)
            {
                mUndoStack->registerAddedConnector(mpTempConnector);
            }
        }
        emit checkMessages();
     }
}

//! Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void GUIContainerObject::cutSelected(CopyStack *xmlStack)
{
    this->copySelected(xmlStack);
    this->mUndoStack->newPost("cut");
    emit deleteSelected();
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void GUIContainerObject::copySelected(CopyStack *xmlStack)
{
    //mUndoStack->newPost();
    gCopyStack.clear();

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        copyRoot = gCopyStack.getCopyRoot();
    }
    else
    {
        copyRoot = xmlStack->getCopyRoot();
    }

        //Copy components
    QList<GUIObject *>::iterator it;
    for(it = mSelectedGUIObjectsList.begin(); it!=mSelectedGUIObjectsList.end(); ++it)
    {
        (*it)->saveToDomElement(*copyRoot);
    }

    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->getStartPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->getEndPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->isActive())
        {
            mSubConnectorList[i]->saveToDomElement(*copyRoot);
        }
    }
}


//! Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GUIContainerObject::paste(CopyStack *xmlStack)
{

    //gpMainWindow->mpMessageWidget->printGUIDebugMessage(gCopyStack.getXML());

    mUndoStack->newPost("paste");
    mpParentProjectTab->hasChanged();

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        copyRoot = gCopyStack.getCopyRoot();
    }
    else
    {
        copyRoot = xmlStack->getCopyRoot();
    }

        //Deselect all components & connectors
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();

    QHash<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called

        //Paste components
    QDomElement objectElement = copyRoot->firstChildElement("component");
    while(!objectElement.isNull())
    {
        GUIObject *pObj = loadGUIModelObject(objectElement, gpMainWindow->mpLibrary, this);

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(mPasteOffset, mPasteOffset);
        mUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());

        renameMap.insert(objectElement.attribute("name"), pObj->getName());
        objectElement.setAttribute("name", renameMap.find(objectElement.attribute("name")).value());
        objectElement = objectElement.nextSiblingElement("component");
    }

        //Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement("connect");
    while(!connectorElement.isNull())
    {
            //Replace names of start and end component, since they likely have been changed
        connectorElement.setAttribute("startcomponent", renameMap.find(connectorElement.attribute("startcomponent")).value());
        connectorElement.setAttribute("endcomponent", renameMap.find(connectorElement.attribute("endcomponent")).value());

        loadConnector(connectorElement, this, UNDO);

        GUIConnector *tempConnector = this->findConnector(connectorElement.attribute("startcomponent"), connectorElement.attribute("startport"),
                                                          connectorElement.attribute("endcomponent"), connectorElement.attribute("endport"));

            //Apply offset to connector and register it in undo stack
//        QList<QPointF> oldPosList;
//        for(size_t i=0; i<tempConnector->getNumberOfLines()-2; ++i)
//        {
//            oldPosList.append(tempConnector->getLine(i+1)->pos());
//        }
        tempConnector->moveAllPoints(mPasteOffset, mPasteOffset);
        tempConnector->drawConnector();
        for(int i=0; i<(tempConnector->getNumberOfLines()-2); ++i)
        {
            mUndoStack->registerModifiedConnector(QPointF(tempConnector->getLine(i)->pos().x()-mPasteOffset, tempConnector->getLine(i)->pos().y()-mPasteOffset),
                                                  tempConnector->getLine(i+1)->pos(), tempConnector, i+1);
        }

        connectorElement = connectorElement.nextSiblingElement("connect");
    }

        //Paste text widgets
    QDomElement textElement = copyRoot->firstChildElement("textwidget");
    while(!textElement.isNull())
    {
        loadTextWidget(textElement, this);
        mTextWidgetList.last()->setSelected(true);
        mTextWidgetList.last()->moveBy(mPasteOffset, mPasteOffset);
        textElement = textElement.nextSiblingElement("textwidget");
    }

        //Paste box widgets
    QDomElement boxElement = copyRoot->firstChildElement("boxwidget");
    while(!boxElement.isNull())
    {
        loadBoxWidget(boxElement, this);
        mBoxWidgetList.last()->setSelected(true);
        mBoxWidgetList.last()->moveBy(mPasteOffset, mPasteOffset);
        boxElement = boxElement.nextSiblingElement("boxwidget");
    }

        //Select all pasted comonents
    QHash<QString, QString>::iterator itn;
    for(itn = renameMap.begin(); itn != renameMap.end(); ++itn)
    {
        mGUIModelObjectMap.find(itn.value()).value()->setSelected(true);
    }

    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Groups the selected objects together
void GUIContainerObject::groupSelected(QPointF pt)
{
    //! @todo dont hardcode group appearance like this
    GUIModelObjectAppearance appdata;
    appdata.setIconPathUser("subsystemtmp.svg");
    appdata.setBaseIconPath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD

    //! @todo add like all other guimodelobjects add as proper object
    GUIGroup *pGroup = new GUIGroup(pt.toPoint(), 0.0, &appdata, this);
    this->getContainedScenePtr()->addItem(pGroup);

    CopyStack copyStack;
    this->cutSelected(&copyStack);
    //this->copySelected(&copyStack);
    //pGroup->setContents(&copyStack);
}


//! Selects all objects and connectors.
void GUIContainerObject::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllGUIConnectors();
}


//! Deselects all objects and connectors.
void GUIContainerObject::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();
}


//! Hides all component names.
//! @see showNames()
void GUIContainerObject::hideNames()
{
    mUndoStack->newPost("hideallnames");
    mIsRenamingObject = false;
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! Shows all component names.
//! @see hideNames()
void GUIContainerObject::showNames()
{
    mUndoStack->newPost("showallnames");
    emit showAllNameText();
}


//! Slot that sets hide ports flag to true or false
void GUIContainerObject::hidePorts(bool doIt)
{
    mPortsHidden = doIt;
}


//! Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void GUIContainerObject::undo()
{
    mUndoStack->undoOneStep();
}


//! Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void GUIContainerObject::redo()
{
    mUndoStack->redoOneStep();
}

//! Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void GUIContainerObject::clearUndo()
{
    mUndoStack->clear();
}


//! Returns true if at least one GUIObject is selected
bool GUIContainerObject::isObjectSelected()
{
    return (mSelectedGUIObjectsList.size() > 0);
}


//! Returns true if at least one GUIConnector is selected
bool GUIContainerObject::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}

QString GUIContainerObject::getUserIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathUser();
}

//! @todo do we return full path or relative
QString GUIContainerObject::getIsoIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathISO();
}

//! @todo do we safe full path or relative
void GUIContainerObject::setUserIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathUser(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}

void GUIContainerObject::setIsoIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathISO(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}

//! Access function for mIsCreatingConnector
//! @param isConnected is the new value
void GUIContainerObject::setIsCreatingConnector(bool isCreatingConnector)
{
    mIsCreatingConnector = isCreatingConnector;
}


//! Access function for mIsCreatingConnector
bool GUIContainerObject::getIsCreatingConnector()
{
    return mIsCreatingConnector;
}


//! Disables the undo function for the current model
void GUIContainerObject::disableUndo()
{
    if(!mUndoDisabled)
    {
        QMessageBox disableUndoWarningBox(QMessageBox::Warning, tr("Warning"),tr("Disabling undo history will clear all undo history for this model. Do you want to continue?"), 0, 0);
        disableUndoWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
        disableUndoWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);
        disableUndoWarningBox.setWindowIcon(gpMainWindow->windowIcon());

        if (disableUndoWarningBox.exec() == QMessageBox::AcceptRole)
        {
            this->clearUndo();
            mUndoDisabled = true;
            gpMainWindow->undoAction->setDisabled(true);
            gpMainWindow->redoAction->setDisabled(true);
        }
        else
        {
            return;
        }
    }
    else
    {
        mUndoDisabled = false;
        gpMainWindow->undoAction->setDisabled(false);
        gpMainWindow->redoAction->setDisabled(false);
    }
}


//! Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void GUIContainerObject::updateUndoStatus()
{
    if(mUndoDisabled)
    {
        gpMainWindow->undoAction->setDisabled(true);
        gpMainWindow->redoAction->setDisabled(true);
    }
    else
    {
        gpMainWindow->undoAction->setDisabled(false);
        gpMainWindow->redoAction->setDisabled(false);
    }
}

//! Sets the iso graphics option for the model
void GUIContainerObject::setGfxType(graphicsType gfxType)
{
    this->mGfxType = gfxType;
    this->mpParentProjectTab->mpGraphicsView->updateViewPort();
    emit setAllGfxType(mGfxType);
}

//! @brief A slot that opens the properties dialog
void GUIContainerObject::openPropertiesDialogSlot()
{
    this->openPropertiesDialog();
}


//! Slot that tells all selected name texts to deselect themselves
void GUIContainerObject::deselectSelectedNameText()
{
    emit deselectAllNameText();
}

//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUIContainerObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    QAction *pAction = this->buildBaseContextMenu(menu, event->screenPos());
    if (pAction == loadAction)
    {
        //! @todo use loadHMF once we have scraped teh text based stuff and only uses xml
        //loadFromHMF();
        QDir fileDialog;
        QFile file;
        QString modelFilePath = QFileDialog::getOpenFileName(mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
                                                             fileDialog.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));

        file.setFileName(modelFilePath);
        QDomDocument domDocument;
        QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
        if (!hmfRoot.isNull())
        {
            //! @todo Check version numbers
            //! @todo check if we could load else give error message and dont attempt to load
            QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
            this->setModelFileInfo(file); //Remember info about the file from which the data was loaded
            this->loadFromDomElement(systemElement);
        }
    }
    QGraphicsItem::contextMenuEvent(event);
}

void GUIContainerObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    this->enterContainer();

}

void GUIContainerObject::openPropertiesDialog()
{
    //! @todo maybe move code from system here
    //Do Nothing
}

void GUIContainerObject::enterContainer()
{
    //Show this scene
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(getContainedScenePtr());
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setContainerPtr(this);
    connect(gpMainWindow->mpBackButton, SIGNAL(clicked()), this, SLOT(exitContainer()));
    gpMainWindow->mpBackButton->show();
}

void GUIContainerObject::exitContainer()
{
    //Go back to parent system
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setScene(this->mpParentContainerObject->getContainedScenePtr());
    mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setContainerPtr(this->mpParentContainerObject);
    disconnect(gpMainWindow->mpBackButton, SIGNAL(clicked()), this, SLOT(exitContainer()));
    gpMainWindow->mpBackButton->hide();
}
