/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   GUIContainerObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Container class (base class for Systems and Groups)
//!
//$Id$

//Qt/other includes
#include <limits>
#include <QDomElement>
#include <QStandardItemModel>
#include <QToolBar>
#include "qfile.h"

//Hopsan includes
#include "Configuration.h"
#include "CopyStack.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIComponent.h"
#include "GUIConnector.h"
#include "GUIContainerObject.h"
#include "GUIContainerPort.h"
#include "GUIGroup.h"
#include "GUIPort.h"
#include "GUISystem.h"
#include "GUIWidgets.h"
#include "loadFunctions.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "version_gui.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/QuickNavigationWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/DataExplorer.h"



//! @brief Construtor for container objects.
//! @param position Initial position where container object is to be placed in its parent container
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to the appearance data object
//! @param startSelected Tells whether or not the object is initially selected
//! @param gfxType Tells whether the initial graphics shall be user or ISO
//! @param pParentContainer Pointer to the parent container object (leave empty if not a sub container)
//! @param pParent Pointer to parent object
ContainerObject::ContainerObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParent)
{
        //Initialize
    mIsCreatingConnector = false;
    mSubComponentPortsHidden = !gpMainWindow->mpTogglePortsAction->isChecked();
    mSubComponentNamesHidden = !gpMainWindow->mpToggleNamesAction->isChecked();
    mLossesVisible = false;
    mUndoDisabled = false;
    mGfxType = UserGraphics;

    mHighestWidgetIndex = 0;

    mPasteOffset = -30;

    //Create the scene
    mpScene = new QGraphicsScene(this);

    //Create the undastack
    mpUndoStack = new UndoStack(this);
    mpUndoStack->clear();

    mpDragCopyStack = new CopyStack();

    mpLogDataHandler = new LogDataHandler(this);

    //Establish connections that should always remain
    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpTerminalWidget, SLOT(checkMessages()), Qt::UniqueConnection);

}


//! @brief Destructor for container object
ContainerObject::~ContainerObject()
{
    qDebug() << ",,,,,,,,,,,,GUIContainer destructor";
}

//! @brief Notify the parent project tab that changes has occured
void ContainerObject::hasChanged()
{
    mpModelWidget->hasChanged();
}

