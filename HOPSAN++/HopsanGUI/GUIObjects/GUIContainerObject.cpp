//!
//! @file   GUIContainerObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Container class (base class for Systems and Groups)
//!
//$Id$

#include "GUIContainerObject.h"

//! @todo clean these up, they are nott all needed probably, copied from elsewere
#include "../MainWindow.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../UndoStack.h"
#include "../GraphicsView.h"
#include "../loadObjects.h"
#include "../CoreAccess.h"
#include "../CopyStack.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Widgets/MessageWidget.h"
#include "../Widgets/LibraryWidget.h"
#include "../Widgets/QuickNavigationWidget.h"
#include "../Widgets/PlotWidget.h"
#include "../Utilities/GUIUtilities.h"
#include "GUIComponent.h"
#include "GUIGroup.h"
#include "GUIContainerPort.h"
#include "GUIWidgets.h"
#include "GUISystem.h"

#include <limits>
#include <QDomElement>


//! @brief Construtor for container objects.
//! @param position Initial position where container object is to be placed in its parent container
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to the appearance data object
//! @param startSelected Tells whether or not the object is initially selected
//! @param gfxType Tells whether the initial graphics shall be user or ISO
//! @param pParentContainer Pointer to the parent container object (leave empty if not a sub container)
//! @param pParent Pointer to parent object
GUIContainerObject::GUIContainerObject(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, GUIContainerObject *pParentContainer, QGraphicsItem *pParent)
        : GUIModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParent)
{
        //Initialize
    setIsCreatingConnector(false);
    mIsRenamingObject = false;
    mPortsHidden = !gpMainWindow->togglePortsAction->isChecked();
    mNamesHidden = !gpMainWindow->toggleNamesAction->isChecked();
    mUndoDisabled = false;
    mGfxType = USERGRAPHICS;

    mHighestWidgetIndex = 0;

    mPasteOffset = -30;

    //Create the scene
    mpScene = new QGraphicsScene(this);

    //Create the undastack
    mUndoStack = new UndoStack(this);
    mUndoStack->clear();

    gpMainWindow->toggleNamesAction->setChecked(true);
    gpMainWindow->togglePortsAction->setChecked(true);

    //Establish connections that should always remain
    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()), Qt::UniqueConnection);

}


//! @brief Destructor for container object
GUIContainerObject::~GUIContainerObject()
{
    qDebug() << ",,,,,,,,,,,,GUIContainer destructor";
}

//! @brief Connects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are swithching what continer we want to the buttons to trigger actions in
void GUIContainerObject::connectMainWindowActions()
{
    connect(gpMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getUndoButton(), SIGNAL(clicked()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getRedoButton(), SIGNAL(clicked()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()), Qt::UniqueConnection);

    //connect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        this,     SLOT(hideNames()), Qt::UniqueConnection);
    //connect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        this,     SLOT(showNames()), Qt::UniqueConnection);
    connect(gpMainWindow->togglePortsAction,    SIGNAL(triggered(bool)),    this,     SLOT(hidePorts(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->toggleNamesAction,    SIGNAL(triggered(bool)),    this,     SLOT(toggleNames(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        this,     SLOT(disableUndo()), Qt::UniqueConnection);
    connect(gpMainWindow->cutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()), Qt::UniqueConnection);
    connect(gpMainWindow->copyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()), Qt::UniqueConnection);
    connect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()), Qt::UniqueConnection);
    connect(gpMainWindow->alignXAction,         SIGNAL(triggered()),        this,     SLOT(alignX()), Qt::UniqueConnection);
    connect(gpMainWindow->alignYAction,         SIGNAL(triggered()),        this,     SLOT(alignY()), Qt::UniqueConnection);
    connect(gpMainWindow->rotateRightAction,    SIGNAL(triggered()),        this,     SLOT(rotateRight()), Qt::UniqueConnection);
    connect(gpMainWindow->rotateLeftAction,     SIGNAL(triggered()),        this,     SLOT(rotateLeft()), Qt::UniqueConnection);
    connect(gpMainWindow->flipHorizontalAction, SIGNAL(triggered()),        this,     SLOT(flipHorizontal()), Qt::UniqueConnection);
    connect(gpMainWindow->flipVerticalAction,   SIGNAL(triggered()),        this,     SLOT(flipVertical()), Qt::UniqueConnection);
    connect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()), Qt::UniqueConnection);

    connect(gpMainWindow->mpStartTimeLineEdit,  SIGNAL(editingFinished()),  this,     SLOT(updateStartTime()), Qt::UniqueConnection);//! @todo should these be here (start stop ts)?  and duplicates?
    connect(gpMainWindow->mpTimeStepLineEdit,   SIGNAL(editingFinished()),  this,     SLOT(updateTimeStep()), Qt::UniqueConnection);
    connect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()),  this,     SLOT(updateStopTime()), Qt::UniqueConnection);
//    connect(gpMainWindow->mpStartTimeLineEdit,  SIGNAL(editingFinished()),  this,     SLOT(updateStartTime())); //! @todo should these be here (start stop ts)?
//    connect(gpMainWindow->mpFinishTimeLineEdit, SIGNAL(editingFinished()),  this,     SLOT(updateStopTime()));
//    connect(gpMainWindow->mpTimeStepLineEdit,   SIGNAL(editingFinished()),  this,     SLOT(updateTimeStep()));

    //getCurrentContainer()->updateUndoStatus();
}

//! @brief Disconnects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are swithching what continer we want to the buttons to trigger actions in
void GUIContainerObject::disconnectMainWindowActions()
{
    disconnect(gpMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    disconnect(gpMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    disconnect(gpMainWindow->mpUndoWidget->getUndoButton(), SIGNAL(clicked()), this, SLOT(undo()));
    disconnect(gpMainWindow->mpUndoWidget->getRedoButton(), SIGNAL(clicked()), this, SLOT(redo()));
    disconnect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()));

//    disconnect(gpMainWindow->hideNamesAction,       SIGNAL(triggered()),        this,    SLOT(hideNames()));
//    disconnect(gpMainWindow->showNamesAction,       SIGNAL(triggered()),        this,    SLOT(showNames()));
    disconnect(gpMainWindow->toggleNamesAction,     SIGNAL(triggered(bool)),    this,      SLOT(toggleNames(bool)));
    disconnect(gpMainWindow->togglePortsAction,     SIGNAL(triggered(bool)),    this,     SLOT(hidePorts(bool)));
    disconnect(gpMainWindow->disableUndoAction,     SIGNAL(triggered()),        this,    SLOT(disableUndo()));
    disconnect(gpMainWindow->cutAction,             SIGNAL(triggered()),        this,    SLOT(cutSelected()));
    disconnect(gpMainWindow->copyAction,            SIGNAL(triggered()),        this,    SLOT(copySelected()));
    disconnect(gpMainWindow->pasteAction,           SIGNAL(triggered()),        this,    SLOT(paste()));
    disconnect(gpMainWindow->alignXAction,          SIGNAL(triggered()),        this,    SLOT(alignX()));
    disconnect(gpMainWindow->alignYAction,          SIGNAL(triggered()),        this,    SLOT(alignY()));
    disconnect(gpMainWindow->rotateRightAction,     SIGNAL(triggered()),        this,    SLOT(rotateRight()));
    disconnect(gpMainWindow->rotateLeftAction,      SIGNAL(triggered()),        this,    SLOT(rotateLeft()));
    disconnect(gpMainWindow->flipHorizontalAction,  SIGNAL(triggered()),        this,    SLOT(flipHorizontal()));
    disconnect(gpMainWindow->flipVerticalAction,    SIGNAL(triggered()),        this,    SLOT(flipVertical()));
    disconnect(gpMainWindow->propertiesAction,      SIGNAL(triggered()),        this,    SLOT(openPropertiesDialogSlot()));

    disconnect(gpMainWindow->mpStartTimeLineEdit,   SIGNAL(editingFinished()),  this,    SLOT(updateStartTime()));//! @todo should these be here (start stop ts)? and duplicates?
    disconnect(gpMainWindow->mpTimeStepLineEdit,    SIGNAL(editingFinished()),  this,    SLOT(updateTimeStep()));
    disconnect(gpMainWindow->mpFinishTimeLineEdit,  SIGNAL(editingFinished()),  this,    SLOT(updateStopTime()));
//    disconnect(gpMainWindow->mpStartTimeLineEdit,   SIGNAL(editingFinished()),  this,    SLOT(updateStartTime())); //! @todo should these be here (start stop ts)?
//    disconnect(gpMainWindow->mpFinishTimeLineEdit,  SIGNAL(editingFinished()),  this,    SLOT(updateStopTime()));
//    disconnect(gpMainWindow->mpTimeStepLineEdit,    SIGNAL(editingFinished()),  this,    SLOT(updateTimeStep()));
}

//! @brief A helpfunction that determines on which edge an external port should be placed based on its internal position
//! @param[in] center The center point of all objects to be compared with
//! @param[in] pt The position of this object, used to determine the center relative posistion
//! @returns An enum that indicates on which side the port should be placed
GUIContainerObject::CONTAINEREDGE GUIContainerObject::findPortEdge(QPointF center, QPointF pt)
{
    //By swapping place of pt1 and pt2 we get the angle in the same coordinate system as the view
    QPointF diff = pt-center;
    //qDebug() << "=============The Diff: " << diff;

    //If only one sysport default to left side
    //! @todo Do this smarter later and take into account port orientation, or position relative all other components, need to extend this function a bit for that though
    if (diff.manhattanLength() < 1.0)
    {
        return LEFTEDGE;
    }

    //Determine on what edge the port should be placed based on the angle from the center point
    qreal angle = normRad(qAtan2(diff.x(), diff.y()));
    //qDebug() << "angle: " << rad2deg(angle);
    if (fabs(angle) <= M_PI_4)
    {
        return RIGHTEDGE;
    }
    else if (fabs(angle) >= 3.0*M_PI_4)
    {
        return LEFTEDGE;
    }
    else if (angle > M_PI_4)
    {
        return BOTTOMEDGE;
    }
    else
    {
        return TOPEDGE;
    }
}

//! @brief Refreshes the appearance and postion of all external ports
void GUIContainerObject::refreshExternalPortsAppearanceAndPosition()
{
    //refresh the external port poses
    GUIModelObjectMapT::iterator moit;
    double val;

    //Set the initial values to be overwriten by the if bellow
    double xMin=std::numeric_limits<double>::max(), xMax=-xMin, yMin=xMin, yMax=xMax;
    for(moit = mGUIModelObjectMap.begin(); moit != mGUIModelObjectMap.end(); ++moit)
    {
        //if(moit.value()->type() == GUICONTAINERPORT)
        //{
            //check x max and min
            val = moit.value()->getCenterPos().x();
            xMin = std::min(xMin,val);
            xMax = std::max(xMax,val);
            //check y max and min
            val = moit.value()->getCenterPos().y();
            yMin = std::min(yMin,val);
            yMax = std::max(yMax,val);
        //}
    }
    //! @todo Find out if it is possible to ask the scene or view for this information instead of calulating it ourselves
    QPointF center = QPointF((xMax+xMin)/2.0, (yMax+yMin)/2.0);
    //qDebug() << "center max min: " << center << " " << xMin << " " << xMax << " " << yMin << " " << yMax;

    QVector<GUIPort*> leftEdge;
    QVector<GUIPort*> rightEdge;
    QVector<GUIPort*> topEdge;
    QVector<GUIPort*> bottomEdge;

    for(moit = mGUIModelObjectMap.begin(); moit != mGUIModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == GUICONTAINERPORT)
        {
            //            QLineF line = QLineF(center, moit.value()->getCenterPos());
            //            this->getContainedScenePtr()->addLine(line); //debug-grej

            CONTAINEREDGE edge = findPortEdge(center, moit.value()->getCenterPos());
            //qDebug() << " sysp: " << moit.value()->getName() << " edge: " << edge;

            //MAke sure we dont screw up in the code and forget to rename or create external ports on internal rename or create
            assert(this->getPort(moit.value()->getName()) != 0);

            switch (edge) {
            case RIGHTEDGE:
                rightEdge.append(this->getPort(moit.value()->getName()));
                break;
            case BOTTOMEDGE:
                bottomEdge.append(this->getPort(moit.value()->getName()));
                break;
            case LEFTEDGE:
                leftEdge.append(this->getPort(moit.value()->getName()));
                break;
            case TOPEDGE:
                topEdge.append(this->getPort(moit.value()->getName()));
                break;
            }
        }
    }

    //Now disperse the port icons evenly along each edge
    QVector<GUIPort*>::iterator it;
    qreal disp; //Dispersion factor
    qreal sdisp; //sumofdispersionfactors

    //! @todo maybe we should be able to update rotation in all of these also
    //! @todo need to be sure we sort them in the correct order, that is the port in top left will be first (highest up) among the external ports
    //! @todo wierd to use createfunction to refresh graphics, but ok for now
    disp = 1.0/((qreal)(rightEdge.size()+1));
    sdisp=disp;
    for (it=rightEdge.begin(); it!=rightEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(1.0, sdisp);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(bottomEdge.size()+1));
    sdisp=disp;
    for (it=bottomEdge.begin(); it!=bottomEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(sdisp, 1.0);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(leftEdge.size()+1));
    sdisp=disp;
    for (it=leftEdge.begin(); it!=leftEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(0.0, sdisp);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(topEdge.size()+1));
    sdisp=disp;
    for (it=topEdge.begin(); it!=topEdge.end(); ++it)
    {
        (*it)->updatePositionByFraction(sdisp, 0.0);
        this->createExternalPort((*it)->getName());    //refresh the external port graphics
        sdisp += disp;
    }
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
    double tanAngle = tan(angle);//Otherwise division by zero
    if(fabs(tanAngle) < 0.0001)
    {
        tanAngle=.0001;
    }
    if(angle>3.1415*3.0/2.0)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>3.1415)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=-std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>3.1415/2.0)
    {
        x=std::max(std::min(h/tanAngle, w), -w);
        y=-std::max(std::min(w*tanAngle, h), -h);
    }
    else
    {
        x=std::max(std::min(h/tanAngle, w), -w);
        y=std::max(std::min(w*tanAngle, h), -h);
    }
}