//! @brief Connects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are swithching what continer we want to the buttons to trigger actions in
void ContainerObject::makeMainWindowConnectionsAndRefresh()
{
    connect(gpMainWindow->mpUndoAction, SIGNAL(triggered()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRedoAction, SIGNAL(triggered()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getUndoButton(),  SIGNAL(clicked()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getRedoButton(),  SIGNAL(clicked()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()), Qt::UniqueConnection);

    connect(gpMainWindow->mpTogglePortsAction,    SIGNAL(triggered(bool)),    this,     SLOT(showSubcomponentPorts(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleNamesAction,    SIGNAL(triggered(bool)),    this,     SLOT(toggleNames(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleSignalsAction,  SIGNAL(triggered(bool)),    this,     SLOT(toggleSignals(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpDisableUndoAction,    SIGNAL(triggered(bool)),    this,     SLOT(setUndoDisabled(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpCutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()), Qt::UniqueConnection);
    connect(gpMainWindow->mpCopyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()), Qt::UniqueConnection);
    connect(gpMainWindow->mpPasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()), Qt::UniqueConnection);
    connect(gpMainWindow->mpAlignXAction,         SIGNAL(triggered()),        this,     SLOT(alignX()), Qt::UniqueConnection);
    connect(gpMainWindow->mpAlignYAction,         SIGNAL(triggered()),        this,     SLOT(alignY()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRotateRightAction,    SIGNAL(triggered()),        this,     SLOT(rotateSubObjects90cw()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRotateLeftAction,     SIGNAL(triggered()),        this,     SLOT(rotateSubObjects90ccw()), Qt::UniqueConnection);
    connect(gpMainWindow->mpFlipHorizontalAction, SIGNAL(triggered()),        this,     SLOT(flipSubObjectsHorizontal()), Qt::UniqueConnection);
    connect(gpMainWindow->mpFlipVerticalAction,   SIGNAL(triggered()),        this,     SLOT(flipSubObjectsVertical()), Qt::UniqueConnection);
    connect(gpMainWindow->mpPropertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()), Qt::UniqueConnection);

    // Update the main window toolbar action buttons that are system specific
    gpMainWindow->mpTogglePortsAction->setChecked(!mSubComponentPortsHidden);
    gpMainWindow->mpToggleNamesAction->setChecked(!mSubComponentNamesHidden);
    gpMainWindow->mpToggleSignalsAction->setChecked(!mSignalsHidden);

    // Update main window widgets with data from this container
    gpMainWindow->mpDataExplorer->setLogdataHandler(this->mpLogDataHandler);
    gpMainWindow->mpPlotWidget->mpPlotVariableTree->setLogDataHandler(this->getLogDataHandler());
    gpMainWindow->mpSystemParametersWidget->update(this);
    gpMainWindow->mpUndoWidget->refreshList();
    gpMainWindow->mpUndoAction->setDisabled(this->mUndoDisabled);
    gpMainWindow->mpRedoAction->setDisabled(this->mUndoDisabled);
}

//! @brief Disconnects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are swithching what continer we want to the buttons to trigger actions in
void ContainerObject::unmakeMainWindowConnectionsAndRefresh()
{
    // Update Systemparameter widget to have no contents
    gpMainWindow->mpSystemParametersWidget->update(0);
    gpMainWindow->mpPlotWidget->mpPlotVariableTree->setLogDataHandler(0);

    disconnect(gpMainWindow->mpUndoAction, SIGNAL(triggered()), this, SLOT(undo()));
    disconnect(gpMainWindow->mpRedoAction, SIGNAL(triggered()), this, SLOT(redo()));
    disconnect(gpMainWindow->mpUndoWidget->getUndoButton(),  SIGNAL(clicked()), this, SLOT(undo()));
    disconnect(gpMainWindow->mpUndoWidget->getRedoButton(),  SIGNAL(clicked()), this, SLOT(redo()));
    disconnect(gpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()));

    disconnect(gpMainWindow->mpToggleNamesAction,     SIGNAL(triggered(bool)),    this,    SLOT(toggleNames(bool)));
    disconnect(gpMainWindow->mpTogglePortsAction,     SIGNAL(triggered(bool)),    this,    SLOT(showSubcomponentPorts(bool)));
    disconnect(gpMainWindow->mpToggleSignalsAction,   SIGNAL(triggered(bool)),    this,    SLOT(toggleSignals(bool)));
    disconnect(gpMainWindow->mpDisableUndoAction,     SIGNAL(triggered(bool)),    this,    SLOT(setUndoDisabled(bool)));
    disconnect(gpMainWindow->mpCutAction,             SIGNAL(triggered()),        this,    SLOT(cutSelected()));
    disconnect(gpMainWindow->mpCopyAction,            SIGNAL(triggered()),        this,    SLOT(copySelected()));
    disconnect(gpMainWindow->mpPasteAction,           SIGNAL(triggered()),        this,    SLOT(paste()));
    disconnect(gpMainWindow->mpAlignXAction,          SIGNAL(triggered()),        this,    SLOT(alignX()));
    disconnect(gpMainWindow->mpAlignYAction,          SIGNAL(triggered()),        this,    SLOT(alignY()));
    disconnect(gpMainWindow->mpRotateRightAction,     SIGNAL(triggered()),        this,    SLOT(rotateSubObjects90cw()));
    disconnect(gpMainWindow->mpRotateLeftAction,      SIGNAL(triggered()),        this,    SLOT(rotateSubObjects90ccw()));
    disconnect(gpMainWindow->mpFlipHorizontalAction,  SIGNAL(triggered()),        this,    SLOT(flipSubObjectsHorizontal()));
    disconnect(gpMainWindow->mpFlipVerticalAction,    SIGNAL(triggered()),        this,    SLOT(flipSubObjectsVertical()));
    disconnect(gpMainWindow->mpPropertiesAction,      SIGNAL(triggered()),        this,    SLOT(openPropertiesDialogSlot()));
}

//! @brief A helpfunction that determines on which edge an external port should be placed based on its internal position
//! @param[in] center The center point of all objects to be compared with
//! @param[in] pt The position of this object, used to determine the center relative posistion
//! @returns An enum that indicates on which side the port should be placed
ContainerObject::ContainerEdgeEnumT ContainerObject::findPortEdge(QPointF center, QPointF pt)
{
    //By swapping place of pt1 and pt2 we get the angle in the same coordinate system as the view
    QPointF diff = pt-center;
    //qDebug() << "=============The Diff: " << diff;

    //If only one sysport default to left side
    //! @todo Do this smarter later and take into account port orientation, or position relative all other components, need to extend this function a bit for that though
    if (diff.manhattanLength() < 1.0)
    {
        return LeftEdge;
    }

    //Determine on what edge the port should be placed based on the angle from the center point
    qreal angle = normRad(qAtan2(diff.x(), diff.y()));
    //qDebug() << "angle: " << rad2deg(angle);
    if (fabs(angle) <= M_PI_4)
    {
        return RightEdge;
    }
    else if (fabs(angle) >= 3.0*M_PI_4)
    {
        return LeftEdge;
    }
    else if (angle > M_PI_4)
    {
        return BottomEdge;
    }
    else
    {
        return TopEdge;
    }
}

//! @brief Refreshes the appearance and postion of all external ports
void ContainerObject::refreshExternalPortsAppearanceAndPosition()
{
    //refresh the external port poses
    ModelObjectMapT::iterator moit;
    double val;

    //Set the initial values to be overwriten by the if bellow
    double xMin=std::numeric_limits<double>::max(), xMax=-xMin, yMin=xMin, yMax=xMax;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
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

    QMap<qreal, Port*> leftEdge, rightEdge, topEdge, bottomEdge;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == ContainerPortType)
        {
            //            QLineF line = QLineF(center, moit.value()->getCenterPos());
            //            this->getContainedScenePtr()->addLine(line); //debug-grej

            ContainerEdgeEnumT edge = findPortEdge(center, moit.value()->getCenterPos());
            //qDebug() << " sysp: " << moit.value()->getName() << " edge: " << edge;

            //Make sure we dont screw up in the code and forget to rename or create external ports on internal rename or create
            assert(this->getPort(moit.value()->getName()) != 0);

            //We insert into maps for automatic sorting based on x or y position as key value
            Port* pPort = this->getPort(moit.value()->getName());
            if(pPort->isAutoPlaced()) //Do not place if autoplaced is not set. Maybe a bit ugly to put an if statement here?
            {
                switch (edge) {
                case RightEdge:
                    rightEdge.insertMulti(moit.value()->getCenterPos().y(), pPort);
                    break;
                case BottomEdge:
                    bottomEdge.insertMulti(moit.value()->getCenterPos().x(), pPort);
                    break;
                case LeftEdge:
                    leftEdge.insertMulti(moit.value()->getCenterPos().y(), pPort);
                    break;
                case TopEdge:
                    topEdge.insertMulti(moit.value()->getCenterPos().x(), pPort);
                    break;
                }
            }
            else
            {
                //! @todo it would be nice if a port pos could be set directly from appearance data with help function
                PortAppearanceMapT::iterator pamit = mModelObjectAppearance.getPortAppearanceMap().find(pPort->getName());
                pPort->setCenterPosByFraction(pamit.value().x, pamit.value().y);
                pPort->setRotation(pamit.value().rot);
                this->createRefreshExternalPort(moit.value()->getName()); //Refresh for ports that are not autoplaced
            }
        }
    }

    //Now disperse the port icons evenly along each edge
    QMap<qreal, Port*>::iterator it;
    qreal disp;  //Dispersion factor
    qreal sdisp; //sumofdispersionfactors

    //! @todo weird to use createfunction to refresh graphics, but ok for now
    disp = 1.0/((qreal)(rightEdge.size()+1));
    sdisp=disp;
    for (it=rightEdge.begin(); it!=rightEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(1.0, sdisp);
        it.value()->setRotation(0);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(bottomEdge.size()+1));
    sdisp=disp;
    for (it=bottomEdge.begin(); it!=bottomEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(sdisp, 1.0);
        it.value()->setRotation(90);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(leftEdge.size()+1));
    sdisp=disp;
    for (it=leftEdge.begin(); it!=leftEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(0.0, sdisp);
        it.value()->setRotation(180);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((qreal)(topEdge.size()+1));
    sdisp=disp;
    for (it=topEdge.begin(); it!=topEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(sdisp, 0.0);
        it.value()->setRotation(270);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }
}

//! @brief Overloaded refreshAppearance for containers, to make sure that port positions are updeted if graphics size is changed
void ContainerObject::refreshAppearance()
{
    ModelObject::refreshAppearance();
    this->refreshExternalPortsAppearanceAndPosition();
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h heigth of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void ContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
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
CoreSystemAccess *ContainerObject::getCoreSystemAccessPtr()
{
    //Should be overloaded
    return 0;
}


//! @brief Retunrs a pointer to the contained scene
QGraphicsScene *ContainerObject::getContainedScenePtr()
{
    return this->mpScene;
}


//! @brief This method creates ONE external port. Or refreshes existing ports. It assumes that port appearance information for this port exists
//! @param[portName] The name of the port to create
//! @todo maybe defualt create that info if it is missing
//! @todo massive duplicate implementation with the one in modelobject
Port *ContainerObject::createRefreshExternalPort(QString portName)
{
    //If port appearance is not already existing then we create it
    if ( mModelObjectAppearance.getPortAppearanceMap().count(portName) == 0 )
    {
        mModelObjectAppearance.addPortAppearance(portName);
    }

    //Fetch appearance data
    PortAppearanceMapT::iterator it = mModelObjectAppearance.getPortAppearanceMap().find(portName);

    //Create new external port if it does not already exist (this is the usual case for individual components)
    Port *pPort = this->getPort(it.key());
    if ( pPort == 0 )
    {
        qDebug() << "##This is OK though as this means that we should create the stupid port for the first time";

        //! @todo to minimaze search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
        QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        qreal x = it.value().x;
        qreal y = it.value().y;
        qDebug() << "x,y: " << x << " " << y;

        if (this->type() == GroupContainerType)
        {
            pPort = new GroupPort(it.key(), x*boundingRect().width(), y*boundingRect().height(), &(it.value()), this);
        }
        else
        {
            pPort = new Port(it.key(), x*boundingRect().width(), y*boundingRect().height(), &(it.value()), this);
        }


        mPortListPtrs.append(pPort);

        pPort->refreshPortGraphics();
    }
    else
    {

        // The external port already seems to exist, lets update it incase something has changed
        //! @todo Maybe need to have a refresh portappearance function, dont really know if this will ever be used though, will fix when it becomes necessary
        pPort->refreshPortGraphics();

        // In this case of container object, also refresh any attached connectors, if types have changed
        //! @todo we allways update, maybe we should be more smart and only update if changed, but I think this should be handled inside the connector class (the smartness)
        QVector<Connector*> connectors = pPort->getAttachedConnectorPtrs();
        for (int i=0; i<connectors.size(); ++i)
        {
            connectors[i]->refreshConnectorAppearance();
        }

        qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << it.key() << " in: " << this->getName();
    }

    return pPort;
}




//! @brief Reanmes an external GUIPort
//! @param[in] oldName The name to be replaced
//! @param[in] newName The new name
//! This function assumes that oldName exist and that newName is correct, no error checking is done
void ContainerObject::renameExternalPort(const QString oldName, const QString newName)
{
    QList<Port*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == oldName )
        {
            //Rename the port appearance data by remove and re-add
            PortAppearance tmp = mModelObjectAppearance.getPortAppearanceMap().value(oldName);
            mModelObjectAppearance.erasePortAppearance(oldName);
            mModelObjectAppearance.addPortAppearance(newName, &tmp);

            //Rename port
            (*plit)->setDisplayName(newName);
            break;
        }
    }
}


//! @brief Helper function that allows calling addGUIModelObject with typeName instead of appearance data
ModelObject* ContainerObject::addModelObject(QString fullTypeName, QPointF position, qreal rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
    ModelObjectAppearance *pAppearanceData = gpMainWindow->mpLibrary->getAppearanceData(fullTypeName);
    if(!pAppearanceData)    //Not an existing component
        return 0;       //No error message here, it depends on from where this function is called
    else
        return addModelObject(pAppearanceData, position, rotation, startSelected, nameStatus, undoSettings);
}


//! @brief Creates and adds a GuiModel Object to the current container
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
//! @returns a pointer to the created and added object
//! @todo only modelobjects for now
ModelObject* ContainerObject::addModelObject(ModelObjectAppearance *pAppearanceData, QPointF position, qreal rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
        //Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();

    QString componentTypeName = pAppearanceData->getTypeName();
    if (componentTypeName == HOPSANGUISYSTEMTYPENAME)
    {
        mpTempGUIModelObject= new SystemContainer(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }
    else if (componentTypeName == HOPSANGUICONTAINERPORTTYPENAME)
    {
        // We must create internal port FIRST before external one
        mpTempGUIModelObject = new ContainerPort(position, rotation, pAppearanceData, this, startSelected, mGfxType);
        this->addExternalContainerPortObject(mpTempGUIModelObject);
        this->refreshExternalPortsAppearanceAndPosition();
    }
    else if (componentTypeName == HOPSANGUIGROUPTYPENAME)
    {
        mpTempGUIModelObject = new GroupContainer(position, rotation, pAppearanceData, this);
    }
    else if (componentTypeName == HOPSANGUISCOPECOMPONENTTYPENAME)
    {
        mpTempGUIModelObject = new ScopeComponent(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }
    else //Assume some standard component type
    {
        mpTempGUIModelObject = new Component(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }

    emit checkMessages();

    if ( mModelObjectMap.contains(mpTempGUIModelObject->getName()) )
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Trying to add component with name: " + mpTempGUIModelObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
        //! @todo Is this check really necessary? Two objects cannot have the same name anyway...
    }
    else
    {
        mModelObjectMap.insert(mpTempGUIModelObject->getName(), mpTempGUIModelObject);
    }

    if(undoSettings == Undo)
    {
        mpUndoStack->registerAddedObject(mpTempGUIModelObject);
    }

    mpTempGUIModelObject->setSelected(false);
    mpTempGUIModelObject->setSelected(true);

    if(nameStatus == NameVisible)
    {
        mpTempGUIModelObject->showName(NoUndo);
    }
    else if(nameStatus == NameNotVisible)
    {
        mpTempGUIModelObject->hideName(NoUndo);
    }

    return mpTempGUIModelObject;
}


bool ContainerObject::areLossesVisible()
{
    return mLossesVisible;
}


TextBoxWidget *ContainerObject::addTextBoxWidget(QPointF position, UndoStatusEnumT undoSettings)
{
    TextBoxWidget *pTempTextBoxWidget;
    pTempTextBoxWidget = new TextBoxWidget("Text", position, 0, Deselected, this, mHighestWidgetIndex);
    qDebug() << "Creating widget, index = " << pTempTextBoxWidget->getWidgetIndex();
    mWidgetMap.insert(mHighestWidgetIndex, pTempTextBoxWidget);
    qDebug() << "Inserting widget in map, index = " << mHighestWidgetIndex;
    ++mHighestWidgetIndex;
    if(undoSettings == Undo)
    {
        mpUndoStack->registerAddedWidget(pTempTextBoxWidget);
    }
    mpModelWidget->hasChanged();

    return pTempTextBoxWidget;
}


//! @brief Removes specified widget
//! Works for both text and box widgets
//! @param pWidget Pointer to widget to remove
//! @param undoSettings Tells whether or not this shall be registered in undo stack
void ContainerObject::deleteWidget(Widget *pWidget, UndoStatusEnumT undoSettings)
{
    if(undoSettings == Undo)
    {
        mpUndoStack->newPost();
        mpUndoStack->registerDeletedWidget(pWidget);
    }

    mSelectedWidgetsList.removeAll(pWidget);
    mWidgetMap.remove(pWidget->getWidgetIndex());
    pWidget->deleteLater();
}


//! @brief Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void ContainerObject::deleteModelObject(QString objectName, UndoStatusEnumT undoSettings)
{
    //qDebug() << "deleteGUIModelObject(): " << objectName << " in: " << this->getName() << " coresysname: " << this->getCoreSystemAccessPtr()->getRootSystemName() ;
    mpLogDataHandler->removeFavoriteVariableByComponentName(objectName);   //Does nothing unless this is a system

    ModelObjectMapT::iterator it = mModelObjectMap.find(objectName);
    ModelObject* obj_ptr = it.value();

        //Remove connectors that are connected to the model object
    QList<Connector *> pConnectorList = obj_ptr->getConnectorPtrs();
    for(int i=0; i<pConnectorList.size(); ++i)
    {
        this->removeSubConnector(pConnectorList[i], undoSettings);
    }

    if (undoSettings == Undo && !mUndoDisabled)
    {
        //Register removal of model object in undo stack
        this->mpUndoStack->registerDeletedObject(it.value());
        //emit componentChanged(); //!< @todo Why do we need to emit this signal after regestering in undostack
        //qDebug() << "Emitting!";
    }


    if (it != mModelObjectMap.end())
    {
        //! @todo maybe this should be handled somwhere else (not sure maybe this is the best place)
        if ((*it)->type() == ContainerPortType )
        {
            this->removeExternalPort((*it)->getName());
        }

        mModelObjectMap.erase(it);
        mSelectedModelObjectsList.removeAll(obj_ptr);
        mpScene->removeItem(obj_ptr);
        obj_ptr->deleteInHopsanCore();
        obj_ptr->deleteLater();
    }
    else
    {
        //qDebug() << "In delete GUIObject: could not find object with name " << objectName;
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Error: Could not delete object with name " + objectName + ", object not found");
    }
    emit checkMessages();
    mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief This function is used to rename a SubGUIObject
void ContainerObject::renameModelObject(QString oldName, QString newName, UndoStatusEnumT undoSettings)
{
    //Avoid work if no change is requested
    if (oldName != newName)
    {
        QString modNewName;
        // First find record with old name
        ModelObjectMapT::iterator it = mModelObjectMap.find(oldName);
        if (it != mModelObjectMap.end())
        {
            // Make a backup copy and erase old record
            ModelObject* obj_ptr = it.value();
            mModelObjectMap.erase(it);

            // Set new name, first in core then in gui object
            switch (obj_ptr->type())
            {
            case ContainerPortType : //!< @todo What will happen when we try to rename a groupport
                modNewName = this->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName);
                renameExternalPort(oldName, modNewName);
                break;
            default :
                modNewName = this->getCoreSystemAccessPtr()->renameSubComponent(oldName, newName);
            }

            // Override the GUI ModelObject name with the new name from the core
            obj_ptr->refreshDisplayName(modNewName);

            //Re-insert
            mModelObjectMap.insert(obj_ptr->getName(), obj_ptr);
        }
        else
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(QString("No GUI Object with name: ") + oldName + " found when attempting rename!");
        }

        if (undoSettings == Undo)
        {
            mpUndoStack->newPost();
            mpUndoStack->registerRenameObject(oldName, modNewName);
            emit componentChanged();
        }
    }
    emit checkMessages();
}


//! @brief Tells whether or not a component with specified name exist in the GraphicsView
bool ContainerObject::hasModelObject(QString name)
{
    return (mModelObjectMap.count(name) > 0);
}

//! @brief Takes ownership of supplied objects, widgets and connectors
//!
//! This method assumes that the previous owner have forgotten all about these objects, it however sets iself as new Qtparent, parentContainer and scene, overwriting the old values
void ContainerObject::takeOwnershipOf(QList<ModelObject*> &rModelObjectList, QList<Widget*> &rWidgetList)
{
    for (int i=0; i<rModelObjectList.size(); ++i)
    {
        //! @todo if a containerport is received we must update the external port list also, we cant handle such objects right now
        if (rModelObjectList[i]->type() != ContainerPortType)
        {
            this->getContainedScenePtr()->addItem(rModelObjectList[i]);
            rModelObjectList[i]->setParentContainerObject(this);
            mModelObjectMap.insert(rModelObjectList[i]->getName(), rModelObjectList[i]);
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
        mWidgetMap.insert(rWidgetList[i]->getWidgetIndex(), rWidgetList[i]);
        //! @todo what if idx already taken, dont care for now as we shal only move into groups when they are created
    }

    QList<Connector*> transitConnectors;
    QList<Connector*> internalConnectors;

    //Determine what connectors are transitconnectors
    for (int i=0; i<rModelObjectList.size(); ++i)
    {
        ModelObject *pObj = rModelObjectList[i];

        QList<Connector*> connectorPtrs = pObj->getConnectorPtrs();
        for(int i=0; i<connectorPtrs.size(); ++i)
        {
            if((rModelObjectList.contains(connectorPtrs[i]->getStartPort()->getParentModelObject())) &&
               (rModelObjectList.contains(connectorPtrs[i]->getEndPort()->getParentModelObject())))
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

                    //! @todo for now we disconnect the transit connection as we are noy yet capable of recreating the external connection
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
        internalConnectors[i]->getParentContainer()->forgetSubConnector(internalConnectors[i]);

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
        if (rModelObjectList.contains(transitConnectors[i]->getStartPort()->getParentModelObject()))
        {
            portpos = transitConnectors[i]->getEndPort()->getParentModelObject()->getCenterPos();
            //Make end object forget about this connector as we are actually splitting it into two new connectors
            transitConnectors[i]->getEndPort()->getParentModelObject()->forgetConnector(transitConnectors[i]);
            endPortIsTransitPort = true;
        }
        else
        {
            portpos = transitConnectors[i]->getStartPort()->getParentModelObject()->getCenterPos();
            //Make start object forget about this connector as we are actually splitting it into two new connectors
            transitConnectors[i]->getStartPort()->getParentModelObject()->forgetConnector(transitConnectors[i]);
        }

        //Create the "transit port"
        ModelObject *pTransPort = this->addModelObject(HOPSANGUICONTAINERPORTTYPENAME, portpos.toPoint(),0);

        //Make previous parent container forget about the connector
        transitConnectors[i]->getParentContainer()->forgetSubConnector(transitConnectors[i]);

        //Add the port to this container
        this->getContainedScenePtr()->addItem(transitConnectors[i]);
        transitConnectors[i]->setParentContainer(this);
        mSubConnectorList.append(transitConnectors[i]);

        //! @todo instead of having set startport and set end port, (we can keep them also maybe), we should have a function that sets startport if no port is set and end port if start port already set, dont know if good idea but we can keep it in mind, then we would not have to do stuff like bellow. (maybe we could call that function "connect")
        if (endPortIsTransitPort)
        {
            //Make new port and connector know about eachother
            transitConnectors[i]->setEndPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getEndPort()->getParentModelObject()->rememberConnector(transitConnectors[i]);
        }
        else
        {
            //Make new port and connector know about eachother
            transitConnectors[i]->setStartPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getStartPort()->getParentModelObject()->rememberConnector(transitConnectors[i]);
        }

        this->refreshExternalPortsAppearanceAndPosition();
    }

    //! @todo do much more stuff

    //! @todo center the objects in the new view

}


//! @brief Notifies container object that a gui widget has been selected
void ContainerObject::rememberSelectedWidget(Widget *widget)
{
    mSelectedWidgetsList.append(widget);
}


//! @brief Notifies container object that a gui widget is no longer selected
void ContainerObject::forgetSelectedWidget(Widget *widget)
{
    mSelectedWidgetsList.removeAll(widget);
}


//! @brief Returns a list with pointers to the selected GUI widgets
QList<Widget *> ContainerObject::getSelectedGUIWidgetPtrs()
{
    return mSelectedWidgetsList;
}


//! @brief Set a system parameter value
//! @todo how will this work in groups
bool ContainerObject::setParameterValue(QString name, QString value, bool force)
{
    const bool rc =  this->getCoreSystemAccessPtr()->setSystemParameterValue(name, value, force);
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

bool ContainerObject::setOrAddParameter(const CoreParameterData &rParameter, bool force)
{
    const bool rc = this->getCoreSystemAccessPtr()->setSystemParameter(rParameter, force);
    emit checkMessages();
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

bool ContainerObject::renameParameter(const QString oldName, const QString newName)
{
    const bool rc = this->getCoreSystemAccessPtr()->renameSystemParameter(oldName, newName);
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

//! @brief Notifies container object that a gui model object has been selected
void ContainerObject::rememberSelectedModelObject(ModelObject *object)
{
    QString name = object->getName();
    if(mModelObjectMap.contains(name) && mModelObjectMap.find(name).value() == object)
        mSelectedModelObjectsList.append(object);
}


//! @brief Notifies container object that a gui model object is no longer selected
void ContainerObject::forgetSelectedModelObject(ModelObject *object)
{
    mSelectedModelObjectsList.removeAll(object);
}


//! @brief Returns a list with pointers to the selected GUI model objects
QList<ModelObject *> ContainerObject::getSelectedModelObjectPtrs()
{
    return mSelectedModelObjectsList;
}


//! @brief Returns a pointer to the component with specified name, 0 if not found
ModelObject *ContainerObject::getModelObject(const QString modelObjectName)
{
    ModelObjectMapT::Iterator moit = mModelObjectMap.find(modelObjectName);
    if (moit != mModelObjectMap.end())
    {
        return moit.value();
    }
    else
    {
        return 0;
    }
}

//! @brief Get the port of a sub model object, returns 0 if modelobject or port not found
Port *ContainerObject::getModelObjectPort(const QString modelObjectName, const QString portName)
{
    ModelObject *pModelObject = this->getModelObject(modelObjectName);
    if (pModelObject != 0)
    {
        return pModelObject->getPort(portName);
    }
    else
    {
        return 0;
    }
}


//! @brief Find a connector in the connector vector
Connector* ContainerObject::findConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    Connector *item;
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
bool ContainerObject::hasConnector(QString startComp, QString startPort, QString endComp, QString endPort)
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


//! @brief Notifies container object that a subconnector has been selected
void ContainerObject::rememberSelectedSubConnector(Connector *pConnector)
{
    mSelectedSubConnectorsList.append(pConnector);
}


//! @brief Notifies container object that a subconnector has been deselected
void ContainerObject::forgetSelectedSubConnector(Connector *pConnector)
{
    mSelectedSubConnectorsList.removeAll(pConnector);
}

void ContainerObject::disconnectGroupPortFromItsRealPort(Port *pGroupPort, Port *pRealPort)
{
    QVector<Port*> connPortsVect = pGroupPort->getConnectedPorts();
    //! @todo what if a connected port is another group port

    assert(connPortsVect[0] == pRealPort);

    // The real port is the first so we skip it (begin at index 1), we cant disconnect from ourself
    // The first connection to the group port does not have an actual core connection
    // The GUI disconect will come later in the function calling this one
    // Disconnect all secondary connections
    for (int i=1; i<connPortsVect.size(); ++i)
    {
        this->getCoreSystemAccessPtr()->disconnect(connPortsVect[i]->getParentModelObjectName(),
                                                   connPortsVect[i]->getName(),
                                                   pRealPort->getParentModelObjectName(),
                                                   pRealPort->getName());

    }

    connPortsVect.erase(connPortsVect.begin());

    // Reconnect all secondary ports, to the new base port
    if (connPortsVect.size() > 0)
    {
        // New real port will be
        Port* pNewGroupRealPort = connPortsVect[0];

        for (int i=1; i<connPortsVect.size(); ++i)
        {
            this->getCoreSystemAccessPtr()->connect(pNewGroupRealPort->getParentModelObjectName(),
                                                    pNewGroupRealPort->getName(),
                                                    connPortsVect[i]->getParentModelObjectName(),
                                                    connPortsVect[i]->getName());
        }
    }
}

//! @brief Removes a specified connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void ContainerObject::removeSubConnector(Connector* pConnector, UndoStatusEnumT undoSettings)
{
    bool success=false;

    //! @todo why do we have this loop, probably as safety not to try and remove connectors thats not in this system
    for(int i=0; i<mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i] == pConnector)
        {
             //! @todo some error handling both ports must exist and be connected to each other
             if(pConnector->isConnected())
             {
                 //GroupPort *pStartGroupPort=0, *pEndGroupPort=0;
                 bool startPortIsGroupPort=false, endPortIsGroupPort=false;
                 Port *pStartP = pConnector->getStartPort();
                 Port *pEndP = pConnector->getEndPort();

                 if (pStartP->getPortType() == "GroupPortType")
                 {
                     startPortIsGroupPort=true;
                 }

                 if (pEndP->getPortType() == "GroupPortType")
                 {
                     endPortIsGroupPort = true;
                 }

                 qDebug() << "startPortIsGroupPort: " << startPortIsGroupPort << " endPortIsGroupPort: " << endPortIsGroupPort;
                 Port *pStartRealPort=0, *pEndRealPort=0;
                 // If no group ports, do normal disconnect
                 if ( !startPortIsGroupPort && !endPortIsGroupPort )
                 {
                    success = this->getCoreSystemAccessPtr()->disconnect(pStartP->getParentModelObjectName(),
                                                                         pStartP->getName(),
                                                                         pEndP->getParentModelObjectName(),
                                                                         pEndP->getName());
                 }
                 // If one or both of the ports were a group port
                 else
                 {
                     bool disconStartRealPort=false;
                     bool disconEndRealPort=false;

                     // If start port is group port but not end port
                     if ( startPortIsGroupPort && !endPortIsGroupPort )
                     {
                        pStartRealPort = pStartP->getRealPort();
                        pEndRealPort = pEndP;

                        // Determine if the port beeing disconnected is the actual REAL port, that is, the first port connected to the group port
                        if (pStartRealPort == pEndRealPort)
                        {
                            disconStartRealPort = true;
                        }
                     }
                     // If start port is not group port but end port is
                     else if ( !startPortIsGroupPort && endPortIsGroupPort )
                     {
                         pStartRealPort = pStartP;
                         pEndRealPort = pEndP->getRealPort();

                         // Determine if the port beeing disconnected is the actual REAL port, that is, the first port connected to the group port
                         if (pStartRealPort == pEndRealPort)
                         {
                             disconEndRealPort = true;
                         }
                     }
                     // Else both were group ports
                     else
                     {
                         pStartRealPort = pStartP->getRealPort();
                         pEndRealPort = pEndP->getRealPort();

                         //! @todo  do this
                         assert(false);
                     }

                     // If one or both real ports are beeing disconnected
                     if (disconStartRealPort || disconEndRealPort)
                     {
                         if (disconStartRealPort && disconEndRealPort)
                         {
                             gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("This is not supported yet, FAILURE! UNDEFINED BEHAVIOUR");
                             success = false;
                         }

                         //Handle disconnection of start base port
                         if (disconStartRealPort)
                         {
                             disconnectGroupPortFromItsRealPort(pStartP, pEndRealPort);
                             success = true;
                         }

                         //Handle disconnection of end base port
                         if (disconEndRealPort)
                         {
                             disconnectGroupPortFromItsRealPort(pEndP, pStartRealPort);
                             success = true;
                         }
                     }
                     else
                     {
                         // Disconnect appropriate core ports (when non is a real ports but one is groupport)
                         success = this->getCoreSystemAccessPtr()->disconnect(pStartRealPort->getParentModelObjectName(),
                                                                              pStartRealPort->getName(),
                                                                              pEndRealPort->getParentModelObjectName(),
                                                                              pEndRealPort->getName());
                     }
                 }
                 emit checkMessages();
             }
             else if (pConnector->isBroken())
             {
                 success = true;
             }
             break;
        }
    }

    //Delete the connector and remove it from scene and lists
    if(success)
    {
        if(undoSettings == Undo)
        {
            mpUndoStack->registerDeletedConnector(pConnector);
        }

        if(pConnector->getEndPort())
        {
            pConnector->getEndPort()->forgetConnection(pConnector);
        }
        if (pConnector->getStartPort())
        {
            pConnector->getStartPort()->forgetConnection(pConnector);
        }

        // Forget and delete the connector
        mSubConnectorList.removeAll(pConnector);
        mSelectedSubConnectorsList.removeAll(pConnector);
        mpScene->removeItem(pConnector);
        pConnector->deleteLater();

        emit connectorRemoved();
    }

    //Refresh the graphics view
    mpModelWidget->getGraphicsView()->updateViewPort();
}



//! @brief Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
//! @return A pointer to the created connector, 0 if failed, or connector unfinnished
Connector* ContainerObject::createConnector(Port *pPort, UndoStatusEnumT undoSettings)
{
    // When clicking end port (finish creation of connector)
    if (mIsCreatingConnector)
    {
        bool success = false;
        if (mpTempConnector->isDangling() && pPort)
        {
            // Check if we are connecting group ports
            Port *pStartRealPort=0, *pEndRealPort=0;
            bool startPortIsGroupPort=false, endPortIsGroupPort=false;
            if (mpTempConnector->getStartPort()->getPortType() == "GroupPortType")
            {
                startPortIsGroupPort=true;
                pStartRealPort = mpTempConnector->getStartPort()->getRealPort();
            }
            else
            {
                pStartRealPort = mpTempConnector->getStartPort();
            }

            if (pPort->getPortType() == "GroupPortType")
            {
                endPortIsGroupPort=true;
                pEndRealPort = pPort->getRealPort();
            }
            else
            {
                pEndRealPort = pPort;
            }
            //qDebug() << "startPortIsGroupPort: " << startPortIsGroupPort << " endPortIsGroupPort: " << endPortIsGroupPort;

            // Abort with error if trying to connect two group ports to each other
            //! @todo this must work in the future, connect will probably be OK, but disconnect is a bit more tricky
            if ( startPortIsGroupPort && endPortIsGroupPort )
            {
                gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("You are not allowed to connect two groups to each other yet. This will be suported in the future");
                return 0;
            }

            // Abort with error if trying to connect two undefined group ports to each other
            if ( (pStartRealPort==0) && (pEndRealPort==0) )
            {
                gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("You are not allowed to connect two undefined group ports to each other");
                return 0;
            }

            // Handle if one or both ports are group ports
            //! @todo cleanup and rewrite
            if ( startPortIsGroupPort || endPortIsGroupPort )
            {
                // If both group ports are defined
                if ( (pStartRealPort != 0) && (pEndRealPort != 0) )
                {
                    success = this->getCoreSystemAccessPtr()->connect(pStartRealPort->getParentModelObjectName(),
                                                                      pStartRealPort->getName(),
                                                                      pEndRealPort->getParentModelObjectName(),
                                                                      pEndRealPort->getName());
                }
                // If start known but not end
                else if ( (pStartRealPort != 0) && (pEndRealPort == 0) )
                {
                    success = true;

                }
                // Else start unknown but end known
                else if ( (pStartRealPort == 0) && (pEndRealPort != 0) )
                {
                    success = true;
                }
                else
                {
                    //This should never happen, handled above
                    success = false;
                }
            }
            // Else treat as normal ports
            else
            {
                success = this->getCoreSystemAccessPtr()->connect(pStartRealPort->getParentModelObjectName(),
                                                                  pStartRealPort->getName(),
                                                                  pEndRealPort->getParentModelObjectName(),
                                                                  pEndRealPort->getName());
            }
        }

        if (success)
        {
            gpMainWindow->hideHelpPopupMessage();
            mpTempConnector->setEndPort(pPort);
            mpTempConnector->finishCreation();
            mSubConnectorList.append(mpTempConnector);

            // Refresh startport now that end port has been connected (for system ports)
            if (mpTempConnector->getStartPort()->getPortType() == "SystemPortType")
            {
                mpTempConnector->getStartPort()->refreshPortGraphics();
            }

            if(undoSettings == Undo)
            {
                mpUndoStack->newPost();
                mpUndoStack->registerAddedConnector(mpTempConnector);
            }
            mpModelWidget->hasChanged();

            mIsCreatingConnector = false;
        }

        emit checkMessages();

        // Return ptr to the created connector
        return mpTempConnector;
    }
    // When clicking start port (begin creation of connector)
    else
    {
        deselectAll();

        mpTempConnector = new Connector(this);

        QPointF startPos;
        if (pPort)
        {
            // Make Connector and Port/ModelObject know about this each other
            mpTempConnector->setStartPort(pPort);

            // Get initial start position for the connector
            startPos = mapToScene(mapFromItem(pPort, pPort->boundingRect().center()));
        }
        // Add and set startpoint (If port was 0 then 0,0 will be used)
        mpTempConnector->setPos(startPos);
        mpTempConnector->updateStartPoint(startPos);
        mpTempConnector->addPoint(startPos);
        mpTempConnector->addPoint(startPos);

        mpTempConnector->drawConnector();

        gpMainWindow->showHelpPopupMessage("Create the connector by clicking in the workspace. Finish connector by clicking on another component port.");
        mIsCreatingConnector = true;

        // Return ptr to the created connector
        return mpTempConnector;
    }
}

//! @brief Create a connector when both ports are known (when loading primarily)
Connector* ContainerObject::createConnector(Port *pPort1, Port *pPort2, UndoStatusEnumT undoSettings)
{
    if (!mIsCreatingConnector)
    {
        createConnector(pPort1, undoSettings);
        Connector* pConn = createConnector(pPort2, undoSettings);

        // If we failed we still want to finnish and ad this connector (if success it was already added)
        if (!pConn->isConnected())
        {
            mpTempConnector->finishCreation();
            mSubConnectorList.append(mpTempConnector);
        }

        // Regardless if fail or not we are no longer creating a connector
        mIsCreatingConnector = false;

        return pConn;
    }
    else
    {
        // This should never happen, but just in case
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not create connector, connector creation already in progress");
        return 0;
    }
}


//! @brief Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void ContainerObject::cutSelected(CopyStack *xmlStack)
{
    this->copySelected(xmlStack);
    this->mpUndoStack->newPost("cut");
    emit deleteSelected();
    mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void ContainerObject::copySelected(CopyStack *xmlStack)
{
    //Don't copy if python widget or message widget as focus (they also use ctrl-c key sequence)
    if(gpMainWindow->mpMessageWidget->textEditHasFocus() || gpMainWindow->mpTerminalWidget->mpConsole->hasFocus())
        return;

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        gCopyStack.clear();
        copyRoot = gCopyStack.getCopyRoot();
    }
    else
    {
        xmlStack->clear();
        copyRoot = xmlStack->getCopyRoot();
    }

        //Store center point
    QPointF center = getCenterPointFromSelection();
    appendCoordinateTag(*copyRoot, center.x(), center.y());

        //Copy components
    QList<ModelObject *>::iterator it;
    for(it = mSelectedModelObjectsList.begin(); it!=mSelectedModelObjectsList.end(); ++it)
    {
        qDebug() << "Copying " << (*it)->getName();
        (*it)->saveToDomElement(*copyRoot);

        QStringList parNames = (*it)->getParameterNames();
        for(int n=0; n<parNames.size(); ++n)
        {
            if(getParameterNames().contains((*it)->getParameterValue(parNames[n])))
            {
                qDebug() << "Component depends on system parameter: " << (*it)->getParameterValue(parNames[n]);
                CoreParameterData parData;
                getParameter((*it)->getParameterValue(parNames[n]), parData);
                QDomElement parElement = appendDomElement(*copyRoot, "parameter");
                parElement.setAttribute("name", parData.mName);
                parElement.setAttribute("value", parData.mValue);
                parElement.setAttribute("type", parData.mType);
            }
        }
    }

        //Copy connectors
    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->getStartPort()->getParentModelObject()->isSelected() && mSubConnectorList[i]->getEndPort()->getParentModelObject()->isSelected() && mSubConnectorList[i]->isActive())
        {
            mSubConnectorList[i]->saveToDomElement(*copyRoot);
        }
    }

        //Copy widgets
    QMap<size_t, Widget *>::iterator itw;
    for(itw = mWidgetMap.begin(); itw!=mWidgetMap.end(); ++itw)
    {
        if((*itw)->isSelected())
        {
            (*itw)->saveToDomElement(*copyRoot);
        }
    }
}


//! @brief Pastes the contents in the copy stack at the mouse position
//! @see cutSelected()
//! @see copySelected()
void ContainerObject::paste(CopyStack *xmlStack)
{

    //gpMainWindow->mpHcomWidget->mpConsole->printDebugMessage(gCopyStack.getXML());

    mpUndoStack->newPost("paste");
    mpModelWidget->hasChanged();

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
    emit deselectAllConnectors();

    QHash<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called

        //Determine paste offset (will paste components at mouse position
    QDomElement coordTag = copyRoot->firstChildElement(HMF_COORDINATETAG);
    double x, y;
    parseCoordinateTag(coordTag, x, y);
    QPointF oldCenter = QPointF(x, y);

    QCursor cursor;
    QPointF newCenter = mpModelWidget->getGraphicsView()->mapToScene(mpModelWidget->getGraphicsView()->mapFromGlobal(cursor.pos()));

    qDebug() << "Pasting at " << newCenter;

    double xOffset = newCenter.x() - oldCenter.x();
    double yOffset = newCenter.y() - oldCenter.y();

        //Paste components
    QDomElement objectElement = copyRoot->firstChildElement(HMF_COMPONENTTAG);
    while(!objectElement.isNull())
    {
        ModelObject *pObj = loadModelObject(objectElement, gpMainWindow->mpLibrary, this);

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(xOffset, yOffset);
        mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());

        renameMap.insert(objectElement.attribute(HMF_NAMETAG), pObj->getName());
        //objectElement.setAttribute("name", renameMap.find(objectElement.attribute(HMF_NAMETAG)).value());
        objectElement = objectElement.nextSiblingElement("component");
    }

        // Paste subsystems
    //! @todo maybe this subsystem loop can be merged with components above somehow. Basically the same code is used now after some cleanup, That way we could  have one loop for guimodelobjects, one for connector and after some cleanup one for widgets
    QDomElement systemElement = copyRoot->firstChildElement(HMF_SYSTEMTAG);
    while (!systemElement.isNull())
    {
        ModelObject* pObj = loadModelObject(systemElement, gpMainWindow->mpLibrary, this, Undo);
        renameMap.insert(systemElement.attribute(HMF_NAMETAG), pObj->getName());
        systemElement = systemElement.nextSiblingElement(HMF_SYSTEMTAG);

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(xOffset, yOffset);
        mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());
    }

        // Paste container ports
    QDomElement systemPortElement = copyRoot->firstChildElement(HMF_SYSTEMPORTTAG);
    while (!systemPortElement.isNull())
    {
        ModelObject* pObj = loadContainerPortObject(systemPortElement, gpMainWindow->mpLibrary, this, Undo);
        renameMap.insert(systemPortElement.attribute(HMF_NAMETAG), pObj->getName());
        systemPortElement = systemPortElement.nextSiblingElement(HMF_SYSTEMPORTTAG);

            //Apply offset to pasted object
        QPointF oldPos = pObj->pos();
        pObj->moveBy(xOffset, yOffset);
        mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());
    }

        //Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement(HMF_CONNECTORTAG);
    while(!connectorElement.isNull())
    {
        QDomElement tempConnectorElement = connectorElement.cloneNode(true).toElement();
        tempConnectorElement.setAttribute("startcomponent", renameMap.find(connectorElement.attribute("startcomponent")).value());
        tempConnectorElement.setAttribute("endcomponent", renameMap.find(connectorElement.attribute("endcomponent")).value());

        bool sucess = loadConnector(tempConnectorElement, this, Undo);
        if (sucess)
        {
            //qDebug() << ",,,,,,,,,: " << tempConnectorElement.attribute("startcomponent") << " " << tempConnectorElement.attribute("startport") << " " << tempConnectorElement.attribute("endcomponent") << " " << tempConnectorElement.attribute("endport");
            Connector *tempConnector = this->findConnector(tempConnectorElement.attribute("startcomponent"), tempConnectorElement.attribute("startport"),
                                                              tempConnectorElement.attribute("endcomponent"), tempConnectorElement.attribute("endport"));

                //Apply offset to connector and register it in undo stack
            tempConnector->moveAllPoints(xOffset, yOffset);
            tempConnector->drawConnector(true);
            for(int i=0; i<(tempConnector->getNumberOfLines()-2); ++i)
            {
                mpUndoStack->registerModifiedConnector(QPointF(tempConnector->getLine(i)->pos().x(), tempConnector->getLine(i)->pos().y()),
                                                      tempConnector->getLine(i+1)->pos(), tempConnector, i+1);
            }
        }

        connectorElement = connectorElement.nextSiblingElement("connect");
    }

        //Paste widgets
    QDomElement textBoxElement = copyRoot->firstChildElement(HMF_TEXTBOXWIDGETTAG);
    while(!textBoxElement.isNull())
    {
        TextBoxWidget *pWidget = loadTextBoxWidget(textBoxElement, this, NoUndo);

        pWidget->setSelected(true);
        pWidget->moveBy(xOffset, yOffset);
        mpUndoStack->registerAddedWidget(pWidget);
        textBoxElement = textBoxElement.nextSiblingElement(HMF_TEXTBOXWIDGETTAG);
    }

        //Paste system parameters
    QDomElement parElement = copyRoot->firstChildElement("parameter");
    while(!parElement.isNull())
    {
        QString name = parElement.attribute("name");
        QString value = parElement.attribute("value");
        QString type = parElement.attribute("type");
        if(!getParameterNames().contains(name))
        {
            CoreParameterData parData = CoreParameterData(name, value, type);
            setOrAddParameter(parData);
        }
        parElement = parElement.nextSiblingElement("parameter");
    }


        //Select all pasted components
    QHash<QString, QString>::iterator itn;
    for(itn = renameMap.begin(); itn != renameMap.end(); ++itn)
    {
        mModelObjectMap.find(itn.value()).value()->setSelected(true);
    }

    mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Aligns all selected objects vertically to the last selected object.
void ContainerObject::alignX()
{
    if(mSelectedModelObjectsList.size() > 1)
    {
        mpUndoStack->newPost("alignx");
        for(int i=0; i<mSelectedModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            mSelectedModelObjectsList.at(i)->setCenterPos(QPointF(mSelectedModelObjectsList.last()->getCenterPos().x(), mSelectedModelObjectsList.at(i)->getCenterPos().y()));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }
        }
        mpModelWidget->hasChanged();
    }
}


//! @brief Aligns all selected objects horizontally to the last selected object.
void ContainerObject::alignY()
{
    if(mSelectedModelObjectsList.size() > 1)
    {
        mpUndoStack->newPost("aligny");
        for(int i=0; i<mSelectedModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            mSelectedModelObjectsList.at(i)->setCenterPos(QPointF(mSelectedModelObjectsList.at(i)->getCenterPos().x(), mSelectedModelObjectsList.last()->getCenterPos().y()));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }
        }
        mpModelWidget->hasChanged();
    }
}


//! @brief Calculates the geometrical center position of the selected objects.
QPointF ContainerObject::getCenterPointFromSelection()
{
    double sumX = 0;
    double sumY = 0;
    int nSelected = 0;
    for(int i=0; i<mSelectedModelObjectsList.size(); ++i)
    {
        sumX += mSelectedModelObjectsList.at(i)->getCenterPos().x();
        sumY += mSelectedModelObjectsList.at(i)->getCenterPos().y();
        ++nSelected;
    }
    for(int i=0; i<mSelectedWidgetsList.size(); ++i)
    {
        sumX += mSelectedWidgetsList.at(i)->getCenterPos().x();
        sumY += mSelectedWidgetsList.at(i)->getCenterPos().y();
        ++nSelected;
    }

    return QPointF(sumX/nSelected, sumY/nSelected);
}


//! @brief Groups the selected objects together.
void ContainerObject::groupSelected(QPointF pt)
{
    qDebug() << "pos where we want to create group: " << pt;
    qDebug() << "In group selected";

    //Copy the selected objects, the lists will be cleared by addGuiobject and we need to keep this information
    QList<ModelObject*> modelObjects = mSelectedModelObjectsList;
    QList<Widget*> widgets = mSelectedWidgetsList;

    //"Detach" the selected objects from this container, basically by removing pointers from the subobject storage maps, make this container forget aboout these objects
    for (int i=0; i<modelObjects.size(); ++i)
    {
        //! @todo if a containerport is selcted we need to remove it in core, not only from the storage vector, we must also make sure that the external ports are updated accordingly, for now we just ignore them (maybe we should allways ignore them when grouping)
        if (modelObjects[i]->type() != ContainerPortType)
        {
            // Maybe take ownership should handle this
            mModelObjectMap.remove(modelObjects[i]->getName());
        }
        else
        {
            //Remove container ports, we cant group them for now
            modelObjects.removeAt(i);
            --i;
        }
    }

    for (int i=0; i<widgets.size(); ++i)
    {
        mWidgetMap.remove(widgets[i]->getWidgetIndex());
    }

    if (modelObjects.size() > 0 || widgets.size() > 0)
    {
        //Create a new group at the location of the specified
        ModelObject* pObj =  this->addModelObject(HOPSANGUIGROUPTYPENAME, pt.toPoint(),0);
        ContainerObject* pContainer =  qobject_cast<ContainerObject*>(pObj);

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
}


void ContainerObject::replaceComponent(QString name, QString newType)
{
    this->deselectAll();
    mSelectedModelObjectsList.clear();

    ModelObject *obj = getModelObject(name);

    qDebug() << "Replacing " << obj->getTypeName() << " with " << newType;


    CopyStack *xmlStack = new CopyStack();
    QDomElement *copyRoot = xmlStack->getCopyRoot();

        //Copy connectors
    for(int i = 0; i != obj->getConnectorPtrs().size(); ++i)
    {
        obj->getConnectorPtrs().at(i)->saveToDomElement(*copyRoot);
    }

    qDebug() << copyRoot->toDocument().toString();

    QStringList parNames = obj->getParameterNames();
    QStringList parValues;
    for(int i=0; i<parNames.size(); ++i)
    {
        parValues << obj->getParameterValue(parNames.at(i));
    }

    qDebug() << "Parameters: " << parNames << parValues;

    QPointF pos = obj->getCenterPos();
    double rot = obj->rotation();

    deleteModelObject(name);

    qDebug() << "Name = " << name;

    ModelObject *newObj = addModelObject(newType, pos, rot);
    renameModelObject(newObj->getName(), name);



        //Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement(HMF_CONNECTORTAG);
    while(!connectorElement.isNull())
    {
        bool sucess = loadConnector(connectorElement, this, Undo);
        if (sucess)
        {
            Connector *tempConnector = this->findConnector(connectorElement.attribute("startcomponent"), connectorElement.attribute("startport"),
                                                           connectorElement.attribute("endcomponent"), connectorElement.attribute("endport"));

            //Apply offset to connector and register it in undo stack
            tempConnector->drawConnector(true);
            for(int i=0; i<(tempConnector->getNumberOfLines()-2); ++i)
            {
                mpUndoStack->registerModifiedConnector(QPointF(tempConnector->getLine(i)->pos().x(), tempConnector->getLine(i)->pos().y()),
                                                       tempConnector->getLine(i+1)->pos(), tempConnector, i+1);
            }
        }

        connectorElement = connectorElement.nextSiblingElement("connect");
    }


    for(int i=0; i<parNames.size(); ++i)
    {
        if(newObj->getParameterNames().contains(parNames.at(i)))
        {
            newObj->setParameterValue(parNames.at(i), parValues.at(i), true);
        }
    }

    this->deselectAll();
    mSelectedModelObjectsList.clear();
}


//! @brief Selects model objects in section with specified number.
//! @param no Number of section
//! @param append True if previously selected objects shall remain selected
void ContainerObject::selectSection(int no, bool append)
{
    if(!append)
    {
        deselectAll();
    }
    if(mSection.size() < no+1)
    {
        return;
    }
    for(int i=0; i<mSection.at(no).size(); ++i)
    {
        mSection[no][i]->select();
    }
}


//! @brief Assigns the selected component to the section with specified number.
//! This is used to "group" components into sections with Ctrl+#, so they can be selected quickly again by pressing #.
//! @param no Number of section
void ContainerObject::assignSection(int no)
{
    if(!isSubObjectSelected()) return;
    while(mSection.size()<no+1)
    {
        QList<ModelObject *> dummyList;
        mSection.append(dummyList);
    }
    mSection[no].clear();
    mSection[no].append(mSelectedModelObjectsList);
}


//! @brief Selects all objects and connectors.
void ContainerObject::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllConnectors();
}


//! @brief Deselects all objects and connectors.
void ContainerObject::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();
}


//! @brief Hides all component names.
//! @see showNames()
void ContainerObject::hideNames()
{
    mpUndoStack->newPost("hideallnames");
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! @brief Shows all component names.
//! @see hideNames()
void ContainerObject::showNames()
{
    mpUndoStack->newPost("showallnames");
    emit showAllNameText();
}


//! @brief Toggles name text on or off
//! @see showNames();
//! @see hideNames();
void ContainerObject::toggleNames(bool value)
{
    if(value)
    {
        emit showAllNameText();
    }
    else
    {
        emit hideAllNameText();
    }
    mSubComponentNamesHidden = !value;
}


void ContainerObject::toggleSignals(bool value)
{
    mSignalsHidden = !value;
    emit showOrHideSignals(value);
}

//! @brief Slot that sets hide ports flag to true or false
void ContainerObject::showSubcomponentPorts(bool doShowThem)
{
    mSubComponentPortsHidden = !doShowThem;
    emit showOrHideAllSubComponentPorts(doShowThem);
}


//! @brief Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void ContainerObject::undo()
{
    mpUndoStack->undoOneStep();
}


//! @brief Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void ContainerObject::redo()
{
    mpUndoStack->redoOneStep();
}

//! @brief Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void ContainerObject::clearUndo()
{
    qDebug() << "before mUndoStack->clear(); in GUIContainerObject: " << this->getName();
    mpUndoStack->clear();
}


//! @brief Returns true if at least one GUIObject is selected
bool ContainerObject::isSubObjectSelected()
{
    return (mSelectedModelObjectsList.size() > 0);
}

bool ContainerObject::setVariableAlias(QString compName, QString portName, QString varName, QString alias)
{
    mpModelWidget->hasChanged();
    return getCoreSystemAccessPtr()->setVariableAlias(compName, portName, varName, alias);
}

QString ContainerObject::getFullNameFromAlias(const QString alias)
{
    QString comp, port, var;
    getCoreSystemAccessPtr()->getFullVariableNameByAlias(alias, comp, port, var);
    return makeConcatName(comp,port,var);
}

QStringList ContainerObject::getAliasNames()
{
    return getCoreSystemAccessPtr()->getAliasNames();
}


//! @brief Returns true if at least one GUIConnector is selected
bool ContainerObject::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}


//! @brief Returns a pointer to the undo stack
UndoStack *ContainerObject::getUndoStackPtr()
{
    return mpUndoStack;
}


//! @brief Returns a pointer to the drag-copy copy stack
CopyStack *ContainerObject::getDragCopyStackPtr()
{
    return mpDragCopyStack;
}

size_t ContainerObject::getNumberOfLogSamples()
{
    //Needs to be overloaded
    assert(false);
}

void ContainerObject::setNumberOfLogSamples(size_t /*nSamples*/)
{
    //Needs to be overloaded
    assert(false);
}


void ContainerObject::setModelInfo(const QString &author, const QString &email, const QString &affiliation, const QString &description)
{
    mAuthor = author;
    mEmail = email;
    mAffiliation = affiliation;
    mDescription = description;
}

void ContainerObject::getModelInfo(QString &author, QString &email, QString &affiliation, QString &description) const
{
    author = mAuthor;
    email = mEmail;
    affiliation = mAffiliation;
    description = mDescription;
}


//! @brief Specifies model file for the container object
void ContainerObject::setModelFile(QString path)
{
    mModelFileInfo.setFile(path);
}


//! @brief Returns a copy of the model file info of the container object
QFileInfo ContainerObject::getModelFileInfo()
{
    return mModelFileInfo;
}


//! @brief Specifies a script file to be executed when model is loaded
//! @todo Shall we have this?
void ContainerObject::setScriptFile(QString path)
{
    mScriptFilePath = path;
}


//! @brief Returns path to the script file
QString ContainerObject::getScriptFile()
{
    return mScriptFilePath;
}


//! @brief Returns a list with the names of the model objects in the container
QStringList ContainerObject::getModelObjectNames()
{
    QStringList retval;
    ContainerObject::ModelObjectMapT::iterator it;
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        retval.append(it.value()->getName());
    }
    return retval;
}


//! @brief Returns a list with pointers to GUI widgets
QList<Widget *> ContainerObject::getWidgets()
{
    QList<Widget *> list;
    QMap<size_t, Widget *>::iterator it;
    for(it=mWidgetMap.begin(); it!=mWidgetMap.end(); ++it)
    {
        list.append(it.value());
    }
    return list;
}

//! @brief Returns the path to the icon with iso graphics.
//! @todo should we return full path or relative
QString ContainerObject::getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType)
{
    return mModelObjectAppearance.getIconPath(gfxType, absrelType);
}


//! @brief Sets the path to the icon of the specified type
void ContainerObject::setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType)
{
    mModelObjectAppearance.setIconPath(path, gfxType, absrelType);
}


//! @brief Access function for mIsCreatingConnector
bool ContainerObject::isCreatingConnector()
{
    return mIsCreatingConnector;
}


//! @brief Tells container object to remember a new sub connector
void ContainerObject::rememberSubConnector(Connector *pConnector)
{
    mSubConnectorList.append(pConnector);
}


//! @brief This is a helpfunction that can be used to make a container "forget" about a certain connector
//!
//! It does not delete the connector and connected components dos not forget about it
//! use only when transfering ownership of objects to an other container
void ContainerObject::forgetSubConnector(Connector *pConnector)
{
    mSubConnectorList.removeAll(pConnector);
}

//! @brief Refresh the graphics of all internal container ports
void ContainerObject::refreshInternalContainerPortGraphics()
{
    ModelObjectMapT::iterator moit;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == ContainerPortType)
        {
            //We assume that a container port only have ONE gui port
            moit.value()->getPortListPtrs().first()->refreshPortGraphics();
        }
    }
}


void ContainerObject::addExternalContainerPortObject(ModelObject* pModelObject)
{
    this->createRefreshExternalPort(pModelObject->getName());
}

//! @brief Aborts creation of new connector.
void ContainerObject::cancelCreatingConnector()
{
    if(mIsCreatingConnector)
    {
        mpTempConnector->getStartPort()->forgetConnection(mpTempConnector);
        if(!mpTempConnector->getStartPort()->isConnected() && !mSubComponentPortsHidden)
        {
            mpTempConnector->getStartPort()->show();
        }
        mIsCreatingConnector = false;
        delete(mpTempConnector);
        gpMainWindow->hideHelpPopupMessage();
    }
}