//! @brief Returns a pointer to the CoreSystemAccess that this container represents
//! @returns Pointer the the CoreSystemAccess that this container represents
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
    //! @todo maybe try to make this function the same as refreshExternal.... and have one common function in modelobject, component and containerports class,
    //This one should not be used in this class only for component and containerport
    assert(false);
}


//! @brief This method creates ONE external port. Or refreshes existing ports. It assumes that port appearance information for this port exists
//! @param[portName] The name of the port to create
//! @todo maybe defualt create that info if it is missing
void GUIContainerObject::createExternalPort(QString portName)
{
    //If port appearance is not already existing then we create it
    if ( mGUIModelObjectAppearance.getPortAppearanceMap().count(portName) == 0 )
    {
        mGUIModelObjectAppearance.addPortAppearance(portName);
    }

    //Fetch appearance data
    PortAppearanceMapT::iterator it = mGUIModelObjectAppearance.getPortAppearanceMap().find(portName);
    if (it != mGUIModelObjectAppearance.getPortAppearanceMap().end())
    {
        //! @todo to minimaze search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
        QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        //Create new external port if if does not already exist (this is the usual case for individual components)
        GUIPort *pPort = this->getPort(it.key());
        if ( pPort == 0 )
        {
            qDebug() << "##This is OK though as this means that we should create the stupid port for the first time";

            qreal x = it.value().x;
            qreal y = it.value().y;

            if (this->type() == GUIGROUP)
            {
                pPort = new GroupPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
            }
            else
            {
                pPort = new GUIPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
            }


            mPortListPtrs.append(pPort);
        }
        else
        {

            //The external port already seems to exist, lets update it incase something has changed
            //! @todo Maybe need to have a refresh portappearance function, dont really know if thiss will ever be used though, will fix when it becomes necessary
            pPort->refreshPortGraphics();
            qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << it.key() << " in: " << this->getName();
        }
    }
    else
    {
        //This should never happen
        qDebug() << "Could not find portappearance info for port: " << portName << " in: " << this->getName();
        assert(false);
    }
}


//! @breif Removes an external Port from a container object
//! @param[in] portName The name of the port to be removed
//! @todo maybe we should use a map instead to make delete more efficient, (may not amtter usually not htat many external ports)
void GUIContainerObject::removeExternalPort(QString portName)
{
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
    QList<GUIPort*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == portName )
        {
            //Delete the GUIPort its post in teh portlist and its appearance data
            mGUIModelObjectAppearance.erasePortAppearance(portName);
            delete *plit;
            mPortListPtrs.erase(plit);
            break;
        }
    }
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
}


//! @brief Reanmes an external GUIPort
//! @param[in] oldName The name to be replaced
//! @param[in] newName The new name
//! This function assumes that oldName exist and that newName is correct, no error checking is done
void GUIContainerObject::renameExternalPort(const QString oldName, const QString newName)
{
    QList<GUIPort*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == oldName )
        {
            //Rename the port appearance data by remove and re-add
            GUIPortAppearance tmp = mGUIModelObjectAppearance.getPortAppearanceMap().value(oldName);
            mGUIModelObjectAppearance.erasePortAppearance(oldName);
            mGUIModelObjectAppearance.addPortAppearance(newName, &tmp);

            //Rename port
            (*plit)->setDisplayName(newName);
            break;
        }
    }
}


//! @brief Creates and adds a GuiModel Object to the current container
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
    if (componentTypeName == HOPSANGUISYSTEMTYPENAME)
    {
        mpTempGUIModelObject= new GUISystem(position, rotation, pAppearanceData, this, startSelected, mGfxType);

            //Disconnect new subsystem with ctrl-z and ctrl-y (they will be reconnected when entering system)
//        //! @todo make sure if this is needed now that we have the "refresh connections function"
//        disconnect(gpMainWindow->undoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(undo()));
//        disconnect(gpMainWindow->redoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(redo()));
    }
    else if (componentTypeName == HOPSANGUICONTAINERPORTTYPENAME)
    {
        mpTempGUIModelObject = new GUIContainerPort(pAppearanceData, position, rotation, this, startSelected, mGfxType);
//        //Add appearance data for the external version of this systemport to the continer object so that the external port can be created with the creatPorts method
//        mGUIModelObjectAppearance.getPortAppearanceMap().insert(mpTempGUIModelObject->getName(), GUIPortAppearance()); //! @todo maybe this should be handeled automatically inside create external port if missing
        this->createExternalPort(mpTempGUIModelObject->getName());
        this->refreshExternalPortsAppearanceAndPosition();
    }
    else if (componentTypeName == HOPSANGUIGROUPTYPENAME)
    {
        mpTempGUIModelObject = new GUIGroup(position, rotation, pAppearanceData, this);
        //Disconnect new subsystem with ctrl-z and ctrl-y (they will be reconnected when entering system)
//        //! @todo make sure if this is needed now that we have the "refresh connections function"
//        disconnect(gpMainWindow->undoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(undo()));
//        disconnect(gpMainWindow->redoAction, SIGNAL(triggered()), mpTempGUIModelObject, SLOT(redo()));
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
    else if(nameStatus == NAMENOTVISIBLE)
    {
        mpTempGUIModelObject->hideName(NOUNDO);
    }
    else if(!mNamesHidden)
    {
        mpTempGUIModelObject->showName(NOUNDO);
    }
    else if(mNamesHidden)
    {
        mpTempGUIModelObject->hideName(NOUNDO);
    }

    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedObject(mpTempGUIModelObject);
    }


    mpTempGUIModelObject->setSelected(false);
    mpTempGUIModelObject->setSelected(true);
    //this->setFocus();


    return mpTempGUIModelObject;
}


//! @brief Returns a list with the favorite plot parameters.
QList<QStringList> GUIContainerObject::getFavoriteParameters()
{
    return mFavoriteParameters;
}


//! @brief Defines a new favorite plot parameter
//! @param componentName Name of the component where the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Unit of the parameter
void GUIContainerObject::setFavoriteParameter(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QStringList tempParameter;
    tempParameter.append(componentName);
    tempParameter.append(portName);
    tempParameter.append(dataName);
    tempParameter.append(dataUnit);
    if(!mFavoriteParameters.contains(tempParameter))
    {
        mFavoriteParameters.append(tempParameter);
    }
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();

    mpParentProjectTab->hasChanged();
}


//! @brief Removes all favorite parameters which belongs to the specified component.
//! @param componentName Name of the component
void GUIContainerObject::removeFavoriteParameterByComponentName(QString componentName)
{
    QList<QStringList>::iterator it;
    for(it=mFavoriteParameters.begin(); it!=mFavoriteParameters.end(); ++it)
    {
        if((*it).at(0) == componentName)
        {
            mFavoriteParameters.removeAll((*it));
            //! @todo why do we need to care about plotwidget stuff after removing favoriteparameters ?
            //! Because favorite parameters are shown in the plot widget!
            gpMainWindow->makeSurePlotWidgetIsCreated();
            gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();
            return;
        }
    }

    mpParentProjectTab->hasChanged();   //! @todo Is this necessary here?
}