//! @brief Swiches mode of connector being created to or from diagonal mode.
//! @param diagonal Tells whether or not connector shall be diagonal or not
void ContainerObject::makeConnectorDiagonal(bool diagonal)
{
    if (mIsCreatingConnector && (mpTempConnector->isMakingDiagonal() != diagonal))
    {
        mpTempConnector->makeDiagonal(diagonal);
        mpTempConnector->drawConnector();
        mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


//! @brief Redraws the connector being created.
//! @param pos Position to draw connector to
void ContainerObject::updateTempConnector(QPointF pos)
{
    mpTempConnector->updateEndPoint(pos);
    mpTempConnector->drawConnector();
}


//! @brief Adds one new line to the connector being created.
//! @param pos Position to add new line at
void ContainerObject::addOneConnectorLine(QPointF pos)
{
    mpTempConnector->addPoint(pos);
}


//! @brief Removse one line from connector being created.
//! @param pos Position to redraw connector to after removing the line
void ContainerObject::removeOneConnectorLine(QPointF pos)
{
    if((mpTempConnector->getNumberOfLines() == 1 && mpTempConnector->isMakingDiagonal()) ||  (mpTempConnector->getNumberOfLines() == 2 && !mpTempConnector->isMakingDiagonal()))
    {
        mpTempConnector->getStartPort()->forgetConnection(mpTempConnector);
        if(!mpTempConnector->getStartPort()->isConnected() && !mSubComponentPortsHidden)
        {
            mpTempConnector->getStartPort()->show();
        }
        mpTempConnector->getStartPort()->getParentModelObject()->forgetConnector(mpTempConnector);
        mIsCreatingConnector = false;
        mpModelWidget->getGraphicsView()->setIgnoreNextContextMenuEvent();
        delete(mpTempConnector);
        gpMainWindow->hideHelpPopupMessage();
    }

    if(mIsCreatingConnector)
    {
        mpTempConnector->removePoint(true);
        mpTempConnector->updateEndPoint(pos);
        mpTempConnector->drawConnector();
        mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


void ContainerObject::setUndoDisabled(bool disabled, bool dontAskJustDoIt)
{
    setUndoEnabled(!disabled, dontAskJustDoIt);
}


//! @brief Disables the undo function for the current model.
//! @param enabled Tells whether or not to enable the undo stack
//! @param dontAskJustDoIt If true, the warning box will not appear
void ContainerObject::setUndoEnabled(bool enabled, bool dontAskJustDoIt)
{
    if(!enabled)
    {
        bool doIt=true;
        if (!dontAskJustDoIt)
        {
            QMessageBox disableUndoWarningBox(QMessageBox::Warning, tr("Warning"),tr("Disabling undo history will clear all undo history for this model. Do you want to continue?"), 0, 0);
            disableUndoWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
            disableUndoWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);
            disableUndoWarningBox.setWindowIcon(gpMainWindow->windowIcon());

            doIt = (disableUndoWarningBox.exec() == QMessageBox::AcceptRole);
        }

        if (doIt)
        {
            this->clearUndo();
            mUndoDisabled = true;
            if(gpMainWindow->mpModelHandler->getCurrentViewContainerObject() == this)      //Only modify main window actions if this is current container
            {
                gpMainWindow->mpUndoAction->setDisabled(true);
                gpMainWindow->mpRedoAction->setDisabled(true);
            }
        }
    }
    else
    {
        mUndoDisabled = false;
        if(gpMainWindow->mpModelHandler->getCurrentViewContainerObject() == this)      //Only modify main window actions if this is current container
        {
            gpMainWindow->mpUndoAction->setDisabled(false);
            gpMainWindow->mpRedoAction->setDisabled(false);
        }
    }

    if(gpMainWindow->mpDisableUndoAction->isChecked() != mUndoDisabled)
        gpMainWindow->mpDisableUndoAction->setChecked(mUndoDisabled);
}


void ContainerObject::setSaveUndo(bool save)
{
    mSaveUndoStack = save;
}


//! @brief Tells whether or not unconnected ports in container are hidden
bool ContainerObject::areSubComponentPortsHidden()
{
    return mSubComponentPortsHidden;
}


//! @brief Tells whether or not object names in container are hidden
bool ContainerObject::areSubComponentNamesHidden()
{
    return mSubComponentNamesHidden;
}


//! @brief Tells whether or not signal components are hidden
bool ContainerObject::areSignalsHidden()
{
    return mSignalsHidden;
}


//! @brief Tells whether or not undo/redo is enabled
bool ContainerObject::isUndoEnabled()
{
    return !mUndoDisabled;
}


//! @brief Tells whether or not the save undo option is active
bool ContainerObject::getSaveUndo()
{
    return mSaveUndoStack;
}


//! @brief Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void ContainerObject::updateMainWindowButtons()
{
    gpMainWindow->mpUndoAction->setDisabled(mUndoDisabled);
    gpMainWindow->mpRedoAction->setDisabled(mUndoDisabled);

    //gpMainWindow->mpPlotAction->setDisabled(mpLogDataHandler->isEmpty());
    gpMainWindow->mpShowLossesAction->setDisabled(mpLogDataHandler->isEmpty());
    //gpMainWindow->mpAnimateAction->setDisabled(mpNewPlotData->isEmpty());
}


//! @brief Sets the iso graphics option for the model
void ContainerObject::setGfxType(GraphicsTypeEnumT gfxType)
{
    this->mGfxType = gfxType;
    this->mpModelWidget->getGraphicsView()->updateViewPort();
    emit setAllGfxType(mGfxType);
}


//! @brief Returns current graphics type used by container object
GraphicsTypeEnumT ContainerObject::getGfxType()
{
    return mGfxType;
}


//! @brief A slot that opens the properties dialog
void ContainerObject::openPropertiesDialogSlot()
{
    this->openPropertiesDialog();
}


//! @brief Slot that tells all selected name texts to deselect themselves
void ContainerObject::deselectSelectedNameText()
{
    emit deselectAllNameText();
}


//! @brief Defines the right click menu for container objects.
//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void ContainerObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    QAction *saveAction = menu.addAction(tr("Save Subsystem As"));
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    if(isExternal())
    {
        saveAction->setDisabled(true);
    }
    QAction *pAction = this->buildBaseContextMenu(menu, event);
    if (pAction == loadAction)
    {
        QDir fileDialog; QFile file;
        QString modelFilePath = QFileDialog::getOpenFileName(gpMainWindow, tr("Choose Subsystem File"),
                                                             gConfig.getSubsystemDir(),
                                                             tr("Hopsan Model Files (*.hmf)"));
        if (!modelFilePath.isNull())
        {
            file.setFileName(modelFilePath);
            QFileInfo fileInfo(file);
            gConfig.setSubsystemDir(fileInfo.absolutePath());

            bool doIt = true;
            if (mModelObjectMap.size() > 0)
            {
                QMessageBox clearAndLoadQuestionBox(QMessageBox::Warning, tr("Warning"),tr("All current contents of the system will be replaced. Do you want to continue?"), 0, 0);
                clearAndLoadQuestionBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
                clearAndLoadQuestionBox.addButton(tr("&No"), QMessageBox::RejectRole);
                clearAndLoadQuestionBox.setWindowIcon(gpMainWindow->windowIcon());
                doIt = (clearAndLoadQuestionBox.exec() == QMessageBox::AcceptRole);
            }

            if (doIt)
            {
                this->clearContents();

                QDomDocument domDocument;
                QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
                if (!hmfRoot.isNull())
                {
                    //! @todo Check version numbers
                    //! @todo check if we could load else give error message and dont attempt to load
                    QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
                    this->setModelFileInfo(file); //Remember info about the file from which the data was loaded
                    QFileInfo fileInfo(file);
                    this->setAppearanceDataBasePath(fileInfo.absolutePath());
                    this->loadFromDomElement(systemElement);
                }
            }
        }
    }
    else if(pAction == saveAction)
    {
        //Get file name
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Save Subsystem As"),
                                                     gConfig.getLoadModelDir(),
                                                     gpMainWindow->tr("Hopsan Model Files (*.hmf)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }


        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
        QFile file(modelFilePath);   //Create a QFile object
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
            return;
        }

        //Save xml document
        QDomDocument domDocument;
        QDomElement rootElement;
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

        // Save the required external lib names
        QVector<QString> extLibNames;
        CoreLibraryAccess coreLibAccess;
        coreLibAccess.getLoadedLibNames(extLibNames);

        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (int i=0; i<extLibNames.size(); ++i)
        {
            appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
        }

        //Save the model component hierarcy
        this->saveToDomElement(rootElement, FullModel);

        //Save to file
        QFile xmlFile(modelFilePath);
        if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not save to file: " + modelFilePath);
            return;
        }
        QTextStream out(&xmlFile);
        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        domDocument.save(out, XMLINDENTATION);

        //Close the file
        xmlFile.close();

       // mpModelWidget->saveTo(modelFilePath, FullModel);
    }

    //Dont call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
    //GUIModelObject::contextMenuEvent(event);
    ////QGraphicsItem::contextMenuEvent(event);
}


//! @brief Defines the double click event for container objects (used to enter containers).
void ContainerObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    ModelObject::mouseDoubleClickEvent(event);
    this->enterContainer();
}


//! @brief Opens the properites dialog for container objects.
void ContainerObject::openPropertiesDialog()
{
    //Do Nothing
}


//! @brief Clears all of the contained objects (and delets them).
//! This code cant be run in the desturctor as this wold cause wired behaviour in the derived susyem class.
//! The core system would be deleted before container clear code is run, that is why we have it as a convenient protected function
void ContainerObject::clearContents()
{
    ModelObjectMapT::iterator mit;
    QMap<size_t, Widget *>::iterator wit;

    qDebug() << "Clearing model objects in " << getName();
    //We cant use for loop over iterators as the maps are modified on each delete (and iterators invalidated)
    mit=mModelObjectMap.begin();
    while (mit!=mModelObjectMap.end())
    {
        //This may lead to a crash if undo stack is not disabled before calling this
        (*mit)->deleteMe();
        mit=mModelObjectMap.begin();
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
void ContainerObject::enterContainer()
{
    // First deselect everything so that buttons pressed in the view are not sent to obejcts in the previous container
    mpParentContainerObject->deselectAll(); //deselect myself and anyone else

    // Show this scene
    mpModelWidget->getGraphicsView()->setScene(getContainedScenePtr());
    mpModelWidget->getGraphicsView()->setContainerPtr(this);
    mpModelWidget->getQuickNavigationWidget()->addOpenContainer(this);

    // Disconnect parent system and connect new system with actions
    mpParentContainerObject->unmakeMainWindowConnectionsAndRefresh();
    this->makeMainWindowConnectionsAndRefresh();



    refreshInternalContainerPortGraphics();

    mpModelWidget->setExternalSystem((this->isExternal() &&
                                           this != mpModelWidget->getTopLevelSystemContainer()) ||
                                           this->isAncestorOfExternalSubsystem());
}

//! @brief Exit a container object and maks its the view represent its parents contents.
void ContainerObject::exitContainer()
{
    this->deselectAll();

    // Go back to parent system
    mpModelWidget->getGraphicsView()->setScene(this->mpParentContainerObject->getContainedScenePtr());
    mpModelWidget->getGraphicsView()->setContainerPtr(this->mpParentContainerObject);

    mpModelWidget->setExternalSystem((mpParentContainerObject->isExternal() &&
                                           mpParentContainerObject != mpModelWidget->getTopLevelSystemContainer()) ||
                                           mpParentContainerObject->isAncestorOfExternalSubsystem());

    // Disconnect this system and connect parent system with undo and redo actions
    this->unmakeMainWindowConnectionsAndRefresh();
    mpParentContainerObject->makeMainWindowConnectionsAndRefresh();

    // Refresh external port appearance
    //! @todo We only need to do this if ports have change, right now we always refresh, dont know if this is a big deal
    this->refreshExternalPortsAppearanceAndPosition();
}


//! @brief Rotates all selected objects right (clockwise)
void ContainerObject::rotateSubObjects90cw()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit rotateSelectedObjectsRight();
    }
}