//! @brief Inserts a new text widget to the container
//! @param position Initial position of the widget
//! @param undoSettings Tells whether or not this shall be registered in the undo stack
void GUIContainerObject::addTextWidget(QPoint position, undoStatus undoSettings)
{
    GUITextWidget *tempTextWidget;
    tempTextWidget = new GUITextWidget("Text", position, 0, DESELECTED, this, mHighestWidgetIndex);
    mTextWidgetList.append(tempTextWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempTextWidget);
    ++mHighestWidgetIndex;
    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedTextWidget(tempTextWidget);
    }
    mpParentProjectTab->hasChanged();
}


//! @brief Inserts a new box widget to the container
//! @param position Initial position of the widget
//! @param undoSettings Tells whether or not this shall be registered in the undo stack
void GUIContainerObject::addBoxWidget(QPoint position, undoStatus undoSettings)
{
    GUIBoxWidget *tempBoxWidget;
    tempBoxWidget = new GUIBoxWidget(position, 0, DESELECTED, this, mHighestWidgetIndex);
    mBoxWidgetList.append(tempBoxWidget);
    mWidgetMap.insert(mHighestWidgetIndex, tempBoxWidget);
    ++mHighestWidgetIndex;
    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedBoxWidget(tempBoxWidget);
    }
    mpParentProjectTab->hasChanged();
}


//! @brief Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void GUIContainerObject::deleteGUIModelObject(QString objectName, undoStatus undoSettings)
{
    qDebug() << "deleteGUIModelObject(): " << objectName << " in: " << this->getName() << " coresysname: " << this->getCoreSystemAccessPtr()->getRootSystemName() ;
    this->removeFavoriteParameterByComponentName(objectName);   //Does nothing unless this is a system

    GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(objectName);
    GUIModelObject* obj_ptr = it.value();

        //Remove connectors that are connected to the model object
    QList<GUIConnector *> pConnectorList = obj_ptr->getGUIConnectorPtrs();
    for(int i=0; i<pConnectorList.size(); ++i)
    {
        this->removeConnector(pConnectorList[i], undoSettings);
    }

    if (undoSettings == UNDO && !mUndoDisabled)
    {
        //Register removal of model object in undo stack
        this->mUndoStack->registerDeletedObject(it.value());
        //emit componentChanged(); //!< @todo Why do we need to emit this signal after regestering in undostack
        //qDebug() << "Emitting!";
    }


    if (it != mGUIModelObjectMap.end())
    {
        //! @todo maybe this should be handled somwhere else (not sure maybe this is the best place)
        if ((*it)->type() == GUICONTAINERPORT )
        {
            this->removeExternalPort((*it)->getName());
        }

        mGUIModelObjectMap.erase(it);
        mSelectedGUIModelObjectsList.removeOne(obj_ptr);
        mpScene->removeItem(obj_ptr);
        delete(obj_ptr);
    }
    else
    {
        //qDebug() << "In delete GUIObject: could not find object with name " << objectName;
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Could not delete object with name " + objectName + ", object not found");
    }
    emit checkMessages();
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief This function is used to rename a SubGUIObject
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
            case GUICONTAINERPORT : //!< @todo What will happen when we try to rename a groupport
                //qDebug() << "GUISYSTEMPORT";
                modNewName = this->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName);
                renameExternalPort(oldName, modNewName);
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


//! @brief Tells whether or not a component with specified name exist in the GraphicsView
bool GUIContainerObject::hasGUIModelObject(QString name)
{
    return (mGUIModelObjectMap.count(name) > 0);
}

//! @brief Takes ownership of supplied objects, widgets and connectors
//!
//! This method assumes that the previous owner have forgotten all about these objects, it however sets iself as new Qtparent, parentContainer and scene, overwriting the old values
void GUIContainerObject::takeOwnershipOf(QList<GUIModelObject*> &rModelObjectList, QList<GUIWidget*> &rWidgetList)
{
    for (int i=0; i<rModelObjectList.size(); ++i)
    {
        //! @todo if a containerport is received we must update the external port list also, we cant handle such objects right now
        if (rModelObjectList[i]->type() != GUICONTAINERPORT)
        {
            this->getContainedScenePtr()->addItem(rModelObjectList[i]);
            rModelObjectList[i]->setParentContainerObject(this);
            mGUIModelObjectMap.insert(rModelObjectList[i]->getName(), rModelObjectList[i]);
            //! @todo what if name already taken, dont care for now as we shal only move into groups when they are created

            //rModelObjectList[i]->refreshParentContainerSigSlotConnections();
        }
        else
        {
            //We cant handle this yet
            assert(false);
        }
    }

    for (int i=0; i<rWidgetList.size(); ++i)
    {
        this->getContainedScenePtr()->addItem(rWidgetList[i]);
        rWidgetList[i]->setParentContainerObject(this);
        mWidgetMap.insert(rWidgetList[i]->mWidgetIndex, rWidgetList[i]);
        //! @todo what if idx already taken, dont care for now as we shal only move into groups when they are created
    }

    QList<GUIConnector*> transitConnectors;
    QList<GUIConnector*> internalConnectors;

    //Determine what connectors are transitconnectors
    for (int i=0; i<rModelObjectList.size(); ++i)
    {
        GUIModelObject *pObj = rModelObjectList[i];

        QList<GUIConnector*> connectorPtrs = pObj->getGUIConnectorPtrs();
        for(int i=0; i<connectorPtrs.size(); ++i)
        {
            if((rModelObjectList.contains(connectorPtrs[i]->getStartPort()->getGuiModelObject())) &&
               (rModelObjectList.contains(connectorPtrs[i]->getEndPort()->getGuiModelObject())))
            {
                //This seems to be an internal connector, add to internal connector list, (if it has not already been added)
                if (!internalConnectors.contains(connectorPtrs[i]))
                {
                    internalConnectors.append(connectorPtrs[i]);
                }

            }
            else
            {
                //This seems to be a connector that connects to an external modelobject, lets create a transit connector
                if (!transitConnectors.contains(connectorPtrs[i]))
                {
                    transitConnectors.append(connectorPtrs[i]);

                    //! @todo for now we dissconnect the transit connection as we are noy yet capable of recreating the external connection
                    this->getCoreSystemAccessPtr()->disconnect(connectorPtrs[i]->getStartComponentName(),
                                                               connectorPtrs[i]->getStartPortName(),
                                                               connectorPtrs[i]->getEndComponentName(),
                                                               connectorPtrs[i]->getEndPortName());
                }
            }
        }
    }

    //! @todo move this code up into loop above
    //Add the internal connectors
    for (int i=0; i<internalConnectors.size(); ++i)
    {
        qDebug() << "___Adding internalConnection";
        //Make previous parent container forget about the connector
        internalConnectors[i]->getParentContainer()->forgetContainedConnector(internalConnectors[i]);

        //Make new container own and know about the connector
        this->getContainedScenePtr()->addItem(internalConnectors[i]);
        internalConnectors[i]->setParentContainer(this);
        mSubConnectorList.append(internalConnectors[i]);
    }

    //Add the transit connectors and create group ports
    for (int i=0; i<transitConnectors.size(); ++i)
    {
        qDebug() << "___Adding transitConnection";
        QPointF portpos;
        bool endPortIsTransitPort = false;
        if (rModelObjectList.contains(transitConnectors[i]->getStartPort()->getGuiModelObject()))
        {
            portpos = transitConnectors[i]->getEndPort()->getGuiModelObject()->getCenterPos();
            //Make end object forget about this connector as we are actually splitting it into two new connectors
            transitConnectors[i]->getEndPort()->getGuiModelObject()->forgetConnector(transitConnectors[i]);
            endPortIsTransitPort = true;
        }
        else
        {
            portpos = transitConnectors[i]->getStartPort()->getGuiModelObject()->getCenterPos();
            //Make start object forget about this connector as we are actually splitting it into two new connectors
            transitConnectors[i]->getStartPort()->getGuiModelObject()->forgetConnector(transitConnectors[i]);
        }

        //Create the "transit port"
        GUIModelObjectAppearance *portApp = gpMainWindow->mpLibrary->getAppearanceData(HOPSANGUICONTAINERPORTTYPENAME);
        GUIModelObject *pTransPort = this->addGUIModelObject(portApp, portpos.toPoint(),0);

        //Make previous parent container forget about the connector
        transitConnectors[i]->getParentContainer()->forgetContainedConnector(transitConnectors[i]);

        //Add the port to this container
        this->getContainedScenePtr()->addItem(transitConnectors[i]);
        transitConnectors[i]->setParentContainer(this);
        mSubConnectorList.append(transitConnectors[i]);

        //! @todo instead of having set startport and set end port, (we can keep them also maybe), we should have a function that sets startport if no port is set and end port if start port already set, dont know if good idea but we can keep it in mind, then we would not have to do stuff like bellow. (maybe we could call that function "connect")
        if (endPortIsTransitPort)
        {
            //Make new port and connector know about eachother
            transitConnectors[i]->setEndPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getEndPort()->getGuiModelObject()->rememberConnector(transitConnectors[i]);
        }
        else
        {
            //Make new port and connector know about eachother
            transitConnectors[i]->setStartPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getStartPort()->getGuiModelObject()->rememberConnector(transitConnectors[i]);
        }
    }

    //! @todo do much more stuff

    //! @todo center the objects in the new view

}


//! @brief Returns a pointer to the component with specified name.
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
    item = 0;
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


//! @brief Removes a specified connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GUIContainerObject::removeConnector(GUIConnector* pConnector, undoStatus undoSettings)
{
    bool doDelete = false;          //! @todo Why would we not want to delete the connector when we call the function that is meant to delete it?
    bool endPortWasConnected = false;       //Tells if connector is finished or being created

    if(undoSettings == UNDO)
    {
        mUndoStack->registerDeletedConnector(pConnector);
    }

    for(int i = 0; i < mSubConnectorList.size(); ++i)
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
             doDelete = true;       //Connector exists, so delete it?!
        }
        if(mSubConnectorList.empty())
        {
            break;
        }
    }


    //Show the end port if it exists and if it is no longer connected
    if(endPortWasConnected)
    {
        pConnector->getEndPort()->removeConnection();
        if(!pConnector->getEndPort()->isConnected())
        {
            pConnector->getEndPort()->setVisible(!mPortsHidden);
        }
    }

    //Show the start port if it is no longer connected
    pConnector->getStartPort()->removeConnection();
    if(!pConnector->getStartPort()->isConnected())
    {
        pConnector->getStartPort()->setVisible(!mPortsHidden);
    }

    //Delete the connector and remove it from scene and lists
    if(doDelete)
    {
        mSubConnectorList.removeAll(pConnector);
        mSelectedSubConnectorsList.removeAll(pConnector);
        mpScene->removeItem(pConnector);
        delete pConnector;
    }

    //Refresh the graphics view
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}



//! @brief Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
void GUIContainerObject::createConnector(GUIPort *pPort, undoStatus undoSettings)
{
        //When clicking start port (begin creation of connector)
    if (!getIsCreatingConnector())
    {
        mpTempConnector = new GUIConnector(pPort, this);
        deselectAll();
        setIsCreatingConnector(true);
        mpTempConnector->drawConnector();
    }
        //When clicking end port (finish creation of connector)
    else
    {

        bool success = false;

            //If we are connecting to group run special gui only check if connection OK
        if (pPort->getGuiModelObject()->type() == GUIGROUP)
        {
            //! @todo do this
        }
        else
        {
            success = this->getCoreSystemAccessPtr()->connect(mpTempConnector->getStartComponentName(), mpTempConnector->getStartPortName(), pPort->getGuiModelObjectName(), pPort->getName() );
        }

        if (success)
        {
            setIsCreatingConnector(false);
            pPort->getGuiModelObject()->rememberConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);
            mpTempConnector->finishCreation();
            mSubConnectorList.append(mpTempConnector);

            if(undoSettings == UNDO)
            {
                mUndoStack->newPost();
                mUndoStack->registerAddedConnector(mpTempConnector);
            }
            mpParentProjectTab->hasChanged();
        }
        emit checkMessages();
     }
}


//! @brief Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void GUIContainerObject::cutSelected(CopyStack *xmlStack)
{
    this->copySelected(xmlStack);
    this->mUndoStack->newPost("cut");
    emit deleteSelected();
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! @brief Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void GUIContainerObject::copySelected(CopyStack *xmlStack)
{
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

        //Store center point
    QPointF center = getCenterPointFromSelection();
    appendCoordinateTag(*copyRoot, center.x(), center.y());

        //Copy components
    QList<GUIModelObject *>::iterator it;
    for(it = mSelectedGUIModelObjectsList.begin(); it!=mSelectedGUIModelObjectsList.end(); ++it)
    {
        (*it)->saveToDomElement(*copyRoot);
    }

        //Copy connectors
    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->getStartPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->getEndPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->isActive())
        {
            mSubConnectorList[i]->saveToDomElement(*copyRoot);
        }
    }

        //Copy widgets
    //! @todo All widgets should probably use the same load/save functions. Then we could use the mSelectedWidgetList, and this would be much nicer.
    QList<GUIBoxWidget *>::iterator itbw;
    for(itbw = mBoxWidgetList.begin(); itbw!=mBoxWidgetList.end(); ++itbw)
    {
        if((*itbw)->isSelected())
        {
            (*itbw)->saveToDomElement(*copyRoot);
        }
    }
    QList<GUITextWidget *>::iterator ittw;
    for(ittw = mTextWidgetList.begin(); ittw!=mTextWidgetList.end(); ++ittw)
    {
        if((*ittw)->isSelected())
        {
            (*ittw)->saveToDomElement(*copyRoot);
        }
    }
}