//! @brief Rotates all selected objects left (counter-clockwise)
void ContainerObject::rotateSubObjects90ccw()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit rotateSelectedObjectsLeft();
    }
}


//! @brief Flips selected contained objects horizontally
void ContainerObject::flipSubObjectsHorizontal()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit flipSelectedObjectsHorizontal();
    }
}


//! @brief Flips selected contained objects vertically
void ContainerObject::flipSubObjectsVertical()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit flipSelectedObjectsVertical();
    }
}


//! @brief Collects the plot data from the last simulation for all plot variables from the core and stores them locally.
void ContainerObject::collectPlotData(bool overWriteLastGeneration)
{
    mpLogDataHandler->collectPlotDataFromModel(overWriteLastGeneration);

    // Now collect plotdata from all subsystems
    ModelObjectMapT::iterator it;
    for (it=mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        if (it.value()->type() == SystemContainerType)
        {
            static_cast<ContainerObject*>(it.value())->collectPlotData();
        }
    }
}


void ContainerObject::showLosses(bool show)
{
    if(!show)
    {
        mLossesVisible=false;
        hideLosses();
        return;
    }


    QLabel *pInfoLabel = new QLabel(tr("This will calculate energy losses for each component."));
    pInfoLabel->setWordWrap(true);
    pInfoLabel->setFixedWidth(300);

    mpLossesDialog = new QDialog(gpMainWindow);
    mpLossesDialog->setWindowTitle("Calculate Losses");

    QGroupBox *pUnitGroupBox = new QGroupBox(mpLossesDialog);
    mpEnergyRadioButton = new QRadioButton(tr("Energy losses"));
    mpAvgPwrRadioButton = new QRadioButton(tr("Average power losses"));
    mpEnergyRadioButton->setChecked(true);
    QVBoxLayout *pUnitLayout = new QVBoxLayout;
    pUnitLayout->addWidget(mpEnergyRadioButton);
    pUnitLayout->addWidget(mpAvgPwrRadioButton);
    pUnitGroupBox->setLayout(pUnitLayout);

    QCheckBox *pIgnoreSmallLossesCheckBox = new QCheckBox("Ignore small losses in bar chart plot");
    pIgnoreSmallLossesCheckBox->setChecked(true);

    QLabel *pMinLossesLabel = new QLabel("Ignore losses smaller than ");
    QLabel *pMinLossesValue = new QLabel();
    QLabel *pMinLossesUnit = new QLabel("%");
    pMinLossesValue->setNum(5);
    mpMinLossesSlider = new QSlider(mpLossesDialog);
    mpMinLossesSlider->setOrientation(Qt::Horizontal);
    mpMinLossesSlider->setMinimum(0);
    mpMinLossesSlider->setMaximum(100);
    mpMinLossesSlider->setValue(5);
    connect(mpMinLossesSlider, SIGNAL(valueChanged(int)), pMinLossesValue, SLOT(setNum(int)));

    QHBoxLayout *pSliderLayout = new QHBoxLayout();
    pSliderLayout->addWidget(pMinLossesLabel);
    pSliderLayout->addWidget(mpMinLossesSlider);
    pSliderLayout->addWidget(pMinLossesValue);
    pSliderLayout->addWidget(pMinLossesUnit);

    connect(pIgnoreSmallLossesCheckBox, SIGNAL(toggled(bool)), pMinLossesLabel, SLOT(setEnabled(bool)));
    connect(pIgnoreSmallLossesCheckBox, SIGNAL(toggled(bool)), mpMinLossesSlider, SLOT(setEnabled(bool)));
    connect(pIgnoreSmallLossesCheckBox, SIGNAL(toggled(bool)), pMinLossesValue, SLOT(setEnabled(bool)));
    connect(pIgnoreSmallLossesCheckBox, SIGNAL(toggled(bool)), pMinLossesUnit, SLOT(setEnabled(bool)));

    QPushButton *pCancelButton = new QPushButton("Cancel");
    QPushButton *pNextButton = new QPushButton("Go!");

    //Toolbar
    QAction *pHelpAction = new QAction("Show Context Help", this);
    pHelpAction->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Help.png"));
    QToolBar *pToolBar = new QToolBar(mpLossesDialog);
    pToolBar->addAction(pHelpAction);

    QGridLayout *pLossesDialogLayout = new QGridLayout;
    pLossesDialogLayout->addWidget(pInfoLabel, 0, 0, 1, 3);
    pLossesDialogLayout->addWidget(pUnitGroupBox, 1, 0, 1, 3);
    pLossesDialogLayout->addWidget(pIgnoreSmallLossesCheckBox, 2, 0, 1, 3);
    pLossesDialogLayout->addLayout(pSliderLayout, 3, 0, 1, 3);
    pLossesDialogLayout->addWidget(pToolBar, 4, 0, 1, 1);
    pLossesDialogLayout->addWidget(pCancelButton, 4, 1, 1, 1);
    pLossesDialogLayout->addWidget(pNextButton, 4, 2, 1, 1);

    mpLossesDialog->setLayout(pLossesDialogLayout);

    mpLossesDialog->setPalette(gConfig.getPalette());

    mpLossesDialog->show();

    connect(pCancelButton, SIGNAL(clicked()), mpLossesDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), this, SLOT(showLossesFromDialog()));
    connect(pHelpAction, SIGNAL(triggered()), gpMainWindow, SLOT(openContextHelp()));
}