//! @brief Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GUIContainerObject::paste(CopyStack *xmlStack)
{

    gpMainWindow->mpMessageWidget->printGUIDebugMessage(gCopyStack.getXML());

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

        //Determine paste offset (will paste components at mouse position
    QDomElement coordTag = copyRoot->firstChildElement(HMF_COORDINATETAG);
    double x, y;
    parseCoordinateTag(coordTag, x, y);
    QPointF oldCenter = QPointF(x, y);

    QCursor cursor;
    QPointF newCenter = mpParentProjectTab->mpGraphicsView->mapToScene(mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos()));

    double xOffset = newCenter.x() - oldCenter.x();
    double yOffset = newCenter.y() - oldCenter.y();

        //Paste components
    QDomElement objectElement = copyRoot->firstChildElement("component");
    while(!objectElement.isNull())
    {
        GUIModelObject *pObj = loadGUIModelObject(objectElement, gpMainWindow->mpLibrary, this);

            //Apply parameter values
        QDomElement xmlParameters = objectElement.firstChildElement(HMF_PARAMETERS);
        QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
        while (!xmlParameter.isNull())
        {
            loadParameterValue(xmlParameter, pObj);
            xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
        }

            //Apply start values
        QDomElement xmlStartValues = objectElement.firstChildElement(HMF_STARTVALUES);
        QDomElement xmlStartValue = xmlStartValues.firstChildElement(HMF_STARTVALUE);
        while (!xmlStartValue.isNull())
        {
            loadStartValue(xmlStartValue, pObj, NOUNDO);
            xmlStartValue = xmlStartValue.nextSiblingElement(HMF_STARTVALUE);
        }

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(xOffset, yOffset);
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
        tempConnector->moveAllPoints(xOffset, yOffset);
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
        loadTextWidget(textElement, this, NOUNDO);
        mTextWidgetList.last()->setSelected(true);
        mTextWidgetList.last()->moveBy(xOffset, yOffset);
        mUndoStack->registerAddedTextWidget(mTextWidgetList.last());
        textElement = textElement.nextSiblingElement("textwidget");
    }

        //Paste box widgets
    QDomElement boxElement = copyRoot->firstChildElement("boxwidget");
    while(!boxElement.isNull())
    {
        loadBoxWidget(boxElement, this, NOUNDO);
        mBoxWidgetList.last()->setSelected(true);
        mBoxWidgetList.last()->moveBy(xOffset, yOffset);
        mUndoStack->registerAddedBoxWidget(mBoxWidgetList.last());
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


//! @brief Aligns all selected objects vertically to the last selected object.
void GUIContainerObject::alignX()
{
    if(mSelectedGUIModelObjectsList.size() > 1)
    {
        mUndoStack->newPost("alignx");
        for(int i=0; i<mSelectedGUIModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedGUIModelObjectsList.at(i)->pos();
            mSelectedGUIModelObjectsList.at(i)->setCenterPos(QPointF(mSelectedGUIModelObjectsList.last()->getCenterPos().x(), mSelectedGUIModelObjectsList.at(i)->getCenterPos().y()));
            QPointF newPos = mSelectedGUIModelObjectsList.at(i)->pos();
            mUndoStack->registerMovedObject(oldPos, newPos, mSelectedGUIModelObjectsList.at(i)->getName());
        }
        mpParentProjectTab->hasChanged();
    }
}


//! @brief Aligns all selected objects horizontally to the last selected object.
void GUIContainerObject::alignY()
{
    if(mSelectedGUIModelObjectsList.size() > 1)
    {
        mUndoStack->newPost("aligny");
        for(int i=0; i<mSelectedGUIModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedGUIModelObjectsList.at(i)->pos();
            mSelectedGUIModelObjectsList.at(i)->setCenterPos(QPointF(mSelectedGUIModelObjectsList.at(i)->getCenterPos().x(), mSelectedGUIModelObjectsList.last()->getCenterPos().y()));
            QPointF newPos = mSelectedGUIModelObjectsList.at(i)->pos();
            mUndoStack->registerMovedObject(oldPos, newPos, mSelectedGUIModelObjectsList.at(i)->getName());
        }
        mpParentProjectTab->hasChanged();
    }
}


//! @brief Calculates the geometrical center position of the selected objects.
QPointF GUIContainerObject::getCenterPointFromSelection()
{
    double sumX = 0;
    double sumY = 0;
    int nSelected = 0;
    for(int i=0; i<mSelectedGUIModelObjectsList.size(); ++i)
    {
        sumX += mSelectedGUIModelObjectsList.at(i)->getCenterPos().x();
        sumY += mSelectedGUIModelObjectsList.at(i)->getCenterPos().y();
        ++nSelected;
    }
    for(int i=0; i<mSelectedGUIWidgetsList.size(); ++i)
    {
        sumX += mSelectedGUIWidgetsList.at(i)->getCenterPos().x();
        sumY += mSelectedGUIWidgetsList.at(i)->getCenterPos().y();
        ++nSelected;
    }

    return QPointF(sumX/nSelected, sumY/nSelected);
}


//! @brief Groups the selected objects together.
void GUIContainerObject::groupSelected(QPointF pt)
{
    gpMainWindow->mpMessageWidget->printGUIWarningMessage("Groups are not yet fully implemented, DO NOT use them, it will only end in tears!");
    qDebug() << "pos where we want to create group: " << pt;
    qDebug() << "In group selected";

    //Copy the selected objects, the lists will be cleared by addGuiobject and we need to keep this information
    QList<GUIModelObject*> modelObjects = mSelectedGUIModelObjectsList;
    QList<GUIWidget*> widgets = mSelectedGUIWidgetsList;

    //"Detach" the selected objects from this container, basically by removing pointers from the subobject storage maps, make this container forget aboout these objects
    for (int i=0; i<modelObjects.size(); ++i)
    {
        //! @todo if a containerport is selcted we need to remove it in core, not only from the storage vector, we must also make sure that the external ports are updated accordingly, for now we just ignore them (maybe we should allways ignore them when grouping)
        if (modelObjects[i]->type() != GUICONTAINERPORT)
        {
            mGUIModelObjectMap.remove(modelObjects[i]->getName());
        }
    }

    for (int i=0; i<widgets.size(); ++i)
    {
        mWidgetMap.remove(widgets[i]->mWidgetIndex);

        //temporary hack, to remove from widget lists
        GUITextWidget *pTextWidget = qobject_cast<GUITextWidget*>(widgets[i]);
        if (pTextWidget != 0)
        {
            mTextWidgetList.removeAll(pTextWidget);
        }
        GUIBoxWidget *pBoxWidget = qobject_cast<GUIBoxWidget*>(widgets[i]);
        if (pBoxWidget != 0)
        {
            mBoxWidgetList.removeAll(pBoxWidget);
        }
    }

    //Create a new group at the location of the specified
    GUIModelObjectAppearance* pAppdata = gpMainWindow->mpLibrary->getAppearanceData("HopsanGUIGroup");
    GUIModelObject* pObj =  this->addGUIModelObject(pAppdata, pt.toPoint(),0);
    GUIContainerObject* pContainer =  qobject_cast<GUIContainerObject*>(pObj);

    //If dyncast sucessfull (it should allways be) then let new group take ownership of objects
    if (pContainer != 0)
    {
        pContainer->takeOwnershipOf(modelObjects, widgets);
    }
    else
    {
        assert(false);
    }
}


//! @brief Selects all objects and connectors.
void GUIContainerObject::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllGUIConnectors();
}


//! @brief Deselects all objects and connectors.
void GUIContainerObject::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();
}


//! @brief Hides all component names.
//! @see showNames()
void GUIContainerObject::hideNames()
{
    mUndoStack->newPost("hideallnames");
    mIsRenamingObject = false;
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! @brief Shows all component names.
//! @see hideNames()
void GUIContainerObject::showNames()
{
    mUndoStack->newPost("showallnames");
    emit showAllNameText();
}


//! @brief Toggles name text on or off
//! @see showNames();
//! @see hideNames();
void GUIContainerObject::toggleNames(bool value)
{
    if(value)
    {
        emit showAllNameText();
    }
    else
    {
        emit hideAllNameText();
    }
    mNamesHidden = !value;
    mpParentProjectTab->hasChanged();
}


//! @brief Slot that sets hide ports flag to true or false
void GUIContainerObject::hidePorts(bool doIt)
{
    mPortsHidden = !doIt;
    mpParentProjectTab->hasChanged();
}


//! @brief Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void GUIContainerObject::undo()
{
    mUndoStack->undoOneStep();
}


//! @brief Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void GUIContainerObject::redo()
{
    mUndoStack->redoOneStep();
}

//! @brief Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void GUIContainerObject::clearUndo()
{
    mUndoStack->clear();
}


//! @brief Returns true if at least one GUIObject is selected
bool GUIContainerObject::isObjectSelected()
{
    return (mSelectedGUIModelObjectsList.size() > 0);
}


//! @brief Returns true if at least one GUIConnector is selected
bool GUIContainerObject::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}


void GUIContainerObject::setScriptFile(QString path)
{
    mScriptFilePath = path;
}


QString GUIContainerObject::getScriptFile()
{
    return mScriptFilePath;
}


//! @brief Returns the path to the icon with user graphics.
QString GUIContainerObject::getUserIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathUser();
}


//! @brief Returns the path to the icon with iso graphics.
//! @todo do we return full path or relative
QString GUIContainerObject::getIsoIconPath()
{
    return this->mGUIModelObjectAppearance.getIconPathISO();
}


//! @brief Sets the path to the icon with user graphics.
//! @todo do we safe full path or relative
void GUIContainerObject::setUserIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathUser(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}


//! @brief Sets the path to the icon with iso graphics.
void GUIContainerObject::setIsoIconPath(QString path)
{
    QFileInfo fi;
    fi.setFile(path);
    this->mGUIModelObjectAppearance.setIconPathISO(fi.fileName());
    this->mGUIModelObjectAppearance.setBaseIconPath(fi.absolutePath()+"/");
}


//! @brief Access function for mIsCreatingConnector
//! @param isConnected is the new value
void GUIContainerObject::setIsCreatingConnector(bool isCreatingConnector)
{
    mIsCreatingConnector = isCreatingConnector;
}


//! @brief Access function for mIsCreatingConnector
bool GUIContainerObject::getIsCreatingConnector()
{
    return mIsCreatingConnector;
}

//! @brief This is a helpfunction that can be used to make a container "forget" about a certain connector
//!
//! It does not delete the connector and connected components dos not forget about it
//! use only when transfering ownership of objects to an other container
void GUIContainerObject::forgetContainedConnector(GUIConnector *pConnector)
{
    mSubConnectorList.removeAll(pConnector);
}


//! @brief Disables the undo function for the current model
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


//! @brief Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
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

//! @brief Sets the iso graphics option for the model
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


//! @brief Slot that tells all selected name texts to deselect themselves
void GUIContainerObject::deselectSelectedNameText()
{
    emit deselectAllNameText();
}


//! @brief Defines the right click menu for container objects.
//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUIContainerObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    QAction *pAction = this->buildBaseContextMenu(menu, event);
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

    //Dont call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
    //GUIModelObject::contextMenuEvent(event);
    ////QGraphicsItem::contextMenuEvent(event);
}


//! @brief Defines the double click event for container objects (used to enter containers).
void GUIContainerObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    GUIModelObject::mouseDoubleClickEvent(event);
    this->enterContainer();
}


//! @brief Opens the properites dialog for container objects.
void GUIContainerObject::openPropertiesDialog()
{
    //Do Nothing
}


//! @brief Clears all of the contained objects (and delets them).
//! This code cant be run in the desturctor as this wold cause wired behaviour in the derived susyem class.
//! The core system would be deleted before container clear code is run, that is why we have it as a convenient protected function
void GUIContainerObject::clearContents()
{
    GUIModelObjectMapT::iterator mit;
    QMap<size_t, GUIWidget *>::iterator wit;

    qDebug() << "Clearing model objects in " << getName();
    //We cant use for loop over iterators as the maps are modified on each delete (and iterators invalidated)
    mit=mGUIModelObjectMap.begin();
    while (mit!=mGUIModelObjectMap.end())
    {
        //! @todo calling deleteMe in this destructor will probably leed to a delete including undo registration which we should avoid in this case, same for widgets bellow
        (*mit)->deleteMe();
        mit=mGUIModelObjectMap.begin();
    }

    qDebug() << "Clearing widget objects in " << getName();
    wit=mWidgetMap.begin();
    while (wit!=mWidgetMap.end())
    {
        (*wit)->deleteMe();
        wit=mWidgetMap.begin();
    }
}


//! @brief Enters a container object and maks the view represent it contents.
void GUIContainerObject::enterContainer()
{
    //First deselect everything so that buttons pressed in the view are not sent to obejcts in the previous container
    //this->deselectAll(); //! @todo maybe this should be a signal
    //! @todo there is apperantly a deselect all guiwidgets also that is not in deselect all, has nothing to do with this code though
    //this->deselect();
    mpParentContainerObject->deselectAll(); //deselect myself and anyone else

    //Show this scene
    mpParentProjectTab->mpGraphicsView->setScene(getContainedScenePtr());
    mpParentProjectTab->mpGraphicsView->setContainerPtr(this);
    mpParentProjectTab->mpQuickNavigationWidget->addOpenContainer(this);

        //Disconnect parent system and connect new system with actions
//    disconnect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(hideNames()));
//    disconnect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(showNames()));
//    disconnect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        mpParentContainerObject,     SLOT(disableUndo()));
//    disconnect(gpMainWindow->cutAction,            SIGNAL(triggered()),        mpParentContainerObject,     SLOT(cutSelected()));
//    disconnect(gpMainWindow->copyAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(copySelected()));
//    disconnect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        mpParentContainerObject,     SLOT(paste()));
//    disconnect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        mpParentContainerObject,     SLOT(openPropertiesDialogSlot()));
//    disconnect(gpMainWindow->undoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(undo()));
//    disconnect(gpMainWindow->redoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(redo()));
      mpParentContainerObject->disconnectMainWindowActions();

//    connect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        this,     SLOT(hideNames()));
//    connect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        this,     SLOT(showNames()));
//    connect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        this,     SLOT(disableUndo()));
//    connect(gpMainWindow->cutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()));
//    connect(gpMainWindow->copyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()));
//    connect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()));
//    connect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()));
//    connect(gpMainWindow->undoAction,           SIGNAL(triggered()),        this,     SLOT(undo()));
//    connect(gpMainWindow->redoAction,           SIGNAL(triggered()),        this,     SLOT(redo()));
      this->connectMainWindowActions();

        //Upddate plot widget and undo widget to new container
    gpMainWindow->makeSurePlotWidgetIsCreated();
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();
    gpMainWindow->mpUndoWidget->refreshList();
    gpMainWindow->undoAction->setDisabled(this->mUndoDisabled);
    gpMainWindow->redoAction->setDisabled(this->mUndoDisabled);
}