void ContainerObject::showLossesFromDialog()
{
    mpLossesDialog->close();
    mLossesVisible=true;

    bool usePower = mpAvgPwrRadioButton->isChecked();
    double time = this->mpLogDataHandler->getTimeVectorCopy(-1).last();

    double limit=0;
    if(mpMinLossesSlider->isEnabled())
    {
        limit=mpMinLossesSlider->value()/100.0;
    }

    //We should not be here if there is no plot data, but let's check to be sure
    if(mpLogDataHandler->isEmpty())
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Attempted to calculate losses for a model that has not been simulated (or is empty).");
        return;
    }

    //Calculate total losses in model
    double totalLosses=0;
    ModelObjectMapT::iterator moit;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        moit.value()->showLosses();
        double componentTotal;
        QMap<QString,double> componentDomainSpecific;
        moit.value()->getLosses(componentTotal, componentDomainSpecific);
        if(componentTotal > 0)
            totalLosses += componentTotal;
    }
    if(usePower) totalLosses /= time;

    //Count number of component that are to be plotted, and store their names
    int nComponents=0;
    QStringList componentNames;
    QList<double> componentLosses;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        double componentTotal;
        QMap<QString,double> componentDomainSpecific;
        moit.value()->getLosses(componentTotal, componentDomainSpecific);
        if(abs(componentTotal) > abs(limit*totalLosses))     //Condition for plotting
        {
            ++nComponents;
            componentNames.append(moit.value()->getName());
            componentLosses.append(componentTotal);
            if(usePower) componentLosses.last() /= time;
        }
    }

    QStringList lossesNames;
    QList<double> lossesValues;
    QStringList addedNames;
    QList<double> addedValues;
    for(int i=0; i<componentLosses.size(); ++i)
    {
        if(componentLosses.at(i) > 0)
        {
            lossesNames.append(componentNames.at(i));
            lossesValues.append(componentLosses.at(i));
        }
        else
        {
            addedNames.append(componentNames.at(i));
            addedValues.append(componentLosses.at(i));
        }
    }

    //Sort losses for plot (bubblesort)
    int i,j;
    for(i=0; i<lossesValues.size(); i++)
    {
        for(j=0;j<i;j++)
        {
            if(fabs(lossesValues[i])>fabs(lossesValues[j]))
            {
                double temp=lossesValues[i];
                lossesValues[i]=lossesValues[j];
                lossesValues[j]=temp;

                QString temp2 = lossesNames[i];
                lossesNames[i]=lossesNames[j];
                lossesNames[j]=temp2;
            }
        }
    }
    for(i=0; i<addedValues.size(); i++)
    {
        for(j=0;j<i;j++)
        {
            if(fabs(addedValues[i])>fabs(addedValues[j]))
            {
                double temp=addedValues[i];
                addedValues[i]=addedValues[j];
                addedValues[j]=temp;

                QString temp2 = addedNames[i];
                addedNames[i]=addedNames[j];
                addedNames[j]=temp2;
            }
        }
    }

    if(addedNames.isEmpty() && lossesNames.isEmpty())
    {
        gpMainWindow->mpShowLossesAction->setChecked(false);
        return;     //Don't attempt to plot when there is nothing to plot (may cause crash in bar chart plotter)
    }

    componentNames.clear();
    componentNames.append(addedNames);
    componentNames.append(lossesNames);

    componentLosses.clear();
    componentLosses.append(addedValues);
    componentLosses.append(lossesValues);

    //Create item model, containing data for bar chart plot
    QStandardItemModel *pItemModel = new QStandardItemModel(2,nComponents,this);
    pItemModel->setHeaderData(0, Qt::Vertical, QColor("crimson"), Qt::BackgroundRole);
    pItemModel->setHeaderData(1, Qt::Vertical, QColor("forestgreen"), Qt::BackgroundRole);

    //Add data to plot bars from each component
    for(int c=0; c<componentLosses.size(); ++c)
    {
        if(abs(componentLosses.at(c)) > abs(limit*totalLosses))
        {
            if(componentLosses.at(c) > 0)
                pItemModel->setData(pItemModel->index(0,c), componentLosses.at(c));
            else
                pItemModel->setData(pItemModel->index(1,c), -componentLosses.at(c));
        }
    }

    pItemModel->setVerticalHeaderLabels(QStringList() << "Added" << "Losses");
    pItemModel->setHorizontalHeaderLabels(componentNames);

    //! @todo meamory leak, should use PlotHandler instead
    PlotWindow *pPlotWindow = new PlotWindow("Energy Losses", gpMainWindow);
    pPlotWindow->getCurrentPlotTab()->setTabName("Energy Losses");
    pPlotWindow->addBarChart(pItemModel);
    pPlotWindow->show();
}