//! @brief Exit a container object and maks its the view represent its parents contents.
void GUIContainerObject::exitContainer()
{
    this->deselectAll();
    //Go back to parent system
    mpParentProjectTab->mpGraphicsView->setScene(this->mpParentContainerObject->getContainedScenePtr());
    mpParentProjectTab->mpGraphicsView->setContainerPtr(this->mpParentContainerObject);

        //Disconnect this system and connect parent system with undo and redo actions
//    disconnect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        this,     SLOT(hideNames()));
//    disconnect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        this,     SLOT(showNames()));
//    disconnect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        this,     SLOT(disableUndo()));
//    disconnect(gpMainWindow->cutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()));
//    disconnect(gpMainWindow->copyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()));
//    disconnect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()));
//    disconnect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()));
//    disconnect(gpMainWindow->undoAction,           SIGNAL(triggered()),        this,     SLOT(undo()));
//    disconnect(gpMainWindow->redoAction,           SIGNAL(triggered()),        this,     SLOT(redo()));

//    connect(gpMainWindow->hideNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(hideNames()));
//    connect(gpMainWindow->showNamesAction,      SIGNAL(triggered()),        mpParentContainerObject,     SLOT(showNames()));
    connect(gpMainWindow->toggleNamesAction,    SIGNAL(triggered(bool)),    mpParentContainerObject,     SLOT(toggleNames(bool)));
    connect(gpMainWindow->disableUndoAction,    SIGNAL(triggered()),        mpParentContainerObject,     SLOT(disableUndo()));
    connect(gpMainWindow->cutAction,            SIGNAL(triggered()),        mpParentContainerObject,     SLOT(cutSelected()));
    connect(gpMainWindow->copyAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(copySelected()));
    connect(gpMainWindow->alignXAction,         SIGNAL(triggered()),        mpParentContainerObject,     SLOT(alignX()));
    connect(gpMainWindow->alignYAction,         SIGNAL(triggered()),        mpParentContainerObject,     SLOT(alignY()));
    connect(gpMainWindow->rotateRightAction,    SIGNAL(triggered()),        mpParentContainerObject,     SLOT(rotateRight()));
    connect(gpMainWindow->rotateLeftAction,     SIGNAL(triggered()),        mpParentContainerObject,     SLOT(rotateLeft()));
    connect(gpMainWindow->flipHorizontalAction, SIGNAL(triggered()),        mpParentContainerObject,     SLOT(flipHorizontal()));
    connect(gpMainWindow->flipVerticalAction,   SIGNAL(triggered()),        mpParentContainerObject,     SLOT(flipVertical()));
    connect(gpMainWindow->pasteAction,          SIGNAL(triggered()),        mpParentContainerObject,     SLOT(paste()));
    connect(gpMainWindow->propertiesAction,     SIGNAL(triggered()),        mpParentContainerObject,     SLOT(openPropertiesDialogSlot()));
    connect(gpMainWindow->undoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(undo()));
    connect(gpMainWindow->redoAction,           SIGNAL(triggered()),        mpParentContainerObject,     SLOT(redo()));

        //Update plot widget and undo widget to new container
    gpMainWindow->makeSurePlotWidgetIsCreated();
    gpMainWindow->mpPlotWidget->mpPlotParameterTree->updateList();
    gpMainWindow->mpUndoWidget->refreshList();
    gpMainWindow->undoAction->setDisabled(mpParentContainerObject->mUndoDisabled);
    gpMainWindow->redoAction->setDisabled(mpParentContainerObject->mUndoDisabled);

        //Refresh external port appearance
    //! @todo We only need to do this if ports have change, right now we always refresh, dont know if this is a big deal
    this->refreshExternalPortsAppearanceAndPosition();
}


//! @brief Rotates all selected objects right (clockwise)
void GUIContainerObject::rotateRight()
{
    if(this->isObjectSelected())
    {
        mUndoStack->newPost();
        mpParentProjectTab->hasChanged();
    }
    emit rotateSelectedObjectsRight();
}


//! @brief Rotates all selected objects left (counter-clockwise)
void GUIContainerObject::rotateLeft()
{
    if(this->isObjectSelected())
    {
        mUndoStack->newPost();
        mpParentProjectTab->hasChanged();
    }
    emit rotateSelectedObjectsLeft();
}


void GUIContainerObject::flipHorizontal()
{
    if(this->isObjectSelected())
    {
        mUndoStack->newPost();
        mpParentProjectTab->hasChanged();
    }
    emit flipSelectedObjectsHorizontal();
}


void GUIContainerObject::flipVertical()
{
    if(this->isObjectSelected())
    {
        mUndoStack->newPost();
        mpParentProjectTab->hasChanged();
    }
    emit flipSelectedObjectsVertical();
}


//! @brief Collects the plot data from the last simulation for all plot variables from the core and stores them locally.
void GUIContainerObject::collectPlotData()
{
    bool timeVectorObtained = false;

    GUIModelObjectMapT::iterator moit;
    QList<GUIPort*>::iterator pit;
    QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > componentMap;
    for(moit=mGUIModelObjectMap.begin(); moit!=mGUIModelObjectMap.end(); ++moit)
    {
        QMap< QString, QMap<QString, QVector<double> > > portMap;
        for(pit=moit.value()->getPortListPtrs().begin(); pit!=moit.value()->getPortListPtrs().end(); ++pit)
        {
            QMap<QString, QVector<double> > variableMap;

            QVector<QString> names;
            QVector<QString> units;
            getCoreSystemAccessPtr()->getPlotDataNamesAndUnits(moit.value()->getName(), (*pit)->getName(), names, units);

            QVector<QString>::iterator nit;
            for(nit=names.begin(); nit!=names.end(); ++nit)
            {
                QVector<double> data;
                getCoreSystemAccessPtr()->getPlotData(moit.value()->getName(), (*pit)->getName(), (*nit), data);
                variableMap.insert((*nit), data);
                if(!timeVectorObtained)
                {
                    mTimeVectors.append(QVector<double>::fromStdVector(getCoreSystemAccessPtr()->getTimeVector(moit.value()->getName(), (*pit)->getName())));
                    timeVectorObtained = true;
                }

                //qDebug() << "Inserting: " << moit.value()->getName() << ", " << (*pit)->getName() << ", " << (*nit);
            }
            portMap.insert((*pit)->getName(), variableMap);
        }
        componentMap.insert(moit.value()->getName(), portMap);
    }
    mPlotData.append(componentMap);
}


QVector<double> GUIContainerObject::getTimeVector(int generation)
{
    return mTimeVectors.at(generation);
}


QVector<double> GUIContainerObject::getPlotData(int generation, QString componentName, QString portName, QString dataName)
{
    qDebug() << "Looking for " << generation << ", " << componentName << ", " << portName << ", " << dataName;
    qDebug() << "Size of data: " << mPlotData.size();
    return mPlotData.at(generation).find(componentName).value().find(portName).value().find(dataName).value();
}


QList< QMap< QString, QMap< QString, QMap<QString, QVector<double> > > > > GUIContainerObject::getAllPlotData()
{
    return mPlotData;
}


int GUIContainerObject::getNumberOfPlotGenerations()
{
    return mPlotData.size();
}