void ContainerObject::hideLosses()
{
    ModelObjectMapT::iterator moit;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        moit.value()->hideLosses();
    }
}


void ContainerObject::measureSimulationTime()
{
    int nSteps = QInputDialog::getInt(gpMainWindow, tr("Measure Simulation Time"),
                                 tr("Number of steps:"), 5, 1);

    QStringList names;
    QList<double> times;
    getCoreSystemAccessPtr()->measureSimulationTime(names, times, nSteps);

    //Use component-wise results to generate lists for type-wise results
    QStringList typeNames;
    QList<double> typeTimes;
    QList<int> typeCounter;
    for(int n=0; n<names.size(); ++n)
    {
        QString typeName = getModelObject(names[n])->getTypeName();
        if(typeNames.contains(typeName))
        {
            int i=typeNames.indexOf(typeName);
            typeCounter[i] = typeCounter[i]+1;
            typeTimes[i] = (typeTimes[i] + times[n]/(double(nSteps)*(typeCounter[i]-1)))*(typeCounter[i]-1)/(typeCounter[i]);
        }
        else
        {
            typeNames.append(typeName);
            typeTimes.append(times[n]/double(nSteps));
            typeCounter.append(1);
        }
    }

    qDebug() << "typeNames: " << typeNames;
    qDebug() << "typeTimes: " << typeTimes;

    QList<QStandardItem *> nameList;
    QList<QStandardItem *> timeList;
    for(int i=0; i<names.size(); ++i)
    {
        //! @todo are these ever deleted, and others below
        QStandardItem *pNameItem = new QStandardItem(names.at(i));
        QStandardItem *pTimeItem = new QStandardItem(QString::number(times.at(i)*1000, 'f')+" ms");
        nameList.append(pNameItem);
        timeList.append(pTimeItem);
    }

    QList<QStandardItem *> typeNameList;
    QList<QStandardItem *> typeTimeList;
    for(int i=0; i<typeNames.size(); ++i)
    {
        QStandardItem *pNameItem = new QStandardItem(typeNames.at(i));
        QStandardItem *pTimeItem = new QStandardItem(QString::number(typeTimes.at(i)*1000, 'f')+" ms");
        typeNameList.append(pNameItem);
        typeTimeList.append(pTimeItem);
    }

    QStandardItemModel *pComponentModel = new QStandardItemModel();
    pComponentModel->insertColumn(0, nameList);
    pComponentModel->insertColumn(1, timeList);
    QStandardItem *pComponentNameHeaderItem = new QStandardItem("Component Name");
    pComponentModel->setHorizontalHeaderItem(0, pComponentNameHeaderItem);
    QStandardItem *pComponentTimeHeaderItem = new QStandardItem("Time");
    pComponentModel->setHorizontalHeaderItem(1, pComponentTimeHeaderItem);

    QStandardItemModel *pTypeModel = new QStandardItemModel();
    pTypeModel->insertColumn(0, typeNameList);
    pTypeModel->insertColumn(1, typeTimeList);
    QStandardItem *pTypeNameHeaderItem = new QStandardItem("Component Type");
    pTypeModel->setHorizontalHeaderItem(0, pTypeNameHeaderItem);
    QStandardItem *pTypeTimeHeaderItem = new QStandardItem("Times");
    pTypeModel->setHorizontalHeaderItem(1, pTypeTimeHeaderItem);

    QDialog *pDialog = new QDialog(gpMainWindow);
    pDialog->setWindowTitle("Simulation Time Measurements");
    pDialog->setWindowModality(Qt::WindowModal);
    pDialog->setWindowIcon(QIcon(QString(ICONPATH)+"Hopsan-MeasureSimulationTime.png"));

    QLabel *pDescriptionLabel = new QLabel("The simulation time for each component is measured as the average simulation time over specified number of time steps. Results may differ slightly each measurement due to external factors such as other processes on the computer.");
    pDescriptionLabel->setWordWrap(true);

    mpComponentTable = new QTableView(pDialog);
    mpComponentTable->setModel(pComponentModel);
    mpComponentTable->setColumnWidth(0,400);
    mpComponentTable->setColumnWidth(1,200);
    mpComponentTable->setSortingEnabled(true);
    mpComponentTable->setAlternatingRowColors(true);
    mpComponentTable->verticalHeader()->setVisible(false);
    mpComponentTable->setVisible(false);
    mpComponentTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    mpTypeTable = new QTableView(pDialog);
    mpTypeTable->setModel(pTypeModel);
    mpTypeTable->setColumnWidth(0,400);
    mpTypeTable->setColumnWidth(1,200);
    mpTypeTable->setSortingEnabled(true);
    mpTypeTable->setAlternatingRowColors(true);
    mpTypeTable->verticalHeader()->setVisible(false);
    mpTypeTable->setVisible(true);
    mpTypeTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QGroupBox *pHowToShowResultsGroupBox = new QGroupBox(pDialog);
    QRadioButton *pTypeRadioButton = new QRadioButton(tr("Show results for component types"));
    QRadioButton *pComponentRadioButton = new QRadioButton(tr("Show results for individual components"));
    pTypeRadioButton->setChecked(true);
    QVBoxLayout *pHowToShowResultsLayout = new QVBoxLayout;
    pHowToShowResultsLayout->addWidget(pTypeRadioButton);
    pHowToShowResultsLayout->addWidget(pComponentRadioButton);
    //pHowToShowResultsLayout->addStretch(1);
    pHowToShowResultsGroupBox->setLayout(pHowToShowResultsLayout);

    QPushButton *pDoneButton = new QPushButton("Done", pDialog);
    QPushButton *pChartButton = new QPushButton("Show Bar Chart", pDialog);
    pChartButton->setCheckable(true);
    pChartButton->setChecked(false);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDialog);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::AcceptRole);
    pButtonBox->addButton(pChartButton, QDialogButtonBox::ActionRole);

    QVBoxLayout *pLayout = new QVBoxLayout(pDialog);
    pLayout->addWidget(pHowToShowResultsGroupBox);
    pLayout->addWidget(pDescriptionLabel);
    pLayout->addWidget(mpComponentTable);
    pLayout->addWidget(mpTypeTable);
    pLayout->addWidget(pButtonBox);

    connect(pTypeRadioButton, SIGNAL(toggled(bool)), mpTypeTable, SLOT(setVisible(bool)));
    connect(pComponentRadioButton, SIGNAL(toggled(bool)), mpComponentTable, SLOT(setVisible(bool)));
    connect(pDoneButton, SIGNAL(clicked()), pDialog, SLOT(close()));
    //connect(pChartButton, SIGNAL(toggled(bool)), pPlotWindow, SLOT(setVisible(bool)));
    connect(pChartButton, SIGNAL(clicked()), this, SLOT(plotMeasuredSimulationTime()));

    pDialog->setLayout(pLayout);
    pDialog->show();
    qApp->processEvents();
    qDebug() << mpComponentTable->size();
    pDialog->setFixedSize(640, 480);
    pDialog->adjustSize();
    pDialog->setModal(false);
    pDialog->exec();


    delete(pDialog);
}

void ContainerObject::plotMeasuredSimulationTime()
{
    QItemSelectionModel *pSelect;
    QStandardItemModel *pModel;
    if(mpTypeTable->isVisible())
    {
        pSelect = mpTypeTable->selectionModel();
        pModel = qobject_cast<QStandardItemModel*>(mpTypeTable->model());
    }
    else
    {
        pSelect = mpComponentTable->selectionModel();
        pModel = qobject_cast<QStandardItemModel*>(mpComponentTable->model());
    }


    QStringList typeNames;
    QList<double> typeTimes;
    if(!pSelect->selectedRows().isEmpty())
    {
        for(int i=0; i<pSelect->selectedRows().size(); ++i)
        {
            typeNames.append(pModel->data(pSelect->selectedRows(0)[i]).toString());
            typeTimes.append(pModel->data(pSelect->selectedRows(1)[i]).toString().remove(" ms").toDouble());
        }
    }
    else
    {
        for(int i=0; i<pModel->rowCount(); ++i)
        {
            typeNames.append(pModel->item(i,0)->data(Qt::DisplayRole).toString());
            typeTimes.append(pModel->item(i,1)->data(Qt::DisplayRole).toString().remove(" ms").toDouble());
        }
    }

    //Bar chart model for typenames
    QStandardItemModel *pBarChartModel = new QStandardItemModel(1,typeNames.size(),this);
    pBarChartModel->setHeaderData(0, Qt::Vertical, QColor("crimson"), Qt::BackgroundRole);
    for(int i=0; i<typeNames.size(); ++i)
    {
        pBarChartModel->setData(pBarChartModel->index(0,i), typeTimes[i]);
    }
    pBarChartModel->setVerticalHeaderLabels(QStringList() << "Time");
    pBarChartModel->setHorizontalHeaderLabels(typeNames);

    //Plot window for typename bar charts
    PlotWindow *pPlotWindow = new PlotWindow("Time measurements", gpMainWindow);
    pPlotWindow->getCurrentPlotTab()->setTabName("Time measurements");
    pPlotWindow->addBarChart(pBarChartModel);
    pPlotWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    pPlotWindow->show();
}


bool ContainerObject::isAncestorOfExternalSubsystem()
{
    if(this == mpModelWidget->getTopLevelSystemContainer())
    {
        return false;
    }
    else if(this->isExternal())
    {
        return true;
    }
    else
    {
        return mpParentContainerObject->isAncestorOfExternalSubsystem();
    }
}


bool ContainerObject::isExternal()
{
    return !mModelFileInfo.filePath().isEmpty();
}



LogDataHandler *ContainerObject::getLogDataHandler()
{
    return mpLogDataHandler;
}


void ContainerObject::setLogDataHandler(LogDataHandler *pHandler)
{
    mpLogDataHandler = pHandler;
    mpLogDataHandler->setParent(this);
}

