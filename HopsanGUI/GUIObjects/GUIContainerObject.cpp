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
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <cassert>

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIComponent.h"
#include "GUIConnector.h"
#include "GUIContainerObject.h"
#include "GUIContainerPort.h"
#include "GUIPort.h"
#include "GUIWidgets.h"
#include "loadFunctions.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HelpPopUpWidget.h"
#include "version_gui.h"
#include "Widgets/HcomWidget.h"
#include "LibraryHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/PlotWidget2.h"
#include "Widgets/QuickNavigationWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/DataExplorer.h"
#include "Widgets/FindWidget.h"
#include "Widgets/MessageWidget.h"
#include "PlotHandler.h"
#include "Utilities/HelpPopUpWidget.h"
#include "GeneratorUtils.h"

//! @brief Constructor for container objects.
//! @param position Initial position where container object is to be placed in its parent container
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to the appearance data object
//! @param startSelected Tells whether or not the object is initially selected
//! @param gfxType Tells whether the initial graphics shall be user or ISO
//! @param pParentSystem Pointer to the parent container object (leave empty if not a sub container)
//! @param pParent Pointer to parent object
SystemObject::SystemObject(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType, SystemObject *pParentSystem, QGraphicsItem *pParent)
        : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentSystem, pParent)
{
    mpModelWidget = pParentSystem->mpModelWidget;
    commonConstructorCode();
}

// Root system specific constructor
SystemObject::SystemObject(ModelWidget *parentModelWidget, QGraphicsItem *pParentGraphicsItem) :
    ModelObject(QPointF(0,0), 0, nullptr, SelectionStatusEnumT::Deselected,  GraphicsTypeEnumT::UserGraphics, nullptr, pParentGraphicsItem)
{
    const auto pDefaultSystemAppearance = gpLibraryHandler->getModelObjectAppearancePtr(HOPSANGUISYSTEMTYPENAME);
    mModelObjectAppearance = *pDefaultSystemAppearance; //This will crash if Subsystem not already loaded
    mpModelWidget = parentModelWidget;
    commonConstructorCode();
    mpUndoStack->newPost();
}

//! @brief This code is common among the two constructors, we use one function to avoid code duplication
void SystemObject::commonConstructorCode()
{
    // Initialize default values
    mIsCreatingConnector = false;
    mShowSubComponentPorts = gpMainWindow->mpTogglePortsAction->isChecked();
    mShowSubComponentNames = gpMainWindow->mpToggleNamesAction->isChecked();
    mSignalsVisible = gpMainWindow->mpToggleSignalsAction->isChecked();
    mLossesVisible = false;
    mUndoEnabled = true;
    mGfxType = UserGraphics;
    mLoadType = "EMBEDED";
    mNumberOfLogSamples = 2048;
    mLogStartTime = 0;
    mSaveUndoStack = false;       //Do not save undo stack by default
    mPasteOffset = -30;

    // Create the scene
    mpScene = new QGraphicsScene(this);
    mGraphicsViewPort = GraphicsViewPort(2500, 2500, 1.0); // Default should be centered

    // Create the undo stack
    mpUndoStack = new UndoStack(this);

    mpDragCopyStack = new CopyStack();

    // Establish connections that should always remain
    connect(this, SIGNAL(checkMessages()), gpMessageHandler, SLOT(collectHopsanCoreMessages()), Qt::UniqueConnection);

    // Connect propagation signal when alias is changed
    connect(this, SIGNAL(aliasChanged(QString,QString)), mpModelWidget, SIGNAL(aliasChanged(QString,QString)));

    // Create the object in core, and update name
    if (this->mpParentSystemObject == 0)
    {
        //Create root system
        qDebug() << "creating ROOT access system";
        mpCoreSystemAccess = new CoreSystemAccess();
        this->setName("RootSystem");
        //qDebug() << "the core root system name: " << mpCoreSystemAccess->getRootSystemName();
    }
    else
    {
        //Create subsystem
        qDebug() << "creating subsystem and setting name in " << mpParentSystemObject->getCoreSystemAccessPtr()->getSystemName();
        if(this->getTypeName() == HOPSANGUICONDITIONALSYSTEMTYPENAME)
        {
            mName = mpParentSystemObject->getCoreSystemAccessPtr()->createConditionalSubSystem(this->getName());
        }
        else
        {
            mName = mpParentSystemObject->getCoreSystemAccessPtr()->createSubSystem(this->getName());
        }
        refreshDisplayName();
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentSystemObject->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentSystemObject->getCoreSystemAccessPtr());
    }

    if(!isTopLevelContainer())
    {
        refreshAppearance();
        refreshExternalPortsAppearanceAndPosition();
        refreshDisplayName(); //Make sure name window is correct size for center positioning
    }

    if(mpParentSystemObject)
    {
        connect(mpParentSystemObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisibleIfSignal(bool)));
    }
}


//! @brief Destructor for container object
SystemObject::~SystemObject()
{
    //qDebug() << ",,,,,,,,,,,,GUIContainer destructor";
}

bool SystemObject::isTopLevelContainer() const
{
    return (mpParentSystemObject==0);
}

QStringList SystemObject::getSystemNameHieararchy() const
{
    QStringList parentSystemNames;
    // Note! This wil lreturn empty lsit for top-level system, and that is OK, it is supposed to do that
    if (mpParentSystemObject)
    {
        parentSystemNames = mpParentSystemObject->getSystemNameHieararchy();
        parentSystemNames << this->getName();
    }
    return parentSystemNames;
}

//! @brief Notify the parent project tab that changes has occurred
void SystemObject::hasChanged()
{
    mpModelWidget->hasChanged();
}

//! @brief Connects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are switching what container we want the buttons to trigger actions in
void SystemObject::makeMainWindowConnectionsAndRefresh()
{
    connect(gpMainWindow->mpUndoAction, SIGNAL(triggered()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRedoAction, SIGNAL(triggered()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getUndoButton(),  SIGNAL(clicked()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getRedoButton(),  SIGNAL(clicked()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()), Qt::UniqueConnection);

    connect(gpMainWindow->mpTogglePortsAction,    SIGNAL(triggered(bool)),    this,     SLOT(showSubcomponentPorts(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleNamesAction,    SIGNAL(triggered(bool)),    this,     SLOT(toggleNames(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleSignalsAction,  SIGNAL(triggered(bool)),    this,     SLOT(toggleSignals(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpEnableUndoAction,     SIGNAL(triggered(bool)),    this,     SLOT(setUndoEnabled(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpCutAction,            SIGNAL(triggered()),        this,     SLOT(cutSelected()), Qt::UniqueConnection);
    connect(gpMainWindow->mpCopyAction,           SIGNAL(triggered()),        this,     SLOT(copySelected()), Qt::UniqueConnection);
    connect(gpMainWindow->mpPasteAction,          SIGNAL(triggered()),        this,     SLOT(paste()), Qt::UniqueConnection);
    connect(gpMainWindow->mpAlignXAction,         SIGNAL(triggered()),        this,     SLOT(alignX()), Qt::UniqueConnection);
    connect(gpMainWindow->mpAlignYAction,         SIGNAL(triggered()),        this,     SLOT(alignY()), Qt::UniqueConnection);
    connect(gpMainWindow->mpDistributeXAction,    SIGNAL(triggered()),        this,     SLOT(distributeX()), Qt::UniqueConnection);
    connect(gpMainWindow->mpDistributeYAction,    SIGNAL(triggered()),        this,     SLOT(distributeY()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRotateRightAction,    SIGNAL(triggered()),        this,     SLOT(rotateSubObjects90cw()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRotateLeftAction,     SIGNAL(triggered()),        this,     SLOT(rotateSubObjects90ccw()), Qt::UniqueConnection);
    connect(gpMainWindow->mpFlipHorizontalAction, SIGNAL(triggered()),        this,     SLOT(flipSubObjectsHorizontal()), Qt::UniqueConnection);
    connect(gpMainWindow->mpFlipVerticalAction,   SIGNAL(triggered()),        this,     SLOT(flipSubObjectsVertical()), Qt::UniqueConnection);
    connect(gpMainWindow->mpPropertiesAction,     SIGNAL(triggered()),        this,     SLOT(openPropertiesDialogSlot()), Qt::UniqueConnection);

    // Update the main window menu and toolbar actions that are system specific
    gpMainWindow->mpTogglePortsAction->setChecked(mShowSubComponentPorts);
    gpMainWindow->mpToggleNamesAction->setChecked(mShowSubComponentNames);
    gpMainWindow->mpToggleSignalsAction->setChecked(mSignalsVisible);
    gpMainWindow->mpEnableUndoAction->setChecked(mUndoEnabled);
    gpMainWindow->mpUndoAction->setEnabled(mUndoEnabled);
    gpMainWindow->mpRedoAction->setEnabled(mUndoEnabled);
    gpMainWindow->mpAnimateAction->setDisabled(mAnimationDisabled);

    // Update main window widgets with data from this container
    gpFindWidget->setContainer(this);
    gpMainWindow->mpSystemParametersWidget->update(this);
    gpUndoWidget->refreshList();
}

//! @brief Disconnects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are switching what container we want the buttons to trigger actions in
void SystemObject::unmakeMainWindowConnectionsAndRefresh()
{
    // Update Systemparameter widget to have no contents, but only if this is the system that is actually represented
    if (gpMainWindow->mpSystemParametersWidget->getRepresentedContainerObject() == this)
    {
        gpMainWindow->mpSystemParametersWidget->update(0);
    }

    disconnect(gpMainWindow->mpUndoAction, SIGNAL(triggered()), this, SLOT(undo()));
    disconnect(gpMainWindow->mpRedoAction, SIGNAL(triggered()), this, SLOT(redo()));
    disconnect(gpUndoWidget->getUndoButton(),  SIGNAL(clicked()), this, SLOT(undo()));
    disconnect(gpUndoWidget->getRedoButton(),  SIGNAL(clicked()), this, SLOT(redo()));
    disconnect(gpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()));

    disconnect(gpMainWindow->mpToggleNamesAction,     SIGNAL(triggered(bool)),    this,    SLOT(toggleNames(bool)));
    disconnect(gpMainWindow->mpTogglePortsAction,     SIGNAL(triggered(bool)),    this,    SLOT(showSubcomponentPorts(bool)));
    disconnect(gpMainWindow->mpToggleSignalsAction,   SIGNAL(triggered(bool)),    this,    SLOT(toggleSignals(bool)));
    disconnect(gpMainWindow->mpEnableUndoAction,      SIGNAL(triggered(bool)),    this,    SLOT(setUndoEnabled(bool)));
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
//! @param[in] pt The position of this object, used to determine the center relative position
//! @returns An enum that indicates on which side the port should be placed
SystemObject::ContainerEdgeEnumT SystemObject::findPortEdge(QPointF center, QPointF pt)
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
    double angle = normRad(qAtan2(diff.x(), diff.y()));
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

//! @brief Refreshes the appearance and position of all external ports
void SystemObject::refreshExternalPortsAppearanceAndPosition()
{
    //refresh the external port poses
    ModelObjectMapT::iterator moit;
    double val;

    //Set the initial values to be overwritten by the if bellow
    double xMin=std::numeric_limits<double>::max(), xMax=-xMin, yMin=xMin, yMax=xMax;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        //check x max and min
        val = moit.value()->getCenterPos().x();
        xMin = std::min(xMin,val);
        xMax = std::max(xMax,val);
        //check y max and min
        val = moit.value()->getCenterPos().y();
        yMin = std::min(yMin,val);
        yMax = std::max(yMax,val);
    }
    //! @todo Find out if it is possible to ask the scene or view for this information instead of calculating it ourselves
    QPointF center = QPointF((xMax+xMin)/2.0, (yMax+yMin)/2.0);
    //qDebug() << "center max min: " << center << " " << xMin << " " << xMax << " " << yMin << " " << yMax;

    QMap<double, Port*> leftEdge, rightEdge, topEdge, bottomEdge;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == SystemPortObjectType)
        {
            //            QLineF line = QLineF(center, moit.value()->getCenterPos());
            //            this->getContainedScenePtr()->addLine(line); //debug-grej

            ContainerEdgeEnumT edge = findPortEdge(center, moit.value()->getCenterPos());
            //qDebug() << " sysp: " << moit.value()->getName() << " edge: " << edge;

            //Make sure we don't screw up in the code and forget to rename or create external ports on internal rename or create
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
                SharedPortAppearanceT pPA = mModelObjectAppearance.getPortAppearanceMap().value(pPort->getName());
                if (pPA)
                {
                    pPort->setCenterPosByFraction(pPA->x, pPA->y);
                    pPort->setRotation(pPA->rot);
                }
                this->createRefreshExternalPort(moit.value()->getName()); //Refresh for ports that are not autoplaced
            }
        }
    }

    //Now disperse the port icons evenly along each edge
    QMap<double, Port*>::iterator it;
    double disp;  //Dispersion factor
    double sdisp; //sumofdispersionfactors

    //! @todo weird to use createfunction to refresh graphics, but ok for now
    disp = 1.0/((double)(rightEdge.size()+1));
    sdisp=disp;
    for (it=rightEdge.begin(); it!=rightEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(1.0, sdisp);
        it.value()->setRotation(0);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((double)(bottomEdge.size()+1));
    sdisp=disp;
    for (it=bottomEdge.begin(); it!=bottomEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(sdisp, 1.0);
        it.value()->setRotation(90);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((double)(leftEdge.size()+1));
    sdisp=disp;
    for (it=leftEdge.begin(); it!=leftEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(0.0, sdisp);
        it.value()->setRotation(180);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }

    disp = 1.0/((double)(topEdge.size()+1));
    sdisp=disp;
    for (it=topEdge.begin(); it!=topEdge.end(); ++it)
    {
        it.value()->setCenterPosByFraction(sdisp, 0.0);
        it.value()->setRotation(270);
        this->createRefreshExternalPort(it.value()->getName());    //refresh the external port graphics
        sdisp += disp;
    }


    PortAppearanceMapT::Iterator itp;
    for(itp=mModelObjectAppearance.getPortAppearanceMap().begin(); itp != mModelObjectAppearance.getPortAppearanceMap().end(); ++itp)
    {
        createRefreshExternalPort(itp.key());
    }
    redrawConnectors();
}

//! @brief Overloaded refreshAppearance for containers, to make sure that port positions are updated if graphics size is changed
void SystemObject::refreshAppearance()
{
    ModelObject::refreshAppearance();
    this->refreshExternalPortsAppearanceAndPosition();
}

//! @brief Use this function to calculate the placement of the ports on a subsystem icon.
//! @param[in] w width of the subsystem icon
//! @param[in] h height of the subsystem icon
//! @param[in] angle the angle in radians of the line between center and the actual port
//! @param[out] x the new calculated horizontal placement for the port
//! @param[out] y the new calculated vertical placement for the port
//! @todo rename this one and maybe change it a bit as it is now included in this class, it should be common for subsystems and groups
void SystemObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
{
    //! @todo make common PI declaration, maybe also PIhalf or include math.h and use M_PI
    double tanAngle = tan(angle);//Otherwise division by zero
    if(fabs(tanAngle) < 0.0001)
    {
        tanAngle=.0001;
    }
    if(angle>M_PI*3.0/2.0)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>M_PI)
    {
        x=-std::max(std::min(h/tanAngle, w), -w);
        y=-std::max(std::min(w*tanAngle, h), -h);
    }
    else if(angle>M_PI/2.0)
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


//! @brief Returns a pointer to the contained scene
QGraphicsScene *SystemObject::getContainedScenePtr()
{
    return mpScene;
}

void SystemObject::setGraphicsViewport(GraphicsViewPort vp)
{
    mGraphicsViewPort = vp;
}

GraphicsViewPort SystemObject::getGraphicsViewport() const
{
    return mGraphicsViewPort;
}


//! @brief This method creates ONE external port. Or refreshes existing ports. It assumes that port appearance information for this port exists
//! @param[portName] The name of the port to create
//! @todo maybe default create that info if it is missing
//! @todo massive duplicate implementation with the one in modelobject
Port *SystemObject::createRefreshExternalPort(QString portName)
{
    // If port appearance is not already existing then we create it
    if ( !mModelObjectAppearance.getPortAppearanceMap().contains(portName) )
    {
        mModelObjectAppearance.addPortAppearance(portName);
    }

    // Fetch appearance data
    Port *pPort=0;
    SharedPortAppearanceT pPA = mModelObjectAppearance.getPortAppearanceMap().value(portName);
    if (pPA)
    {
        // Create new external port if it does not already exist (this is the usual case for individual components)
        pPort = this->getPort(portName);
        if ( pPort == 0 )
        {
            qDebug() << "##This is OK though as this means that we should create the stupid port for the first time";

            //! @todo to minimize search time make a get porttype  and nodetype function, we need to search twice now
            QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(portName, portName);
            QString portType = this->getCoreSystemAccessPtr()->getPortType(portName, portName);
            pPA->selectPortIcon(getTypeCQS(), portType, nodeType);

            double x = pPA->x;
            double y = pPA->y;
            //qDebug() << "x,y: " << x << " " << y;

            pPort = new Port(portName, x*boundingRect().width(), y*boundingRect().height(), pPA, this);

            mPortListPtrs.append(pPort);

            pPort->refreshPortGraphics();
        }
        else
        {

            // The external port already seems to exist, lets update it in case something has changed
            //! @todo Maybe need to have a refresh port appearance function, don't really know if this will ever be used though, will fix when it becomes necessary
            pPort->refreshPortGraphics();

            // In this case of container object, also refresh any attached connectors, if types have changed
            //! @todo we always update, maybe we should be more smart and only update if changed, but I think this should be handled inside the connector class (the smartness)
            QVector<Connector*> connectors = pPort->getAttachedConnectorPtrs();
            for (int i=0; i<connectors.size(); ++i)
            {
                connectors[i]->refreshConnectorAppearance();
            }

            qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << portName << " in: " << this->getName();
        }
    }

    return pPort;
}




//! @brief Renames an external GUIPort
//! @param[in] oldName The name to be replaced
//! @param[in] newName The new name
//! This function assumes that oldName exist and that newName is correct, no error checking is done
void SystemObject::renameExternalPort(const QString oldName, const QString newName)
{
    QList<Port*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == oldName )
        {
            //Rename the port appearance data by remove and re-add
            SharedPortAppearanceT tmp = mModelObjectAppearance.getPortAppearanceMap().value(oldName);
            mModelObjectAppearance.erasePortAppearance(oldName);
            mModelObjectAppearance.addPortAppearance(newName, tmp);

            //Rename port
            (*plit)->setDisplayName(newName);
            break;
        }
    }
}


//! @brief Helper function that allows calling addGUIModelObject with typeName instead of appearance data
ModelObject* SystemObject::addModelObject(QString fullTypeName, QPointF position, double rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
    ModelObjectAppearance *pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(fullTypeName).data();

    if(!pAppearanceData)    //Not an existing component
    {
        return 0;       //No error message here, it depends on from where this function is called
    }
    else if(!pAppearanceData->getHmfFile().isEmpty())
    {
        QString hmfFile = pAppearanceData->getHmfFile();
        QString subTypeName = pAppearanceData->getSubTypeName();
        SystemObject *pObj = dynamic_cast<SystemObject*>(addModelObject("Subsystem", position, rotation, startSelected, nameStatus, undoSettings));
        pObj->setSubTypeName(subTypeName);
        //pObj->clearContents();

        QFile file(pAppearanceData->getBasePath()+hmfFile);

        QDomDocument domDocument;
        QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, hmf::root);
        if (!hmfRoot.isNull())
        {
            //! @todo Check version numbers
            //! @todo check if we could load else give error message and don't attempt to load
            QDomElement systemElement = hmfRoot.firstChildElement(hmf::system);
            pObj->setModelFileInfo(file); //Remember info about the file from which the data was loaded
            QFileInfo fileInfo(file);
            pObj->setAppearanceDataBasePath(fileInfo.absolutePath());
            pObj->loadFromDomElement(systemElement);
            pObj->setIconPath(pAppearanceData->getIconPath(UserGraphics, Absolute), UserGraphics, Absolute);
            pObj->getAppearanceData()->setIconScale(pAppearanceData->getIconScale(UserGraphics), UserGraphics);
            pObj->refreshAppearance();
            return pObj;
        }
    }
    return addModelObject(pAppearanceData, position, rotation, startSelected, nameStatus, undoSettings);
}


//! @brief Creates and adds a GuiModel Object to the current container
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
//! @returns a pointer to the created and added object
//! @todo only modelobjects for now
ModelObject* SystemObject::addModelObject(ModelObjectAppearance *pAppearanceData, QPointF position, double rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
    // Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();

    ModelObject* pNewModelObject = nullptr;
    QString componentTypeName = pAppearanceData->getTypeName();
    if (componentTypeName == HOPSANGUISYSTEMTYPENAME || componentTypeName == HOPSANGUICONDITIONALSYSTEMTYPENAME)
    {
        pNewModelObject = new SystemObject(position, rotation, pAppearanceData, startSelected, mGfxType, this);
    }
    else if (componentTypeName == HOPSANGUISYSTEMPORTTYPENAME)
    {
        // We must create internal port FIRST before external one
        pNewModelObject = new SystemPortObject(position, rotation, pAppearanceData, this, startSelected, mGfxType);
        this->addExternalSystemPortObject(pNewModelObject);
        this->refreshExternalPortsAppearanceAndPosition();
    }
    else if (componentTypeName == HOPSANGUISCOPECOMPONENTTYPENAME)
    {
        pNewModelObject = new ScopeComponent(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }
    else //Assume some standard component type
    {
        pNewModelObject = new Component(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }

    emit checkMessages();

    mModelObjectMap.insert(pNewModelObject->getName(), pNewModelObject);

    if(undoSettings == Undo)
    {
        mpUndoStack->registerAddedObject(pNewModelObject);
    }

    pNewModelObject->setSelected(false);
    pNewModelObject->setSelected(true);

    if(nameStatus == NameVisible)
    {
        pNewModelObject->showName(NoUndo);
    }
    else if(nameStatus == NameNotVisible)
    {
        pNewModelObject->hideName(NoUndo);
    }
    else if (nameStatus == UseDefault)
    {
        if (areSubComponentNamesShown())
        {
            pNewModelObject->showName(NoUndo);
        }
        else
        {
            pNewModelObject->hideName(NoUndo);
        }
    }

    return pNewModelObject;
}


bool SystemObject::areLossesVisible()
{
    return mLossesVisible;
}


TextBoxWidget *SystemObject::addTextBoxWidget(QPointF position, UndoStatusEnumT undoSettings)
{
    return addTextBoxWidget(position, 0, undoSettings);
}

TextBoxWidget *SystemObject::addTextBoxWidget(QPointF position, int desiredWidgetId, UndoStatusEnumT undoSettings)
{
    TextBoxWidget *pNewTextBoxWidget;
    if (mWidgetMap.contains(desiredWidgetId)) {
        desiredWidgetId = mWidgetMap.keys().last()+1;
    }
    constexpr double angle = 0;
    pNewTextBoxWidget = new TextBoxWidget("Text", position, angle, Deselected, this, desiredWidgetId);
    mWidgetMap.insert(pNewTextBoxWidget->getWidgetIndex(), pNewTextBoxWidget);

    if(undoSettings == Undo) {
        mpUndoStack->registerAddedWidget(pNewTextBoxWidget);
    }
    mpModelWidget->hasChanged();

    return pNewTextBoxWidget;
}


ImageWidget *SystemObject::addImageWidget(QPointF position, UndoStatusEnumT undoSettings)
{
    return addImageWidget(position, 0, undoSettings);
}

ImageWidget *SystemObject::addImageWidget(QPointF position, int desiredWidgetId, UndoStatusEnumT undoSettings)
{
    ImageWidget *pNewImageWidget;
    if (mWidgetMap.contains(desiredWidgetId)) {
        desiredWidgetId = mWidgetMap.keys().last()+1;
    }
    constexpr double angle = 0;
    pNewImageWidget = new ImageWidget(position, angle, Deselected, this, desiredWidgetId);
    mWidgetMap.insert(pNewImageWidget->getWidgetIndex(), pNewImageWidget);

    if(undoSettings == Undo) {
        mpUndoStack->registerAddedWidget(pNewImageWidget);
    }
    mpModelWidget->hasChanged();

    return pNewImageWidget;
}


//! @brief Removes specified widget
//! Works for both text and box widgets
//! @param pWidget Pointer to widget to remove
//! @param undoSettings Tells whether or not this shall be registered in undo stack
void SystemObject::deleteWidget(Widget *pWidget, UndoStatusEnumT undoSettings)
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

void SystemObject::deleteWidget(const int id, UndoStatusEnumT undoSettings)
{
    Widget *pWidget = mWidgetMap.value(id, 0);
    if (pWidget)
    {
        deleteWidget(pWidget, undoSettings);
    }
}


//! @brief Delete ModelObject with specified name
//! @param rObjectName is the name of the component to delete
void SystemObject::deleteModelObject(const QString &rObjectName, UndoStatusEnumT undoSettings)
{
    ModelObjectMapT::iterator it = mModelObjectMap.find(rObjectName);
    if (it != mModelObjectMap.end())
    {
        ModelObject* pModelObject = it.value();

        // Remove connectors that are connected to the model object
        QList<Connector *> connectorPtrList = pModelObject->getConnectorPtrs();
        for(Connector *pConn : connectorPtrList)
        {
            removeSubConnector(pConn, undoSettings);
        }

        if ( (undoSettings == Undo) && mUndoEnabled)
        {
            //First save aliases for component to remove (if any)
            QStringList aliasesToSave;
            QStringList aliases = this->getAliasNames();
            for(const QString &alias : aliases) {
                QString fullName = getFullNameFromAlias(alias);
                QString compName = fullName.section("#",0,0);
                if(compName == pModelObject->getName()) {
                    aliasesToSave.append(alias);
                }
            }

            // Register removal of model object in undo stack
            this->mpUndoStack->registerDeletedObject(pModelObject);

            //Register the aliaes after the component, to make sure component is re-added before the aliases
            if(!aliasesToSave.isEmpty())
            {
                this->mpUndoStack->registerRemovedAliases(aliasesToSave);
            }
        }

        //! @todo maybe this should be handled somewhere else (not sure maybe this is the best place)
        if (pModelObject->type() == SystemPortObjectType )
        {
            removeExternalPort(pModelObject->getName());
        }

        mModelObjectMap.erase(it);
        mSelectedModelObjectsList.removeAll(pModelObject);
        mpScene->removeItem(pModelObject);
        pModelObject->deleteInHopsanCore();
        pModelObject->deleteLater();
    }
    else
    {
        gpMessageHandler->addErrorMessage("Could not delete object with name " + rObjectName + ", object not found");
    }
    emit checkMessages();
    mpModelWidget->getGraphicsView()->updateViewPort(); //!< @todo maybe handle by signal, and maybe have a bool in view that tells it to hold redraw (useful on multiple deletes like when closing a model)
}


//! @brief This function is used to rename a SubGUIObject
void SystemObject::renameModelObject(QString oldName, QString newName, UndoStatusEnumT undoSettings)
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
            ModelObject* pModelObject = it.value();
            mModelObjectMap.erase(it);

            // Set new name, first in core then in gui object
            switch (pModelObject->type())
            {
            case SystemPortObjectType : //!< @todo What will happen when we try to rename a groupport
                modNewName = this->getCoreSystemAccessPtr()->renameSystemPort(oldName, newName);
                renameExternalPort(oldName, modNewName);
                break;
            default :
                modNewName = this->getCoreSystemAccessPtr()->renameSubComponent(oldName, newName);
            }

            // Override the GUI ModelObject name with the new name from the core
            pModelObject->refreshDisplayName(modNewName);

            //Re-insert
            mModelObjectMap.insert(pModelObject->getName(), pModelObject);
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("No GUI Object with name: ") + oldName + " found when attempting rename!");
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
bool SystemObject::hasModelObject(const QString &rName) const
{
    return (mModelObjectMap.count(rName) > 0);
}

//! @brief Takes ownership of supplied objects, widgets and connectors
//!
//! This method assumes that the previous owner have forgotten all about these objects, it however sets itself as new Qtparent, parentContainer and scene, overwriting the old values
void SystemObject::takeOwnershipOf(QList<ModelObject*> &rModelObjectList, QList<Widget*> &rWidgetList)
{
    for (int i=0; i<rModelObjectList.size(); ++i)
    {
        //! @todo if a systemport is received we must update the external port list also, we cant handle such objects right now
        if (rModelObjectList[i]->type() != SystemPortObjectType)
        {
            this->getContainedScenePtr()->addItem(rModelObjectList[i]);
            rModelObjectList[i]->setParentSystemObject(this);
            mModelObjectMap.insert(rModelObjectList[i]->getName(), rModelObjectList[i]);
            //! @todo what if name already taken, don't care for now as we shall only move into groups when they are created

            //rModelObjectList[i]->refreshParentSystemSigSlotConnections();
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
        rWidgetList[i]->setParentSystemObject(this);
        mWidgetMap.insert(rWidgetList[i]->getWidgetIndex(), rWidgetList[i]);
        //! @todo what if idx already taken, don't care for now as we shall only move into groups when they are created
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

                    //! @todo for now we disconnect the transit connection as we are not yet capable of recreating the external connection
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
        ModelObject *pTransPort = this->addModelObject(HOPSANGUISYSTEMPORTTYPENAME, portpos.toPoint(),0);

        //Make previous parent container forget about the connector
        transitConnectors[i]->getParentContainer()->forgetSubConnector(transitConnectors[i]);

        //Add the port to this container
        this->getContainedScenePtr()->addItem(transitConnectors[i]);
        transitConnectors[i]->setParentContainer(this);
        mSubConnectorList.append(transitConnectors[i]);

        //! @todo instead of having set startport and set end port, (we can keep them also maybe), we should have a function that sets startport if no port is set and end port if start port already set, don't know if good idea but we can keep it in mind, then we would not have to do stuff like bellow. (maybe we could call that function "connect")
        if (endPortIsTransitPort)
        {
            //Make new port and connector know about each other
            transitConnectors[i]->setEndPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getEndPort()->getParentModelObject()->rememberConnector(transitConnectors[i]);
        }
        else
        {
            //Make new port and connector know about each other
            transitConnectors[i]->setStartPort(pTransPort->getPortListPtrs().at(0));
            transitConnectors[i]->getStartPort()->getParentModelObject()->rememberConnector(transitConnectors[i]);
        }

        this->refreshExternalPortsAppearanceAndPosition();
    }

    //! @todo do much more stuff

    //! @todo center the objects in the new view

}


//! @brief Notifies container object that a gui widget has been selected
void SystemObject::rememberSelectedWidget(Widget *widget)
{
    mSelectedWidgetsList.append(widget);
}


//! @brief Notifies container object that a gui widget is no longer selected
void SystemObject::forgetSelectedWidget(Widget *widget)
{
    mSelectedWidgetsList.removeAll(widget);
}


//! @brief Returns a list with pointers to the selected GUI widgets
QList<Widget *> SystemObject::getSelectedGUIWidgetPtrs()
{
    return mSelectedWidgetsList;
}


//! @brief Set a system parameter value
bool SystemObject::setParameterValue(QString name, QString value, bool force)
{
    const bool rc =  this->getCoreSystemAccessPtr()->setSystemParameterValue(name, value, force);
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

bool SystemObject::setParameter(const CoreParameterData &rParameter, bool force)
{
    const bool rc = this->getCoreSystemAccessPtr()->setSystemParameter(rParameter, false, force);
    emit checkMessages();
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

bool SystemObject::setOrAddParameter(const CoreParameterData &rParameter, bool force)
{
    const bool rc = this->getCoreSystemAccessPtr()->setSystemParameter(rParameter, true, force);
    emit checkMessages();
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

bool SystemObject::renameParameter(const QString oldName, const QString newName)
{
    const bool rc = this->getCoreSystemAccessPtr()->renameSystemParameter(oldName, newName);
    if (rc)
    {
        hasChanged();
        emit systemParametersChanged();
    }
    return rc;
}

void SystemObject::setNumHopScript(const QString &rScript)
{
    mNumHopScript = rScript;
    CoreSystemAccess *pCoreSys = getCoreSystemAccessPtr();
    if (pCoreSys)
    {
        pCoreSys->setNumHopScript(mNumHopScript);
    }
}

QString SystemObject::getNumHopScript() const
{
    return mNumHopScript;
}

void SystemObject::runNumHopScript(const QString &rScript, bool printOutput, QString &rOutput)
{
    CoreSystemAccess *pCoreSys = getCoreSystemAccessPtr();
    if (pCoreSys)
    {
        pCoreSys->runNumHopScript(rScript, printOutput, rOutput);
    }
}

//! @brief Notifies container object that a gui model object has been selected
void SystemObject::rememberSelectedModelObject(ModelObject *object)
{
    QString name = object->getName();
    if(mModelObjectMap.contains(name) && mModelObjectMap.find(name).value() == object)
        mSelectedModelObjectsList.append(object);
}


//! @brief Notifies container object that a gui model object is no longer selected
void SystemObject::forgetSelectedModelObject(ModelObject *object)
{
    mSelectedModelObjectsList.removeAll(object);
}


//! @brief Returns a list with pointers to the selected GUI model objects
QList<ModelObject *> SystemObject::getSelectedModelObjectPtrs()
{
    return mSelectedModelObjectsList;
}


//! @brief Returns a pointer to the component with specified name, 0 if not found
ModelObject *SystemObject::getModelObject(const QString &rModelObjectName)
{
    auto moit = mModelObjectMap.find(rModelObjectName);
    if (moit != mModelObjectMap.end())
    {
        return moit.value();
    }
    else
    {
        return nullptr;
    }
}

QList<ModelObject *> SystemObject::getModelObjects() const
{
    return mModelObjectMap.values();
}

//! @brief Get the port of a sub model object, returns 0 if modelobject or port not found
Port *SystemObject::getModelObjectPort(const QString modelObjectName, const QString portName)
{
    ModelObject *pModelObject = this->getModelObject(modelObjectName);
    if (pModelObject != nullptr)
    {
        return pModelObject->getPort(portName);
    }
    else
    {
        return nullptr;
    }
}


//! @brief Find a connector in the connector vector
Connector* SystemObject::findConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    Connector *pFoundConnector = nullptr;
    for(auto& pConnector : mSubConnectorList)
    {
        if((pConnector->getStartComponentName() == startComp) &&
           (pConnector->getStartPortName() == startPort) &&
           (pConnector->getEndComponentName() == endComp) &&
           (pConnector->getEndPortName() == endPort))
        {
            pFoundConnector = pConnector;
            break;
        }
        //Find even if the caller mixed up start and stop
        else if((pConnector->getStartComponentName() == endComp) &&
                (pConnector->getStartPortName() == endPort) &&
                (pConnector->getEndComponentName() == startComp) &&
                (pConnector->getEndPortName() == startPort))
        {
            pFoundConnector = pConnector;
            break;
        }
    }
    return pFoundConnector;
}


//! @brief Tells whether or not there is a connector between two specified ports
bool SystemObject::hasConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    return (findConnector(startComp, startPort, endComp, endPort) != nullptr);
}


//! @brief Notifies container object that a subconnector has been selected
void SystemObject::rememberSelectedSubConnector(Connector *pConnector)
{
    mSelectedSubConnectorsList.append(pConnector);
}


//! @brief Notifies container object that a subconnector has been deselected
void SystemObject::forgetSelectedSubConnector(Connector *pConnector)
{
    mSelectedSubConnectorsList.removeAll(pConnector);
}


//! @brief Removes a specified connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void SystemObject::removeSubConnector(Connector* pConnector, UndoStatusEnumT undoSettings)
{
    bool success=false;

    if (pConnector->isDangling() && !pConnector->isBroken())
    {
        cancelCreatingConnector();

        return;
    }

    // Make sure we can only remove connector that we own
    if (mSubConnectorList.contains(pConnector))
    {
        //! @todo some error handling both ports must exist and be connected to each other
        if(pConnector->isConnected())
        {
            Port *pStartP = pConnector->getStartPort();
            Port *pEndP = pConnector->getEndPort();

            // Volunector disconnect
            if ( pConnector->isVolunector() )
            {
                bool ok1 = this->getCoreSystemAccessPtr()->disconnect(pStartP->getParentModelObjectName(),
                                                                      pStartP->getName(),
                                                                      pConnector->getVolunectorComponent()->getName(),
                                                                      "P1");
                bool ok2 = this->getCoreSystemAccessPtr()->disconnect(pConnector->getVolunectorComponent()->getName(),
                                                                      "P2",
                                                                      pEndP->getParentModelObjectName(),
                                                                      pEndP->getName());

                Component *vComp = pConnector->getVolunectorComponent();
                vComp->scene()->removeItem(vComp);
                vComp->deleteInHopsanCore();
                vComp->deleteLater();

                success = (ok1 && ok2);
            }
            // Ordinary disconnect
            else
            {
                success = this->getCoreSystemAccessPtr()->disconnect(pStartP->getParentModelObjectName(),
                                                                     pStartP->getName(),
                                                                     pEndP->getParentModelObjectName(),
                                                                     pEndP->getName());
            }
            emit checkMessages();
        }
        else if (pConnector->isBroken())
        {
            success = true;
        }
    }

    // Delete the connector and remove it from scene and lists
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

    // Refresh the graphics view
    mpModelWidget->getGraphicsView()->updateViewPort();
}



//! @brief Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
//! @return A pointer to the created connector, 0 if failed, or connector unfinished
Connector* SystemObject::createConnector(Port *pPort, UndoStatusEnumT undoSettings)
{
    // When clicking end port (finish creation of connector)
    if (mIsCreatingConnector)
    {
        bool success = false;
        if (mpTempConnector->isDangling() && pPort)
        {
            Port *pStartPort = mpTempConnector->getStartPort();
            Port *pEndPort = pPort;

#ifdef DEVELOPMENT
            if(pStartPort->getNodeType() == "NodeHydraulic" &&                 //Connecting two Q-type hydraulic ports, add a "volunector"
                    pEndPort->getNodeType() == "NodeHydraulic" &&
                    pStartPort->getParentModelObject()->getTypeCQS() == "Q" &&
                    pEndPort->getParentModelObject()->getTypeCQS() == "Q")
            {
                mpTempConnector->makeVolunector();
                bool ok1 = getCoreSystemAccessPtr()->connect(pStartPort->getParentModelObjectName(),
                                                             pStartPort->getName(),
                                                             mpTempConnector->getVolunectorComponent()->getName(),
                                                             "P1");
                bool ok2 = getCoreSystemAccessPtr()->connect(mpTempConnector->getVolunectorComponent()->getName(),
                                                             "P2",
                                                             pEndPort->getParentModelObjectName(),
                                                             pEndPort->getName());
                success = ok1 && ok2;
            }
            // Else treat as normal ports
            else
#endif
            {
                success = this->getCoreSystemAccessPtr()->connect(pStartPort->getParentModelObjectName(),
                                                                  pStartPort->getName(),
                                                                  pEndPort->getParentModelObjectName(),
                                                                  pEndPort->getName());
            }
        }

        if (success)
        {
            gpHelpPopupWidget->hide();
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

        gpHelpPopupWidget->showHelpPopupMessage("Create the connector by clicking in the workspace. Finish connector by clicking on another component port.");
        mIsCreatingConnector = true;

        // Return ptr to the created connector
        return mpTempConnector;
    }
}

//! @brief Create a connector when both ports are known (when loading primarily)
Connector* SystemObject::createConnector(Port *pPort1, Port *pPort2, UndoStatusEnumT undoSettings)
{
    if (!mIsCreatingConnector)
    {
        createConnector(pPort1, undoSettings);
        Connector* pConn = createConnector(pPort2, undoSettings);

        // If we failed we still want to finish and add this connector (if success it was already added)
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
        gpMessageHandler->addErrorMessage("Could not create connector, connector creation already in progress");
        return 0;
    }
}


//! @brief Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void SystemObject::cutSelected(CopyStack *xmlStack)
{
    // Don't copy if message widget as focus (they also use ctrl-c key sequence)
    // Also Check if we have any selected object, prevent clearing copy stack if nothing selected
    // Also Check if this system is locked then copy should not be allowed
    bool haveSelected = !mSelectedModelObjectsList.empty() || !mSelectedSubConnectorsList.empty() || !mSelectedWidgetsList.empty();
    if (!getContainedScenePtr()->hasFocus() || !haveSelected || isLocallyLocked() || getModelLockLevel()>NotLocked)
    {
        return;
    }

    this->copySelected(xmlStack);
    this->mpUndoStack->newPost(undo::cut);
    emit deleteSelected();
    mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void SystemObject::copySelected(CopyStack *xmlStack)
{
    // Don't copy if message widget as focus (they also use ctrl-c key sequence)
    // Also Check if we have any selected object, prevent clearing copy stack if nothing selected
    // Also Check if this system is locked then copy should not be allowed
    bool haveSelected = !mSelectedModelObjectsList.empty() || !mSelectedSubConnectorsList.empty() || !mSelectedWidgetsList.empty();
    if (!getContainedScenePtr()->hasFocus() || !haveSelected || isLocallyLocked() || getModelLockLevel()>NotLocked)
    {
        return;
    }

    QDomElement *copyRoot;
    if(xmlStack == nullptr) {
        gpCopyStack->clear();
        copyRoot = gpCopyStack->getCopyRoot();
    }
    else {
        xmlStack->clear();
        copyRoot = xmlStack->getCopyRoot();
    }

    // Store center point
    QPointF center = getCenterPointFromSelection();
    appendCoordinateTag(*copyRoot, center.x(), center.y());

    // Copy components
    const auto thisSystemsParameterNames = getParameterNames();
    for(auto& selectedMO : mSelectedModelObjectsList)
    {
        selectedMO->saveToDomElement(*copyRoot, FullModel);

        QStringList componentParNames = selectedMO->getParameterNames();
        for(const QString& componentParName : componentParNames)
        {
            QStringList exprVariables;
            CoreParameterData componentParameterData;
            selectedMO->getParameter(componentParName, componentParameterData);
            if (componentParameterData.mType == "double") {
                if (!componentParameterData.hasDoubleValue()) {
                    exprVariables = getEmbeddedSriptVariableNames(componentParameterData.mValue, selectedMO->getName(), getCoreSystemAccessPtr());
                }
            }
            else if (componentParameterData.mType == "integer") {
                if (!componentParameterData.hasIntegerValue()) {
                    exprVariables.append(componentParameterData.mValue);
                }
            }
            else if (componentParameterData.mType == "bool") {
                if (!componentParameterData.hasBooleanValue()) {
                    exprVariables.append(componentParameterData.mValue);
                }
            }
            else {
                exprVariables.append(componentParameterData.mValue);
            }

            // Erase any begining with .self as those can not be system parameters
            auto new_end = std::remove_if(exprVariables.begin(), exprVariables.end(), [](QString& v){ return v.startsWith("self."); });
            exprVariables.erase(new_end, exprVariables.end());

            for (const auto& var : exprVariables) {
                if(thisSystemsParameterNames.contains(var)) {
                    CoreParameterData systemParameterData;
                    getParameter(var, systemParameterData);
                    QDomElement xmlParameter = appendDomElement(*copyRoot, hmf::parameter::root);
                    xmlParameter.setAttribute(hmf::name, systemParameterData.mName);
                    xmlParameter.setAttribute(hmf::value, systemParameterData.mValue);
                    xmlParameter.setAttribute(hmf::type, systemParameterData.mType);
                    xmlParameter.setAttribute(hmf::internal, systemParameterData.mInternal);
                    if (!systemParameterData.mQuantity.isEmpty())
                    {
                        xmlParameter.setAttribute(hmf::quantity, systemParameterData.mQuantity);
                    }
                    if (!systemParameterData.mUnit.isEmpty())
                    {
                        xmlParameter.setAttribute(hmf::unit, systemParameterData.mUnit);
                    }
                    if (!systemParameterData.mDescription.isEmpty())
                    {
                        xmlParameter.setAttribute(hmf::modelinfo::description, systemParameterData.mDescription);
                    }
                }
            }
        }
    }

    // Copy connectors
    for(const auto pConnector : mSubConnectorList) {
        if (pConnector->isActive() && !pConnector->isBroken()) {
            Port *pStartPort = pConnector->getStartPort();
            Port *pEndPort = pConnector->getEndPort();
            if (pStartPort && pEndPort && pStartPort->getParentModelObject()->isSelected() && pEndPort->getParentModelObject()->isSelected()) {
                pConnector->saveToDomElement(*copyRoot);
            }
        }
    }

    // Copy widgets
    for (const auto pSelectedWidget : mSelectedWidgetsList) {
        if(pSelectedWidget->isSelected()) {
            pSelectedWidget->saveToDomElement(*copyRoot);
        }
    }
}


//! @brief Pastes the contents in the copy stack at the mouse position
//! @see cutSelected()
//! @see copySelected()
void SystemObject::paste(CopyStack *xmlStack)
{
    // Do not allow past if model or container is locked
    // Do not paste if some other widget has focus (eg. terminal)
    // Do not paste if left mouse button is pressed, it will mess with the positions of the components
    if (!getContainedScenePtr()->hasFocus() || isLocallyLocked() || (getModelLockLevel() > NotLocked) ||
         mpModelWidget->getGraphicsView()->isLeftMouseButtonPressed()) {
        return;
    }

    bool didPaste = false;
    mpUndoStack->newPost(undo::paste);

    QDomElement *copyRoot;
    if(xmlStack == nullptr) {
        copyRoot = gpCopyStack->getCopyRoot();
    }
    else {
        copyRoot = xmlStack->getCopyRoot();
    }

    // Deselect all components & connectors
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();

    // Used a map to track name changes, so that connectors will know what components and ports are actually named
    struct RenamedCompOrSysport {
        QString actualName;
        bool isSystemPort = false;
    };

    QHash<QString, RenamedCompOrSysport> renamedMap;

    // Determine paste offset (will paste components at mouse position
    QDomElement coordTag = copyRoot->firstChildElement(hmf::connector::coordinate);
    double x, y;
    parseCoordinateTag(coordTag, x, y);
    QPointF oldCenter(x, y);

    QCursor cursor;
    QPointF newCenter = mpModelWidget->getGraphicsView()->mapToScene(mpModelWidget->getGraphicsView()->mapFromGlobal(cursor.pos()));

    const QPointF offset = newCenter - oldCenter;

    qDebug() << "Pasting at: " << newCenter;
    qDebug() << "Paste with offset: " << offset;
    QString str;
    QTextStream out(&str);
    copyRoot->ownerDocument().save(out, XMLINDENTATION);
    qDebug() << str;

    // Help function to load and paste Component or System type model objects
    auto pasteComponentOrSystem = [this, copyRoot, offset, &renamedMap, &didPaste](const QString& tagName) {
        // Paste components
        QDomElement objectElement = copyRoot->firstChildElement(tagName);
        while(!objectElement.isNull()) {
            ModelObject *pObj = loadModelObject(objectElement, this, Undo);
            if (pObj) {
                // Apply offset to pasted object
                const auto prevPos = pObj->pos();
                pObj->moveBy(offset.x(), offset.y());
                // Map renamed components
                const QString desiredName = objectElement.attribute(hmf::name);
                const QString actualNameAfterLoad = pObj->getName();
                renamedMap.insert(desiredName, {actualNameAfterLoad, false});
                didPaste = true;
                mpUndoStack->registerMovedObject(prevPos, pObj->pos(), actualNameAfterLoad);
            }
            objectElement = objectElement.nextSiblingElement(tagName);
        }
    };

    // Paste system parameters
    QDomElement parElement = copyRoot->firstChildElement(hmf::parameter::root);
    while(!parElement.isNull())
    {
        QString name = parElement.attribute(hmf::name);
        if(!getParameterNames().contains(name))
        {
            QString value = parElement.attribute(hmf::value);
            QString type = parElement.attribute(hmf::type);
            QString quantityORunit = parElement.attribute(hmf::quantity, parElement.attribute(hmf::unit));
            QString description = parElement.attribute(hmf::modelinfo::description);
            bool internal = parseAttributeBool(parElement, hmf::internal, false);

            CoreParameterData parData = CoreParameterData(name, value, type, quantityORunit, "", description, internal);
            setOrAddParameter(parData);
            didPaste = true;
        }
        parElement = parElement.nextSiblingElement(hmf::parameter::root);
    }

    // Paste Components and Systems
    pasteComponentOrSystem(hmf::component);
    pasteComponentOrSystem(hmf::system);

    // Paste system ports
    QDomElement systemPortElement = copyRoot->firstChildElement(hmf::systemport);
    while (!systemPortElement.isNull()) {
        ModelObject* pObj = loadSystemPortObject(systemPortElement, this, Undo);
        if (pObj) {
            // Apply offset to pasted object
            const auto prevPos = pObj->pos();
            pObj->moveBy(offset.x(), offset.y());
            // Map renamed components
            const QString desiredName = systemPortElement.attribute(hmf::name);
            const QString actualNameAfterLoad = pObj->getName();
            renamedMap.insert(desiredName, {actualNameAfterLoad, true});
            didPaste = true;
            mpUndoStack->registerMovedObject(prevPos, pObj->pos(), actualNameAfterLoad);
        }
        systemPortElement = systemPortElement.nextSiblingElement(hmf::systemport);
    }

    // Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement(hmf::connector::root);
    while(!connectorElement.isNull()) {
        QDomElement tempConnectorElement = connectorElement.cloneNode(true).toElement();
        const RenamedCompOrSysport actualStartComp = renamedMap.value(connectorElement.attribute(hmf::connector::startcomponent));
        const RenamedCompOrSysport actualEndComp = renamedMap.value(connectorElement.attribute(hmf::connector::endcomponent));

        // Replace component names with actual names
        tempConnectorElement.setAttribute(hmf::connector::startcomponent, actualStartComp.actualName);
        tempConnectorElement.setAttribute(hmf::connector::endcomponent, actualEndComp.actualName);
        // Replace system port port names with actual names
        if (actualStartComp.isSystemPort) {
            tempConnectorElement.setAttribute(hmf::connector::startport, actualStartComp.actualName);
        }
        if (actualEndComp.isSystemPort) {
            tempConnectorElement.setAttribute(hmf::connector::endport, actualStartComp.actualName);
        }

        bool sucess = loadConnector(tempConnectorElement, this, Undo);
        if (sucess) {
            Connector *tempConnector = this->findConnector(tempConnectorElement.attribute(hmf::connector::startcomponent), tempConnectorElement.attribute(hmf::connector::startport),
                                                           tempConnectorElement.attribute(hmf::connector::endcomponent), tempConnectorElement.attribute(hmf::connector::endport));
            // Apply offset to connector
            tempConnector->moveAllPoints(offset.x(), offset.y());
            tempConnector->drawConnector(true);
            didPaste = true;
        }

        connectorElement = connectorElement.nextSiblingElement(hmf::connector::root);
    }

    // Paste widgets
    QDomElement textBoxElement = copyRoot->firstChildElement(hmf::widget::textboxwidget);
    while(!textBoxElement.isNull())
    {
        TextBoxWidget *pWidget = loadTextBoxWidget(textBoxElement, this, Undo);
        if (pWidget) {
            pWidget->setSelected(true);
            const auto prevPos = pWidget->pos();
            pWidget->moveBy(offset.x(), offset.y());
            didPaste = true;
            mpUndoStack->registerMovedWidget(pWidget, prevPos, pWidget->pos());
        }
        textBoxElement = textBoxElement.nextSiblingElement(hmf::widget::textboxwidget);
    }

    // Paste image widgets
    QDomElement imageElement = copyRoot->firstChildElement(hmf::widget::imagewidget);
    while(!imageElement.isNull())
    {
        ImageWidget *pWidget = loadImageWidget(imageElement, this, Undo);
        if (pWidget) {
            pWidget->setSelected(true);
            const auto prevPos = pWidget->pos();
            pWidget->moveBy(offset.x(), offset.y());
            didPaste = true;
            mpUndoStack->registerMovedWidget(pWidget, prevPos, pWidget->pos());
        }
        imageElement = imageElement.nextSiblingElement(hmf::widget::imagewidget);
    }

    if (didPaste) {
        // Select all pasted components
        for(auto itn = renamedMap.begin(); itn != renamedMap.end(); ++itn) {
            mModelObjectMap.find(itn.value().actualName).value()->setSelected(true);
        }
        mpModelWidget->hasChanged();
        mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


//! @brief Aligns all selected objects vertically to the last selected object.
void SystemObject::alignX()
{
    double newX;
    if(!mSelectedModelObjectsList.isEmpty())
    {
        newX = mSelectedModelObjectsList.last()->getCenterPos().x();
    }
    else if(!mSelectedWidgetsList.isEmpty())
    {
        newX = mSelectedWidgetsList.last()->getCenterPos().x();
    }
    else
    {
        return;
    }

    if(mSelectedModelObjectsList.size()+mSelectedWidgetsList.size() > 1)
    {
        mpUndoStack->newPost(undo::alignx);
        for(int i=0; i<mSelectedModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            mSelectedModelObjectsList.at(i)->setCenterPos(QPointF(newX, mSelectedModelObjectsList.at(i)->getCenterPos().y()));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }
        }
        for(int i=0; i<mSelectedWidgetsList.size(); ++i)
        {
            QPointF oldPos = mSelectedWidgetsList.at(i)->pos();
            mSelectedWidgetsList.at(i)->setCenterPos(QPointF(newX, mSelectedWidgetsList.at(i)->getCenterPos().y()));
            QPointF newPos = mSelectedWidgetsList.at(i)->pos();
            mpUndoStack->registerMovedWidget(mSelectedWidgetsList.at(i), oldPos, newPos);
        }
        mpModelWidget->hasChanged();
    }
}


//! @brief Aligns all selected objects horizontally to the last selected object.
void SystemObject::alignY()
{
    double newY;
    if(!mSelectedModelObjectsList.isEmpty())
    {
        newY = mSelectedModelObjectsList.last()->getCenterPos().y();
    }
    else if(!mSelectedWidgetsList.isEmpty())
    {
        newY = mSelectedWidgetsList.last()->getCenterPos().y();
    }
    else
    {
        return;
    }

    if(mSelectedModelObjectsList.size()+mSelectedWidgetsList.size() > 1)
    {
        mpUndoStack->newPost(undo::aligny);
        for(int i=0; i<mSelectedModelObjectsList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            mSelectedModelObjectsList.at(i)->setCenterPos(QPointF(mSelectedModelObjectsList.at(i)->getCenterPos().x(), newY));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }
        }
        for(int i=0; i<mSelectedWidgetsList.size(); ++i)
        {
            QPointF oldPos = mSelectedWidgetsList.at(i)->pos();
            mSelectedWidgetsList.at(i)->setCenterPos(QPointF(mSelectedWidgetsList.at(i)->getCenterPos().x(), newY));
            QPointF newPos = mSelectedWidgetsList.at(i)->pos();
            mpUndoStack->registerMovedWidget(mSelectedWidgetsList.at(i), oldPos, newPos);
        }
        mpModelWidget->hasChanged();
    }
}


//Distributes selected model objects equally horizontally
void SystemObject::distributeX()
{
    if(!mSelectedModelObjectsList.isEmpty())
    {
        QList<ModelObject*> tempList;
        tempList.append(mSelectedModelObjectsList.first());
        for(int i=1; i<mSelectedModelObjectsList.size(); ++i)
        {
            int idx = 0;
            while(idx<tempList.size() && tempList[idx]->getCenterPos().x() > mSelectedModelObjectsList[i]->getCenterPos().x())
            {
                ++idx;
            }
            tempList.insert(idx,mSelectedModelObjectsList[i]);
        }

        double min = tempList.first()->getCenterPos().x();
        double max = tempList.last()->getCenterPos().x();
        double diff = (max-min)/(tempList.size()-1.0);

        mpUndoStack->newPost(undo::distributex);
        double pos = min+diff;
        for(int i=1; i<tempList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            tempList[i]->setCenterPos(QPointF(pos, tempList[i]->getCenterPos().y()));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }

            pos += diff;
        }
        mpModelWidget->hasChanged();
    }
}


//Distributes selected model objects equally vertically
void SystemObject::distributeY()
{
    if(!mSelectedModelObjectsList.isEmpty())
    {
        QList<ModelObject*> tempList;
        tempList.append(mSelectedModelObjectsList.first());
        for(int i=1; i<mSelectedModelObjectsList.size(); ++i)
        {
            int idx = 0;
            while(idx<tempList.size() && tempList[idx]->getCenterPos().y() > mSelectedModelObjectsList[i]->getCenterPos().y())
            {
                ++idx;
                qDebug() << "idx = " << idx;
            }
            tempList.insert(idx,mSelectedModelObjectsList[i]);
        }

        double min = tempList.first()->getCenterPos().y();
        double max = tempList.last()->getCenterPos().y();
        double diff = (max-min)/(tempList.size()-1.0);

        mpUndoStack->newPost(undo::distributey);
        double pos = min+diff;
        for(int i=1; i<tempList.size()-1; ++i)
        {
            QPointF oldPos = mSelectedModelObjectsList.at(i)->pos();
            tempList[i]->setCenterPos(QPointF(tempList[i]->getCenterPos().x(), pos));
            QPointF newPos = mSelectedModelObjectsList.at(i)->pos();
            mpUndoStack->registerMovedObject(oldPos, newPos, mSelectedModelObjectsList.at(i)->getName());
            for(int j=0; j<mSelectedModelObjectsList.at(i)->getConnectorPtrs().size(); ++j)
            {
                mSelectedModelObjectsList.at(i)->getConnectorPtrs().at(j)->drawConnector(true);
            }

            pos += diff;
        }
        mpModelWidget->hasChanged();
    }
}


//! @brief Calculates the geometrical center position of the selected objects.
QPointF SystemObject::getCenterPointFromSelection()
{
    QPointF sum;
    for(const auto& pSelectedMO : mSelectedModelObjectsList){
        sum += pSelectedMO->getCenterPos();
    }
    for(const auto& pSelectedWidget : mSelectedWidgetsList){
        sum += pSelectedWidget->getCenterPos();
    }

    return sum / (mSelectedModelObjectsList.size()+mSelectedWidgetsList.size());
}



void SystemObject::replaceComponent(QString name, QString newType)
{
    if(!gpLibraryHandler->getLoadedTypeNames().contains(newType))
    {
        gpMessageHandler->addErrorMessage("Replacement type name not found: \""+newType+"\". Check the XML code.");
        return;
    }

    this->deselectAll();
    mSelectedModelObjectsList.clear();

    ModelObject *obj = getModelObject(name);

    QString oldType = obj->getTypeName();

    qDebug() << "Replacing " << oldType << " with " << newType;


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
    bool flipped = obj->isFlipped();

    deleteModelObject(name);

    qDebug() << "Name = " << name;

    ModelObject *newObj = addModelObject(newType, pos, 0);
    newObj->rotate(rot);
    if(flipped) {
        newObj->flipHorizontal();
    }

    if(!newObj)
    {
        return; // Should never happen due to check above, but keep it just in case
    }

    renameModelObject(newObj->getName(), name);

        //Paste connectors
    QDomElement connectorElement = copyRoot->firstChildElement(hmf::connector::root);
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
void SystemObject::selectSection(int no, bool append)
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
void SystemObject::assignSection(int no)
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
void SystemObject::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllConnectors();
}


//! @brief Deselects all objects and connectors.
void SystemObject::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();
}


//! @brief Hides all component names.
//! @see showNames()
void SystemObject::hideNames()
{
    mpUndoStack->newPost(undo::hideallnames);
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! @brief Shows all component names.
//! @see hideNames()
void SystemObject::showNames()
{
    mpUndoStack->newPost(undo::showallnames);
    emit showAllNameText();
}


//! @brief Toggles name text on or off
//! @see showNames();
//! @see hideNames();
void SystemObject::toggleNames(bool value)
{
    if(value)
    {
        emit showAllNameText();
    }
    else
    {
        emit hideAllNameText();
    }
    mShowSubComponentNames = value;
}


void SystemObject::toggleSignals(bool value)
{
    mSignalsVisible = value;
    emit showOrHideSignals(value);
}

//! @brief Slot that sets hide ports flag to true or false
void SystemObject::showSubcomponentPorts(bool doShowThem)
{
    mShowSubComponentPorts = doShowThem;
    emit showOrHideAllSubComponentPorts(doShowThem);
}


//! @brief Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void SystemObject::undo()
{
    mpUndoStack->undoOneStep();
}


//! @brief Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void SystemObject::redo()
{
    mpUndoStack->redoOneStep();
}

//! @brief Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void SystemObject::clearUndo()
{
    mpUndoStack->clear();
}


//! @brief Returns true if at least one GUIObject is selected
bool SystemObject::isSubObjectSelected()
{
    return (mSelectedModelObjectsList.size() > 0);
}

bool SystemObject::setVariableAlias(const QString &rFullName, const QString &rAlias)
{
    QString compName, portName, varName;
    QStringList dummy;
    splitFullVariableName(rFullName, dummy, compName, portName, varName);
    bool isOk = getCoreSystemAccessPtr()->setVariableAlias(compName, portName, varName, rAlias);
    if (isOk)
    {
        emit aliasChanged(rFullName, rAlias);
        mpModelWidget->hasChanged();
    }
    else
    {
        emit checkMessages();
    }

    return isOk;
}

QString SystemObject::getVariableAlias(const QString &rFullName)
{
    QString compName, portName, varName;
    QStringList dummy;
    splitFullVariableName(rFullName, dummy, compName, portName, varName);
    return getCoreSystemAccessPtr()->getVariableAlias(compName, portName, varName);
}

QString SystemObject::getFullNameFromAlias(const QString alias)
{
    QString comp, port, var;
    getCoreSystemAccessPtr()->getFullVariableNameByAlias(alias, comp, port, var);
    return makeFullVariableName(getParentSystemNameHieararchy(), comp,port,var);
}

QStringList SystemObject::getAliasNames()
{
    return getCoreSystemAccessPtr()->getAliasNames();
}


//! @brief Returns true if at least one GUIConnector is selected
bool SystemObject::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}


//! @brief Returns a pointer to the undo stack
UndoStack *SystemObject::getUndoStackPtr()
{
    return mpUndoStack;
}


//! @brief Returns a pointer to the drag-copy copy stack
CopyStack *SystemObject::getDragCopyStackPtr()
{
    return mpDragCopyStack;
}

void SystemObject::setModelInfo(const QString &author, const QString &email, const QString &affiliation, const QString &description)
{
    mAuthor = author;
    mEmail = email;
    mAffiliation = affiliation;
    mDescription = description;
}

void SystemObject::getModelInfo(QString &author, QString &email, QString &affiliation, QString &description) const
{
    author = mAuthor;
    email = mEmail;
    affiliation = mAffiliation;
    description = mDescription;
}


//! @brief Specifies model file for the container object
void SystemObject::setModelFile(QString path)
{
    mModelFileInfo.setFile(path);
}


//! @brief Returns a copy of the model file info of the container object
const QFileInfo &SystemObject::getModelFileInfo() const
{
    return mModelFileInfo;
}

//! @brief Returns the file path to the model that this container belongs to
//! @details Will ask the parent if the container is an embedded container else returns the path to the external system model
QString SystemObject::getModelFilePath() const
{
    if (mModelFileInfo.isFile())
    {
        return mModelFileInfo.canonicalFilePath();
    }
    else if (mpParentSystemObject)
    {
        return mpParentSystemObject->getModelFilePath();
    }
    else
    {
        return "";
    }
}

//! @brief Returns the path to the directory where the model that this container belongs to resides
//! @details Will ask the parent if the container is an embedded container else returns the path to the external system model
QString SystemObject::getModelPath() const
{
    QFileInfo fi(getModelFilePath());
    return fi.absolutePath();
}


//! @brief Returns a list with the names of the model objects in the container
QStringList SystemObject::getModelObjectNames() const
{
    QStringList names;
    for(const auto& mo : mModelObjectMap)
    {
        names.append(mo->getName());
    }
    return names;
}


//! @brief Returns a list with pointers to GUI widgets
QList<Widget *> SystemObject::getWidgets() const
{
    return mWidgetMap.values();
}

Widget *SystemObject::getWidget(const int id) const
{
    return mWidgetMap.value(id, nullptr);
}

//! @brief Returns the path to the icon with iso graphics.
//! @todo should we return full path or relative
QString SystemObject::getIconPath(const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType)
{
    return mModelObjectAppearance.getIconPath(gfxType, absrelType);
}


//! @brief Sets the path to the icon of the specified type
void SystemObject::setIconPath(const QString path, const GraphicsTypeEnumT gfxType, const AbsoluteRelativeEnumT absrelType)
{
    mModelObjectAppearance.setIconPath(path, gfxType, absrelType);
}


//! @brief Access function for mIsCreatingConnector
bool SystemObject::isCreatingConnector()
{
    return mIsCreatingConnector;
}


//! @brief Tells container object to remember a new sub connector
void SystemObject::rememberSubConnector(Connector *pConnector)
{
    mSubConnectorList.append(pConnector);
}


//! @brief This is a helpfunction that can be used to make a container "forget" about a certain connector
//!
//! It does not delete the connector and connected components dos not forget about it
//! use only when transferring ownership of objects to an other container
void SystemObject::forgetSubConnector(Connector *pConnector)
{
    mSubConnectorList.removeAll(pConnector);
}

//! @brief Refresh the graphics of all internal container ports
void SystemObject::refreshInternalSystemPortGraphics()
{
    ModelObjectMapT::iterator moit;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == SystemPortObjectType)
        {
            //We assume that a container port only have ONE gui port
            moit.value()->getPortListPtrs().first()->refreshPortGraphics();
        }
    }
}


void SystemObject::addExternalSystemPortObject(ModelObject* pModelObject)
{
    this->createRefreshExternalPort(pModelObject->getName());
}

//! @brief Aborts creation of new connector.
void SystemObject::cancelCreatingConnector()
{
    if(mIsCreatingConnector)
    {
        mpTempConnector->getStartPort()->forgetConnection(mpTempConnector);
        if(!mpTempConnector->getStartPort()->isConnected() && mShowSubComponentPorts)
        {
            mpTempConnector->getStartPort()->show();
        }
        mIsCreatingConnector = false;
        mpTempConnector->deleteLater();
        mpTempConnector=0;
        gpHelpPopupWidget->hide();
    }
}


//! @brief Switches mode of connector being created to or from diagonal mode.
//! @param diagonal Tells whether or not connector shall be diagonal or not
void SystemObject::makeConnectorDiagonal(bool diagonal)
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
void SystemObject::updateTempConnector(QPointF pos)
{
    mpTempConnector->updateEndPoint(pos);
    mpTempConnector->drawConnector();
}


//! @brief Adds one new line to the connector being created.
//! @param pos Position to add new line at
void SystemObject::addOneConnectorLine(QPointF pos)
{
    mpTempConnector->addPoint(pos);
}


//! @brief Removes one line from connector being created.
//! @param pos Position to redraw connector to after removing the line
void SystemObject::removeOneConnectorLine(QPointF pos)
{
    if((mpTempConnector->getNumberOfLines() == 1 && mpTempConnector->isMakingDiagonal()) ||  (mpTempConnector->getNumberOfLines() == 2 && !mpTempConnector->isMakingDiagonal()))
    {
        mpTempConnector->getStartPort()->forgetConnection(mpTempConnector);
        if(!mpTempConnector->getStartPort()->isConnected() && mShowSubComponentPorts)
        {
            mpTempConnector->getStartPort()->show();
        }
        mpTempConnector->getStartPort()->getParentModelObject()->forgetConnector(mpTempConnector);
        mIsCreatingConnector = false;
        mpModelWidget->getGraphicsView()->setIgnoreNextContextMenuEvent();
        mpModelWidget->getGraphicsView()->setIgnoreNextMouseReleaseEvent();
        delete(mpTempConnector);
        gpHelpPopupWidget->hide();
    }

    if(mIsCreatingConnector)
    {
        mpTempConnector->removePoint(true);
        mpTempConnector->updateEndPoint(pos);
        mpTempConnector->drawConnector();
        mpModelWidget->getGraphicsView()->updateViewPort();
    }
}


//! @brief Disables the undo function for the current model.
//! @param enabled Tells whether or not to enable the undo stack
//! @param dontAskJustDoIt If true, the warning box will not appear
void SystemObject::setUndoEnabled(bool enabled, bool dontAskJustDoIt)
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
            mpUndoStack->setEnabled(false);
            mpUndoStack->clear();
            mUndoEnabled = false;
        }
    }
    else
    {
        mpUndoStack->setEnabled(true);
        mUndoEnabled = true;
    }

    // Only modify main window actions if this is current container
    if(gpModelHandler->getCurrentViewContainerObject() == this)
    {
        gpMainWindow->mpUndoAction->setEnabled(mUndoEnabled);
        gpMainWindow->mpRedoAction->setEnabled(mUndoEnabled);
        if(gpMainWindow->mpEnableUndoAction->isChecked() != mUndoEnabled) {
            gpMainWindow->mpEnableUndoAction->setChecked(mUndoEnabled);
        }
    }

}


void SystemObject::setSaveUndo(bool save)
{
    mSaveUndoStack = save;
}


//! @brief Tells whether or not unconnected ports in container are hidden
bool SystemObject::areSubComponentPortsShown()
{
    return mShowSubComponentPorts;
}


//! @brief Tells whether or not object names in container are hidden
bool SystemObject::areSubComponentNamesShown()
{
    return mShowSubComponentNames;
}


//! @brief Tells whether or not signal components are hidden
bool SystemObject::areSignalsVisible()
{
    return mSignalsVisible;
}


//! @brief Tells whether or not undo/redo is enabled
bool SystemObject::isUndoEnabled()
{
    return mUndoEnabled;
}


//! @brief Tells whether or not the save undo option is active
bool SystemObject::getSaveUndo()
{
    return mSaveUndoStack;
}


//! @brief Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void SystemObject::updateMainWindowButtons()
{
    gpMainWindow->mpUndoAction->setEnabled(mUndoEnabled);
    gpMainWindow->mpRedoAction->setEnabled(mUndoEnabled);
    gpMainWindow->mpAnimateAction->setDisabled(mAnimationDisabled);

    //gpMainWindow->mpPlotAction->setDisabled(mpLogDataHandler->isEmpty());
    //gpMainWindow->mpShowLossesAction->setDisabled(mpLogDataHandler->isEmpty());
    //gpMainWindow->mpAnimateAction->setDisabled(mpNewPlotData->isEmpty());

    gpMainWindow->mpToggleNamesAction->setChecked(mShowSubComponentNames);
    gpMainWindow->mpTogglePortsAction->setChecked(mShowSubComponentPorts);
    gpMainWindow->mpShowLossesAction->setChecked(mLossesVisible);
}


//! @brief Sets the iso graphics option for the model
void SystemObject::setGfxType(GraphicsTypeEnumT gfxType)
{
    this->mGfxType = gfxType;
    this->mpModelWidget->getGraphicsView()->updateViewPort();
    emit setAllGfxType(mGfxType);
}


//! @brief Returns current graphics type used by container object
GraphicsTypeEnumT SystemObject::getGfxType()
{
    return mGfxType;
}


//! @brief A slot that opens the properties dialog
void SystemObject::openPropertiesDialogSlot()
{
    this->openPropertiesDialog();
}


//! @brief Slot that tells all selected name texts to deselect themselves
void SystemObject::deselectSelectedNameText()
{
    emit deselectAllNameText();
}


//! @brief Clears all of the contained objects (and deletes them).
//! This code cant be run in the destructor as this wold cause wired behaviour in the derived system class.
//! The core system would be deleted before container clear code is run, that is why we have it as a convenient protected function
void SystemObject::clearContents()
{
    ModelObjectMapT::iterator mit;
    QMap<size_t, Widget *>::iterator wit;

    qDebug() << "Clearing model objects in " << getName();
    //We cant use for loop over iterators as the maps are modified on each delete (and iterators invalidated)
    mit=mModelObjectMap.begin();
    while (mit!=mModelObjectMap.end())
    {
        //This may lead to a crash if undo stack is not disabled before calling this
        (*mit)->setIsLocked(false);     //Must unlock object in order to remove it
        deleteModelObject((*mit)->getName(), NoUndo);
        mit=mModelObjectMap.begin();
    }

    qDebug() << "Clearing widget objects in " << getName();
    wit=mWidgetMap.begin();
    while (wit!=mWidgetMap.end())
    {
        deleteWidget(*wit, NoUndo);
        wit=mWidgetMap.begin();
    }
}


//! @brief Enters a container object and makes the view represent it contents.
void SystemObject::enterContainer()
{
    // First deselect everything so that buttons pressed in the view are not sent to objects in the previous container
    mpParentSystemObject->deselectAll(); //deselect myself and anyone else

    // Remember current viewport, before we switch to the new one
    mpParentSystemObject->setGraphicsViewport(mpModelWidget->getGraphicsView()->getViewPort());

    // Show this scene
    mpModelWidget->getGraphicsView()->setContainerPtr(this);
    mpModelWidget->getQuickNavigationWidget()->addOpenContainer(this);
    mpModelWidget->getGraphicsView()->setViewPort(getGraphicsViewport());

    // Disconnect parent system and connect new system with actions
    mpParentSystemObject->unmakeMainWindowConnectionsAndRefresh();
    this->makeMainWindowConnectionsAndRefresh();

    refreshInternalSystemPortGraphics();

    mpModelWidget->handleSystemLock((this->isExternal() && this != mpModelWidget->getTopLevelSystemContainer()) || this->isAncestorOfExternalSubsystem(),
                                    isLocallyLocked());
}

//! @brief Exit a container object and make its the view represent its parents contents.
void SystemObject::exitContainer()
{
    this->deselectAll();

    // Remember current viewport, before we set parents
    this->setGraphicsViewport(mpModelWidget->getGraphicsView()->getViewPort());

    // Go back to parent system
    mpModelWidget->getGraphicsView()->setContainerPtr(mpParentSystemObject);
    mpModelWidget->getGraphicsView()->setViewPort(mpParentSystemObject->getGraphicsViewport());

    mpModelWidget->handleSystemLock((mpParentSystemObject->isExternal() && mpParentSystemObject != mpModelWidget->getTopLevelSystemContainer()) || mpParentSystemObject->isAncestorOfExternalSubsystem(),
                                    mpParentSystemObject->isLocallyLocked());

    // Disconnect this system and connect parent system with undo and redo actions
    this->unmakeMainWindowConnectionsAndRefresh();
    mpParentSystemObject->makeMainWindowConnectionsAndRefresh();

    // Refresh external port appearance
    //! @todo We only need to do this if ports have change, right now we always refresh, don't know if this is a big deal
    this->refreshExternalPortsAppearanceAndPosition();
}


//! @brief Rotates all selected objects right (clockwise)
void SystemObject::rotateSubObjects90cw()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit rotateSelectedObjectsRight();
    }
}


//! @brief Rotates all selected objects left (counter-clockwise)
void SystemObject::rotateSubObjects90ccw()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit rotateSelectedObjectsLeft();
    }
}


//! @brief Flips selected contained objects horizontally
void SystemObject::flipSubObjectsHorizontal()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit flipSelectedObjectsHorizontal();
    }
}


//! @brief Flips selected contained objects vertically
void SystemObject::flipSubObjectsVertical()
{
    if(this->isSubObjectSelected())
    {
        mpUndoStack->newPost();
        emit flipSelectedObjectsVertical();
    }
}


void SystemObject::showLosses(bool show)
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
    pHelpAction->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Help.svg"));
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

    mpLossesDialog->setPalette(gpConfig->getPalette());

    mpLossesDialog->show();

    connect(pCancelButton, SIGNAL(clicked()), mpLossesDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), this, SLOT(showLossesFromDialog()));
    connect(pHelpAction, SIGNAL(triggered()), gpHelpPopupWidget, SLOT(openContextHelp()));
}


void SystemObject::showLossesFromDialog()
{
    mpLossesDialog->close();

    //We should not be here if there is no plot data, but let's check to be sure
    if(getLogDataHandler()->isEmpty())
    {
        gpMessageHandler->addErrorMessage("Attempted to calculate losses for a model that has not been simulated (or is empty).");
        return;
    }

    mLossesVisible=true;

    bool usePower = mpAvgPwrRadioButton->isChecked();
    double time = getLogDataHandler()->copyTimeVector(-1).last();

    double limit=0;
    if(mpMinLossesSlider->isEnabled())
    {
        limit=mpMinLossesSlider->value()/100.0;
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
        if(qAbs(componentTotal) > qAbs(limit*totalLosses))     //Condition for plotting
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
        if(qAbs(componentLosses.at(c)) > qAbs(limit*totalLosses))
        {
            if(componentLosses.at(c) > 0)
                pItemModel->setData(pItemModel->index(0,c), componentLosses.at(c));
            else
                pItemModel->setData(pItemModel->index(1,c), -componentLosses.at(c));
        }
    }

    pItemModel->setVerticalHeaderLabels(QStringList() << "Added" << "Losses");
    pItemModel->setHorizontalHeaderLabels(componentNames);

    PlotWindow *pWindow = gpPlotHandler->createNewUniquePlotWindow("Energy Losses");
    pWindow->addPlotTab("Energy Losses", BarchartPlotType)->addBarChart(pItemModel);
}


void SystemObject::hideLosses()
{
    ModelObjectMapT::iterator moit;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        moit.value()->hideLosses();
    }
}


void SystemObject::measureSimulationTime()
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
            typeTimes[i] = (typeTimes[i] + times[n]/((typeCounter[i]-1)))*(typeCounter[i]-1)/(typeCounter[i]);
        }
        else
        {
            typeNames.append(typeName);
            typeTimes.append(times[n]);
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
        QStandardItem *pTimeItem = new QStandardItem(QString::number(times.at(i), 'f')+" ms");
        nameList.append(pNameItem);
        timeList.append(pTimeItem);
    }

    QList<QStandardItem *> typeNameList;
    QList<QStandardItem *> typeTimeList;
    for(int i=0; i<typeNames.size(); ++i)
    {
        QStandardItem *pNameItem = new QStandardItem(typeNames.at(i));
        QStandardItem *pTimeItem = new QStandardItem(QString::number(typeTimes.at(i), 'f')+" ms");
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

    QDialog *pDialog = new QDialog(gpMainWindowWidget);
    pDialog->setWindowTitle("Simulation Time Measurements");
    pDialog->setWindowModality(Qt::WindowModal);
    pDialog->setWindowIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-MeasureSimulationTime.svg"));

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
    QRadioButton *pTypeRadioButton = new QRadioButton(tr("Show average execution time for each component type"));
    QRadioButton *pComponentRadioButton = new QRadioButton(tr("Show execution time for each individual component"));
    pTypeRadioButton->setChecked(true);
    QVBoxLayout *pHowToShowResultsLayout = new QVBoxLayout;
    pHowToShowResultsLayout->addWidget(pTypeRadioButton);
    pHowToShowResultsLayout->addWidget(pComponentRadioButton);
    //pHowToShowResultsLayout->addStretch(1);
    pHowToShowResultsGroupBox->setLayout(pHowToShowResultsLayout);

    QPushButton *pDoneButton = new QPushButton("Done", pDialog);
    QPushButton *pChartButton = new QPushButton("Show Bar Chart", pDialog);
    QPushButton *pExportButton = new QPushButton("Export to CSV", pDialog);
    pChartButton->setCheckable(true);
    pChartButton->setChecked(false);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDialog);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::AcceptRole);
    pButtonBox->addButton(pChartButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pExportButton, QDialogButtonBox::ActionRole);

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
    connect(pExportButton, SIGNAL(clicked()), this, SLOT(exportMesasuredSimulationTime()));

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

void SystemObject::plotMeasuredSimulationTime()
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

    // Bar chart model for typenames
    QStandardItemModel *pBarChartModel = new QStandardItemModel(1,typeNames.size(),this);
    pBarChartModel->setHeaderData(0, Qt::Vertical, QColor("crimson"), Qt::BackgroundRole);
    for(int i=0; i<typeNames.size(); ++i)
    {
        pBarChartModel->setData(pBarChartModel->index(0,i), typeTimes[i]);
    }
    pBarChartModel->setVerticalHeaderLabels(QStringList() << "Time");
    pBarChartModel->setHorizontalHeaderLabels(typeNames);

    // Plot window for typename bar charts
    PlotWindow *pWindow = gpPlotHandler->createNewUniquePlotWindow("Time measurements");
    pWindow->addPlotTab("Time measurements",BarchartPlotType)->addBarChart(pBarChartModel);
    //pPlotWindow->setAttribute(Qt::WA_DeleteOnClose, false);
}

void SystemObject::exportMesasuredSimulationTime()
{
    //! @todo Ask for filename
    QString pathStr = QFileDialog::getSaveFileName(gpMainWindowWidget, "Save measured simulation times", gpConfig->getStringSetting(cfg::dir::plotdata), "*.csv");

    if(pathStr.isEmpty())
        return; //User aborted

    gpConfig->setStringSetting(cfg::dir::plotdata, QFileInfo(pathStr).absolutePath());


    QFile csvFile(pathStr);
    if(!csvFile.open(QFile::Text | QFile::WriteOnly | QFile::Truncate))
    {
        gpMessageHandler->addErrorMessage("Unable to open file for writing: "+QFileInfo(csvFile).absoluteFilePath());
        return;
    }

    //! @todo Get data somehow...
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
            //typeNames.append(pModel->data(pSelect->selectedRows(0)[i]).toString());
            //typeTimes.append(pModel->data(pSelect->selectedRows(1)[i]).toString().remove(" ms").toDouble());
            csvFile.write(QString(pModel->data(pSelect->selectedRows(0)[i]).toString()+","+pModel->data(pSelect->selectedRows(1)[i]).toString().remove(" ms")+"\n").toUtf8());
        }
    }
    else
    {
        for(int i=0; i<pModel->rowCount(); ++i)
        {
            //typeNames.append(pModel->item(i,0)->data(Qt::DisplayRole).toString());
            //typeTimes.append(pModel->item(i,1)->data(Qt::DisplayRole).toString().remove(" ms").toDouble());
            csvFile.write(QString(pModel->item(i,0)->data(Qt::DisplayRole).toString()+","+pModel->item(i,1)->data(Qt::DisplayRole).toString().remove(" ms")+"\n").toUtf8());
        }
    }

    csvFile.close();
}


bool SystemObject::isAncestorOfExternalSubsystem()
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
        return mpParentSystemObject->isAncestorOfExternalSubsystem();
    }
}


bool SystemObject::isExternal()
{
    return !mModelFileInfo.filePath().isEmpty();
}

bool SystemObject::isAnimationDisabled()
{
    return mAnimationDisabled;
}

void SystemObject::setAnimationDisabled(bool disabled)
{
    if(disabled != mAnimationDisabled)
    {
        this->hasChanged();
    }
    mAnimationDisabled = disabled;
    gpMainWindow->mpAnimateAction->setDisabled(disabled);
}


//! @brief Returns a list with pointers to all sub-connectors in container
QList<Connector *> SystemObject::getSubConnectorPtrs()
{
    return mSubConnectorList;
}



QSharedPointer<LogDataHandler2> SystemObject::getLogDataHandler()
{
    return mpModelWidget->getLogDataHandler();
    //return mpLogDataHandler;
}

QStringList SystemObject::getRequiredComponentLibraries() const
{
    QStringList requiredLibraryIds;
    for (const auto& mo : mModelObjectMap)
    {
        const auto entry = gpLibraryHandler->getEntry(mo->getTypeName(), mo->getSubTypeName());
        const auto& entryLib = entry.pLibrary;
        if (entry.isValid()) {
            requiredLibraryIds.append(entryLib->id);
        }
        // Append subsystem requirements
        if (mo->type() == SystemObjectType) {
            requiredLibraryIds.append(qobject_cast<SystemObject*>(mo)->getRequiredComponentLibraries());
        }
    }
    requiredLibraryIds.removeDuplicates();
    return requiredLibraryIds;
}

void SystemObject::exportToLabView()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(gpMainWindowWidget, tr("Export to LabVIEW/SIT"),
                                  "This will create source code for a LabVIEW/SIT DLL-file from current model. The  HopsanCore source code is included but you will need Visual Studio 2003 to compile it.\nContinue?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    //Open file dialogue and initialize the file stream
    QString filePath;
    filePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Export Project to HopsanRT Wrapper Code"),
                                            gpConfig->getStringSetting(cfg::dir::labviewexport),
                                            tr("C++ Source File (*.cpp)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFileInfo file(filePath);
    gpConfig->setStringSetting(cfg::dir::labviewexport, file.absolutePath());

    auto spGenerator = createDefaultExportGenerator();
    if (!spGenerator->generateToLabViewSIT(filePath, mpCoreSystemAccess->getCoreSystemPtr()))
    {
        gpMessageHandler->addErrorMessage("LabView SIT export failed");
    }
}

void SystemObject::exportToFMU1_32()
{
    exportToFMU("", 1, ArchitectureEnumT::x86);
}

void SystemObject::exportToFMU1_64()
{
    exportToFMU("", 1, ArchitectureEnumT::x64);
}

void SystemObject::exportToFMU2_32()
{
    exportToFMU("", 2, ArchitectureEnumT::x86);
}

void SystemObject::exportToFMU2_64()
{
    exportToFMU("", 2, ArchitectureEnumT::x64);
}

void SystemObject::exportToFMU3_32()
{
    exportToFMU("", 3, ArchitectureEnumT::x86);
}

void SystemObject::exportToFMU3_64()
{
    exportToFMU("", 3, ArchitectureEnumT::x64);
}



void SystemObject::exportToFMU(QString savePath, int version, ArchitectureEnumT arch)
{
    if(savePath.isEmpty())
    {
        //Open file dialogue and initialize the file stream
        QDir fileDialogSaveDir;
        savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Create Functional Mockup Unit"),
                                                        gpConfig->getStringSetting(cfg::dir::fmuexport),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

        QDir saveDir;
        saveDir.setPath(savePath);
        gpConfig->setStringSetting(cfg::dir::fmuexport, saveDir.absolutePath());
        saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        if(!saveDir.entryList().isEmpty())
        {
            qDebug() << saveDir.entryList();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            msgBox.setText(QString("Folder is not empty!"));
            msgBox.setInformativeText("Are you sure you want to export files here?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            int answer = msgBox.exec();
            if(answer == QMessageBox::No)
            {
                return;
            }
        }
    }

    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    if(!mpModelWidget->isSaved())
    {
        QMessageBox::information(gpMainWindowWidget, tr("Model not saved"), tr("Please save your model before exporting an FMU"));
        return;
    }

    //Save model to hmf in export directory
    mpModelWidget->saveTo(savePath+"/"+mModelFileInfo.completeBaseName().replace(" ", "_")+".hmf");

    auto spGenerator = createDefaultExportGenerator();
    spGenerator->setCompilerPath(gpConfig->getCompilerPath(arch));

    HopsanGeneratorGUI::TargetArchitectureT garch;
    if (arch == ArchitectureEnumT::x64)
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x64;
    }
    else
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x86;
    }
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    auto fmuVersion = static_cast<HopsanGeneratorGUI::FmuVersionT>(version);
    QStringList externalLibraries;
    //! @todo an idea here is to always treat the default library as external, and export it as such (and never build it in by default), that would reduce special handling of the default library
    //! @todo This code prevents nesting an external fmu inside an export, not sure if we need to support this
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        const auto mainFile = pLib->getLibraryMainFilePath();
        spGenerator->checkComponentLibrary(mainFile);
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    spGenerator->setAutoCloseWidgetsOnSuccess(false);
    if (!spGenerator->generateToFmu(savePath, pCoreSystem, externalLibraries, fmuVersion, garch))
    {
        gpMessageHandler->addErrorMessage("Failed to export FMU");
    }
}

void SystemObject::exportToSimulink()
{
    QDialog *pExportDialog = new QDialog(gpMainWindowWidget);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

//    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
//    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
//    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
//    pMSVC2008RadioButton->setChecked(true);
//    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
//    pCompilerLayout->addWidget(pMSVC2008RadioButton);
//    pCompilerLayout->addWidget(pMSVC2010RadioButton);
//    pCompilerLayout->addStretch(1);
//    pCompilerGroupBox->setLayout(pCompilerLayout);

//    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
//    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
//    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
//    p32bitRadioButton->setChecked(true);
//    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
//    pArchitectureLayout->addWidget(p32bitRadioButton);
//    pArchitectureLayout->addWidget(p64bitRadioButton);
//    pArchitectureLayout->addStretch(1);
//    pArchitectureGroupBox->setLayout(pArchitectureLayout);

//    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
//    pExportDialogLayout->addWidget(pCompilerGroupBox);
//    pExportDialogLayout->addWidget(pArchitectureGroupBox);
//    pExportDialogLayout->addWidget(pExportDialogLabel2);
    pExportDialogLayout->addWidget(pDisablePortLabels);
    pExportDialogLayout->addWidget(pExportButtonBox);
    pExportDialog->setLayout(pExportDialogLayout);

    connect(pExportButtonBox, SIGNAL(accepted()), pExportDialog, SLOT(accept()));
    connect(pExportButtonBox, SIGNAL(rejected()), pExportDialog, SLOT(reject()));

    //connect(pExportButtonOk,        SIGNAL(clicked()), pExportDialog, SLOT(accept()));
    //connect(pExportButtonCancel,    SIGNAL(clicked()), pExportDialog, SLOT(reject()));

    if(pExportDialog->exec() == QDialog::Rejected)
    {
        return;
    }


    //QMessageBox::information(gpMainWindow, gpMainWindow->tr("Create Simulink Source Files"),
    //                         gpMainWindow->tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console.\n\nVisual Studio 2008 compiler is supported, although other versions might work as well.."));

    QString fileName;
    if(!mModelFileInfo.fileName().isEmpty())
    {
        fileName = mModelFileInfo.fileName();
    }
    else
    {
        fileName = "untitled.hmf";
    }


        //Open file dialogue and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Create Simulink Source Files"),
                                                    gpConfig->getStringSetting(cfg::dir::simulinkexport),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gpConfig->setStringSetting(cfg::dir::simulinkexport, file.absolutePath());

    // Save xml document
    mpModelWidget->saveTo(savePath+"/"+fileName);

//    int compiler;
//    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
//    {
//        compiler=0;
//    }
//    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
//    {
//        compiler=1;
//    }
//    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
//    {
//        compiler=2;
//    }
//    else/* if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())*/
//    {
//        compiler=3;
//    }

    auto spGenerator = createDefaultExportGenerator();
    QString modelPath = getModelFileInfo().fileName();
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    QStringList externalLibraries;
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    auto portLabels = pDisablePortLabels->isChecked() ? HopsanGeneratorGUI::UsePortlablesT::DisablePortLables :
                                                        HopsanGeneratorGUI::UsePortlablesT::EnablePortLabels;

    if (!spGenerator->generateToSimulink(savePath, modelPath, pCoreSystem, externalLibraries, portLabels ))
    {
        gpMessageHandler->addErrorMessage("Simulink export generator failed");
    }


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
//    delete(pMSVC2008RadioButton);
//    delete(pMSVC2010RadioButton);
//    delete(p32bitRadioButton);
    //    delete(p64bitRadioButton);
}

void SystemObject::exportToExecutableModel(QString savePath, ArchitectureEnumT arch)
{
    if(savePath.isEmpty())
    {
        //Open file dialog and initialize the file stream
        QDir fileDialogSaveDir;
        savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Compile Executable Model"),
                                                        gpConfig->getStringSetting(cfg::dir::exeexport),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

        QDir saveDir;
        saveDir.setPath(savePath);
        gpConfig->setStringSetting(cfg::dir::exeexport, saveDir.absolutePath());
        saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        if(!saveDir.entryList().isEmpty())
        {
            qDebug() << saveDir.entryList();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            msgBox.setWindowTitle("Warning");
            msgBox.setText(QString("Folder is not empty!"));
            msgBox.setInformativeText("Are you sure you want to export files here?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            int answer = msgBox.exec();
            if(answer == QMessageBox::No)
            {
                return;
            }
        }
    }

    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    if(!mpModelWidget->isSaved())
    {
        QMessageBox::information(gpMainWindowWidget, tr("Model not saved"), tr("Please save your model before compiling an executable model"));
        return;
    }

    //Save model to hmf in export directory
    mpModelWidget->saveTo(savePath+"/"+mModelFileInfo.fileName().replace(" ", "_"));

    auto spGenerator = createDefaultExportGenerator();
    spGenerator->setCompilerPath(gpConfig->getCompilerPath(arch));

    HopsanGeneratorGUI::TargetArchitectureT garch;
    if (arch == ArchitectureEnumT::x64)
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x64;
    }
    else
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x86;
    }
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    QStringList externalLibraries;
    //! @todo an idea here is to always treat the default library as external, and export it as such (and never build it in by default), that would reduce special handling of the default library
    //! @todo This code prevents nesting an external fmu inside an export, not sure if we need to support this
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        const auto mainFile = pLib->getLibraryMainFilePath();
        spGenerator->checkComponentLibrary(mainFile);
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    spGenerator->setAutoCloseWidgetsOnSuccess(false);
    if (!spGenerator->generateToExe(savePath, pCoreSystem, externalLibraries, garch))
    {
        gpMessageHandler->addErrorMessage("Failed to compile executable model");
    }
}

int SystemObject::type() const
{
    return SystemObjectType;
}

QString SystemObject::getHmfTagName() const
{
    return hmf::system;
}

void SystemObject::deleteInHopsanCore()
{
    this->setUndoEnabled(false, true); //The last true means DONT ASK
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,GUISystem destructor";
    //First remove all contents
    this->clearContents();

    if (mpParentSystemObject != 0)
    {
        mpParentSystemObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
    }
    else
    {
        mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
void SystemObject::setName(QString newName)
{
    if (mpParentSystemObject == 0)
    {
        mName = mpCoreSystemAccess->setSystemName(newName);
    }
    else
    {
        mpParentSystemObject->renameModelObject(this->getName(), newName);
    }
    refreshDisplayName();
}


//! Returns a string with the sub system type.
QString SystemObject::getTypeName() const
{
     return mModelObjectAppearance.getTypeName();
}

//! @brief Get the system cqs type
//! @returns A string containing the CQS type
QString SystemObject::getTypeCQS() const
{
    return mpCoreSystemAccess->getSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QStringList containing the parameter names
QStringList SystemObject::getParameterNames()
{
    return mpCoreSystemAccess->getSystemParameterNames();
}

//! @brief Get a vector contain data from all parameters
//! @param [out] rParameterDataVec A vector that will contain parameter data
void SystemObject::getParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    mpCoreSystemAccess->getSystemParameters(rParameterDataVec);
}

//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString SystemObject::getParameterValue(const QString paramName)
{
    return mpCoreSystemAccess->getSystemParameterValue(paramName);
}

bool SystemObject::hasParameter(const QString &rParamName)
{
    return mpCoreSystemAccess->hasSystemParameter(rParamName);
}

//! @brief Get parameter data for a specific parameter
//! @param [out] rData The parameter data
void SystemObject::getParameter(const QString paramName, CoreParameterData &rData)
{
    return mpCoreSystemAccess->getSystemParameter(paramName, rData);
}

//! @brief Returns a pointer to the CoreSystemAccess that this container represents
//! @returns Pointer the the CoreSystemAccess that this container represents
CoreSystemAccess* SystemObject::getCoreSystemAccessPtr()
{
    return mpCoreSystemAccess;
}

//! @brief Overloaded version that returns self if root system
SystemObject *SystemObject::getParentSystemObject()
{
    if (mpParentSystemObject==0)
    {
        return this;
    }
    else
    {
        return mpParentSystemObject;
    }
}





//! @brief Saves the System specific core data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
void SystemObject::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement, contents);

    if (mLoadType == "EXTERNAL" && contents == FullModel)
    {
        // Determine the relative path
        QFileInfo parentModelPath(mpParentSystemObject->getModelFilePath());
        QString relPath = relativePath(getModelFilePath(), parentModelPath.absolutePath());

        // This information should ONLY be used to indicate that a subsystem is external, it SHOULD NOT be included in the actual external system
        // If it would be, the load function will fail
        rDomElement.setAttribute( hmf::externalpath, relPath );
    }

    if (mLoadType != "EXTERNAL" && contents == FullModel)
    {
        appendSimulationTimeTag(rDomElement, mpModelWidget->getStartTime().toDouble(), this->getTimeStep(), mpModelWidget->getStopTime().toDouble(), this->doesInheritTimeStep());
        appendLogSettingsTag(rDomElement, getLogStartTime(), getNumberOfLogSamples());
    }

    // Save the NumHop script
    if (!mNumHopScript.isEmpty())
    {
        appendDomTextNode(rDomElement, hmf::numhopscript, mNumHopScript);
    }

    // Save the parameter values for the system
    QVector<CoreParameterData> paramDataVector;
    this->getParameters(paramDataVector);
    QDomElement xmlParameters = appendDomElement(rDomElement, hmf::parameters);
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        QDomElement xmlParameter = appendDomElement(xmlParameters, hmf::parameter::root);
        xmlParameter.setAttribute(hmf::name, paramDataVector[i].mName);
        xmlParameter.setAttribute(hmf::value, paramDataVector[i].mValue);
        xmlParameter.setAttribute(hmf::type, paramDataVector[i].mType);
        xmlParameter.setAttribute(hmf::internal, bool2str(paramDataVector[i].mInternal));
        if (!paramDataVector[i].mQuantity.isEmpty())
        {
            xmlParameter.setAttribute(hmf::quantity, paramDataVector[i].mQuantity);
        }
        if (!paramDataVector[i].mUnit.isEmpty())
        {
            xmlParameter.setAttribute(hmf::unit, paramDataVector[i].mUnit);
        }
        if (!paramDataVector[i].mDescription.isEmpty())
        {
            xmlParameter.setAttribute(hmf::modelinfo::description, paramDataVector[i].mDescription);
        }
    }

    // Save the alias names in this system
    QDomElement xmlAliases = appendDomElement(rDomElement, hmf::aliases);
    QStringList aliases = getAliasNames();
    //! @todo need one function that gets both alias and full maybe
    for (int i=0; i<aliases.size(); ++i)
    {
        QDomElement alias = appendDomElement(xmlAliases, hmf::alias);
        alias.setAttribute(hmf::type, "variable"); //!< @todo not manual type
        alias.setAttribute(hmf::name, aliases[i]);
        QString fullName = getFullNameFromAlias(aliases[i]);
        appendDomTextNode(alias, "fullname",fullName );
    }
}

//! @brief Defines the right click menu for container objects.
void SystemObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
        return;

    bool allowFullEditing = (!isLocallyLocked() && (getModelLockLevel() == NotLocked));
    //bool allowLimitedEditing = (!isLocallyLocked() && (getModelLockLevel() <= LimitedLock));

    QMenu menu;
    QAction *enterAction = menu.addAction(tr("Enter Subsystem"));
    menu.addSeparator();

    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    QAction *saveAction = menu.addAction(tr("Save Subsystem As"));
    QAction *saveAsComponentAction = menu.addAction(tr("Save As Component"));
    menu.addSeparator();
    QAction *saveParameterValuesAction = menu.addAction(tr("Save parameter values to file"));
    QAction *loadParameterValuesAction = menu.addAction(tr("Load parameter values from file"));
    loadAction->setEnabled(allowFullEditing);
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    if(isExternal())
    {
        saveAction->setDisabled(true);
        saveAsComponentAction->setDisabled(true);
    }

    QAction *pAction = this->buildBaseContextMenu(menu, event);
    if (pAction == loadAction)
    {
        QString modelFilePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Choose Subsystem File"),
                                                             gpConfig->getStringSetting(cfg::dir::subsystem),
                                                             tr("Hopsan Model Files (*.hmf)"));
        if (!modelFilePath.isNull())
        {
            QFile file;
            file.setFileName(modelFilePath);
            QFileInfo fileInfo(file);
            gpConfig->setStringSetting(cfg::dir::subsystem, fileInfo.absolutePath());

            bool doIt = true;
            if (mModelObjectMap.size() > 0)
            {
                QMessageBox clearAndLoadQuestionBox(QMessageBox::Warning, tr("Warning"),tr("All current contents of the system will be replaced. Do you want to continue?"), 0, 0);
                clearAndLoadQuestionBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
                clearAndLoadQuestionBox.addButton(tr("&No"), QMessageBox::RejectRole);
                clearAndLoadQuestionBox.setWindowIcon(gpMainWindowWidget->windowIcon());
                doIt = (clearAndLoadQuestionBox.exec() == QMessageBox::AcceptRole);
            }

            if (doIt)
            {
                this->clearContents();

                QDomDocument domDocument;
                QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, hmf::root);
                if (!hmfRoot.isNull())
                {
                    //! @todo Check version numbers
                    //! @todo check if we could load else give error message and don't attempt to load
                    QDomElement systemElement = hmfRoot.firstChildElement(hmf::system);
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
        modelFilePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Subsystem As"),
                                                     gpConfig->getStringSetting(cfg::dir::loadmodel),
                                                     tr("Hopsan Model Files (*.hmf)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }


        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
        QFile file(modelFilePath);   //Create a QFile object
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
            return;
        }

        //Save xml document
        QDomDocument domDocument;
        QDomElement rootElement;
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

        // Save the required external library names
        QStringList requiredLibraries = this->getRequiredComponentLibraries();
        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (const auto& libID : requiredLibraries)
        {
            auto pLibrary = gpLibraryHandler->getLibrary(libID);
            if (pLibrary) {
                auto libdom = appendDomElement(reqDom, "componentlibrary");
                appendDomTextNode(libdom, "id", libID);
                appendDomTextNode(libdom, "name", pLibrary->name);
            }
        }

        //Save the model component hierarchy
        this->saveToDomElement(rootElement, FullModel);

        //Save to file
        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        bool saveOK = saveXmlFile(modelFilePath, gpMessageHandler, [&](){return domDocument;});
        if (!saveOK) {
            return;
        }

       // mpModelWidget->saveTo(modelFilePath, FullModel);
    }
    else if(pAction == saveAsComponentAction)
    {
        //Get file name
        QString cafFilePath;
        cafFilePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Subsystem As"),
                                                   gpConfig->getStringSetting(cfg::dir::loadmodel),
                                                   tr("Hopsan Component Appearance Files (*.xml)"));

        if(cafFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }

        QString iconFileName = QFileInfo(getIconPath(UserGraphics, Absolute)).fileName();
        QString modelFileName = QFileInfo(cafFilePath).baseName()+".hmf";

        //! @todo why is graphics copied twice
        QFile::copy(getIconPath(UserGraphics, Absolute), QFileInfo(cafFilePath).path()+"/"+iconFileName);
        QFile::copy(getIconPath(UserGraphics, Absolute), getAppearanceData()->getBasePath()+"/"+iconFileName);

        bool ok;
        QString subtype = QInputDialog::getText(gpMainWindowWidget, tr("Decide a unique Subtype"),
                                                tr("Decide a unique subtype name for this component:"), QLineEdit::Normal,
                                                QString(""), &ok);
        if (!ok || subtype.isEmpty())
        {
            gpMessageHandler->addErrorMessage("You must specify a subtype name. Aborting!");
            return;
        }

        //! @todo it would be better if this xml would only include hmffile attribute and all otehr info loaded from there
        QString cafStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        cafStr.append(QString("<hopsanobjectappearance version=\"0.3\">\n"));
        cafStr.append(QString("    <modelobject hmffile=\"%1\" displayname=\"%2\" typename=\"%3\" subtypename=\"%4\">\n").arg(modelFileName).arg(getName()).arg("Subsystem").arg(subtype));
        cafStr.append(QString("        <icons>\n"));
        cafStr.append(QString("            <icon scale=\"1\" path=\"%1\" iconrotation=\"ON\" type=\"user\"/>\n").arg(iconFileName));
        cafStr.append(QString("        </icons>\n"));
        cafStr.append(QString("    </modelobject>\n"));
        cafStr.append(QString("</hopsanobjectappearance>\n"));

        QFile cafFile(cafFilePath);
        if(!cafFile.open(QFile::Text | QFile::WriteOnly))
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+cafFile.fileName()+" for writing.");
            return;
        }
        cafFile.write(cafStr.toUtf8());
        cafFile.close();

        QString modelFilePath = QFileInfo(cafFilePath).path()+"/"+QFileInfo(cafFilePath).baseName()+".hmf";

        QString orgIconPath = this->getIconPath(UserGraphics, Relative);
        this->setIconPath(iconFileName, UserGraphics, Relative);

        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
        QFile file(modelFilePath);   //Create a QFile object
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
            return;
        }

        //Save xml document
        QDomDocument domDocument;
        QDomElement rootElement;
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

        // Save the required external library names
        QStringList requiredLibraries = this->getRequiredComponentLibraries();
        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (const auto& libID : requiredLibraries)
        {
            auto pLibrary = gpLibraryHandler->getLibrary(libID);
            if (pLibrary) {
                auto libdom = appendDomElement(reqDom, "componentlibrary");
                appendDomTextNode(libdom, "id", libID);
                appendDomTextNode(libdom, "name", pLibrary->name);
            }
        }

        //Save the model component hierarchy
        QString old_subtype = this->getAppearanceData()->getSubTypeName();
        this->getAppearanceData()->setSubTypeName(subtype);
        this->saveToDomElement(rootElement, FullModel);
        this->getAppearanceData()->setSubTypeName(old_subtype);

        //Save to file
        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        bool saveOK = saveXmlFile(modelFilePath, gpMessageHandler, [&](){return domDocument;});
        if (!saveOK) {
            return;
        }

        this->setIconPath(orgIconPath, UserGraphics, Relative);

        QFile::remove(getModelFilePath()+"/"+iconFileName);
    }
    else if (pAction == saveParameterValuesAction)
    {
        this->saveParameterValuesToFile();
    }
    else if (pAction == loadParameterValuesAction)
    {
        this->loadParameterValuesFromFile();
    }
    else if (pAction == enterAction)
    {
        enterContainer();
    }

    // Don't call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
}


//! @brief Defines the double click event for container objects (used to enter containers).
void SystemObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    if (isExternal())
    {
        openPropertiesDialog();
    }
    else
    {
        enterContainer();
    }
}

void SystemObject::saveSensitivityAnalysisSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLsens = appendDomElement(rDomElement, hmf::sensitivityanalysis::root);
    QDomElement XMLsetting = appendDomElement(XMLsens, hmf::sensitivityanalysis::settings);
    appendDomIntegerNode(XMLsetting, hmf::sensitivityanalysis::iterations, mSensSettings.nIter);
    if(mSensSettings.distribution == SensitivityAnalysisSettings::UniformDistribution)
    {
        appendDomTextNode(XMLsetting, hmf::sensitivityanalysis::distribution, hmf::sensitivityanalysis::uniformdistribution);
    }
    else if(mSensSettings.distribution == SensitivityAnalysisSettings::NormalDistribution)
    {
        appendDomTextNode(XMLsetting, hmf::sensitivityanalysis::distribution, hmf::sensitivityanalysis::normaldistribution);
    }

    //Parameters
    QDomElement XMLparameters = appendDomElement(XMLsens, hmf::parameters);
    for(int i = 0; i < mSensSettings.parameters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, hmf::parameter::root);
        appendDomTextNode(XMLparameter, hmf::component, mSensSettings.parameters.at(i).compName);
        appendDomTextNode(XMLparameter, hmf::parameter::root, mSensSettings.parameters.at(i).parName);
        appendDomValueNode2(XMLparameter, hmf::sensitivityanalysis::minmax, mSensSettings.parameters.at(i).min, mSensSettings.parameters.at(i).max);
        appendDomValueNode(XMLparameter, hmf::sensitivityanalysis::average, mSensSettings.parameters.at(i).aver);
        appendDomValueNode(XMLparameter, hmf::sensitivityanalysis::sigma, mSensSettings.parameters.at(i).sigma);
    }

    //Variables
    QDomElement XMLobjectives = appendDomElement(XMLsens, hmf::sensitivityanalysis::plotvariables);
    for(int i = 0; i < mSensSettings.variables.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, hmf::sensitivityanalysis::plotvariable);
        appendDomTextNode(XMLobjective, hmf::component, mSensSettings.variables.at(i).compName);
        appendDomTextNode(XMLobjective, hmf::port, mSensSettings.variables.at(i).portName);
        appendDomTextNode(XMLobjective, hmf::sensitivityanalysis::plotvariable, mSensSettings.variables.at(i).varName);
    }
}


void SystemObject::loadSensitivityAnalysisSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(hmf::sensitivityanalysis::settings);
    if(!settingsElement.isNull())
    {
        mSensSettings.nIter = parseDomIntegerNode(settingsElement.firstChildElement(hmf::sensitivityanalysis::iterations), mSensSettings.nIter);
        QDomElement distElement = settingsElement.firstChildElement(hmf::sensitivityanalysis::distribution);
        if(!distElement.isNull() && distElement.text() == hmf::sensitivityanalysis::uniformdistribution)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::UniformDistribution;
        }
        else if(!distElement.isNull() && distElement.text() == hmf::sensitivityanalysis::normaldistribution)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::NormalDistribution;
        }
    }

    QDomElement parametersElement = rDomElement.firstChildElement(hmf::parameters);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement =parametersElement.firstChildElement(hmf::parameter::root);
        while (!parameterElement.isNull())
        {
            SensitivityAnalysisParameter par;
            par.compName = parameterElement.firstChildElement(hmf::component).text();
            par.parName = parameterElement.firstChildElement(hmf::parameter::root).text();
            parseDomValueNode2(parameterElement.firstChildElement(hmf::sensitivityanalysis::minmax), par.min, par.max);
            par.aver = parseDomValueNode(parameterElement.firstChildElement(hmf::sensitivityanalysis::average), 0);
            par.sigma = parseDomValueNode(parameterElement.firstChildElement(hmf::sensitivityanalysis::sigma), 0);
            mSensSettings.parameters.append(par);

            parameterElement = parameterElement.nextSiblingElement(hmf::parameter::root);
        }
    }

    QDomElement variablesElement = rDomElement.firstChildElement(hmf::sensitivityanalysis::plotvariables);
    if(!variablesElement.isNull())
    {
        QDomElement variableElement = variablesElement.firstChildElement(hmf::sensitivityanalysis::plotvariable);
        while (!variableElement.isNull())
        {
            SensitivityAnalysisVariable var;

            var.compName = variableElement.firstChildElement(hmf::component).text();
            var.portName = variableElement.firstChildElement(hmf::port).text();
            var.varName = variableElement.firstChildElement(hmf::sensitivityanalysis::plotvariable).text();
            mSensSettings.variables.append(var);

            variableElement = variableElement.nextSiblingElement((hmf::sensitivityanalysis::plotvariable));
        }
    }
}


void SystemObject::saveOptimizationSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLopt = appendDomElement(rDomElement, hmf::optimization::root);
    QDomElement XMLsetting = appendDomElement(XMLopt, hmf::sensitivityanalysis::settings);
    appendDomTextNode(XMLsetting, hmf::scriptfile, mOptSettings.mScriptFile);
    appendDomIntegerNode(XMLsetting, hmf::sensitivityanalysis::iterations, mOptSettings.mNiter);
    appendDomIntegerNode(XMLsetting, hmf::optimization::numberofsearchpoints, mOptSettings.mNsearchp);
    appendDomValueNode(XMLsetting, hmf::optimization::reflectioncoefficient, mOptSettings.mRefcoeff);
    appendDomValueNode(XMLsetting, hmf::optimization::randomfactor, mOptSettings.mRandfac);
    appendDomValueNode(XMLsetting, hmf::optimization::forgettingfactor, mOptSettings.mForgfac);
    appendDomValueNode(XMLsetting, hmf::optimization::partol, mOptSettings.mPartol);
    appendDomBooleanNode(XMLsetting, hmf::optimization::plot, mOptSettings.mPlot);
    appendDomBooleanNode(XMLsetting, hmf::optimization::savecsv, mOptSettings.mSavecsv);
    appendDomBooleanNode(XMLsetting, hmf::optimization::finaleval, mOptSettings.mFinalEval);

    //Parameters
    appendDomBooleanNode(XMLsetting, hmf::optimization::logpar, mOptSettings.mlogPar);
    QDomElement XMLparameters = appendDomElement(XMLopt, hmf::parameters);
    for(int i = 0; i < mOptSettings.mParamters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, hmf::parameter::root);
        appendDomTextNode(XMLparameter, hmf::component, mOptSettings.mParamters.at(i).mComponentName);
        appendDomTextNode(XMLparameter, hmf::parameter::root, mOptSettings.mParamters.at(i).mParameterName);
        appendDomValueNode2(XMLparameter, hmf::sensitivityanalysis::minmax, mOptSettings.mParamters.at(i).mMin, mOptSettings.mParamters.at(i).mMax);
    }

    //Objective Functions
    QDomElement XMLobjectives = appendDomElement(XMLopt, hmf::optimization::objectives);
    for(int i = 0; i < mOptSettings.mObjectives.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, hmf::optimization::objective);
        appendDomTextNode(XMLobjective, hmf::optimization::functionname, mOptSettings.mObjectives.at(i).mFunctionName);
        appendDomValueNode(XMLobjective, hmf::optimization::weight, mOptSettings.mObjectives.at(i).mWeight);
        appendDomValueNode(XMLobjective, hmf::optimization::norm, mOptSettings.mObjectives.at(i).mNorm);
        appendDomValueNode(XMLobjective, hmf::optimization::exp, mOptSettings.mObjectives.at(i).mExp);

        QDomElement XMLObjectiveVariables = appendDomElement(XMLobjective, hmf::sensitivityanalysis::plotvariables);
        if(!(mOptSettings.mObjectives.at(i).mVariableInfo.isEmpty()))
        {
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mVariableInfo.size(); ++j)
            {
                QDomElement XMLObjectiveVariable = appendDomElement(XMLObjectiveVariables, hmf::sensitivityanalysis::plotvariable);
                appendDomTextNode(XMLObjectiveVariable, hmf::component, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(0));
                appendDomTextNode(XMLObjectiveVariable, hmf::port, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(1));
                appendDomTextNode(XMLObjectiveVariable, hmf::sensitivityanalysis::plotvariable, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(2));
            }
        }


        if(!(mOptSettings.mObjectives.at(i).mData.isEmpty()))
        {
            QDomElement XMLdata = appendDomElement(XMLobjective, hmf::optimization::data);
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mData.size(); ++j)
            {
                appendDomTextNode(XMLdata, hmf::parameter::root, mOptSettings.mObjectives.at(i).mData.at(j));
            }
        }
    }
}


void SystemObject::loadOptimizationSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(hmf::sensitivityanalysis::settings);
    if(!settingsElement.isNull())
    {
        mOptSettings.mScriptFile = parseDomStringNode(settingsElement.firstChildElement(hmf::scriptfile), mOptSettings.mScriptFile);
        mOptSettings.mNiter = parseDomIntegerNode(settingsElement.firstChildElement(hmf::sensitivityanalysis::iterations), mOptSettings.mNiter);
        mOptSettings.mNsearchp = parseDomIntegerNode(settingsElement.firstChildElement(hmf::optimization::numberofsearchpoints), mOptSettings.mNsearchp);
        mOptSettings.mRefcoeff = parseDomValueNode(settingsElement.firstChildElement(hmf::optimization::reflectioncoefficient), mOptSettings.mRefcoeff);
        mOptSettings.mRandfac = parseDomValueNode(settingsElement.firstChildElement(hmf::optimization::randomfactor), mOptSettings.mRandfac);
        mOptSettings.mForgfac = parseDomValueNode(settingsElement.firstChildElement(hmf::optimization::forgettingfactor), mOptSettings.mForgfac);
        mOptSettings.mPartol = parseDomValueNode(settingsElement.firstChildElement(hmf::optimization::partol), mOptSettings.mPartol);
        mOptSettings.mPlot = parseDomBooleanNode(settingsElement.firstChildElement(hmf::optimization::plot), mOptSettings.mPlot);
        mOptSettings.mSavecsv = parseDomBooleanNode(settingsElement.firstChildElement(hmf::optimization::savecsv), mOptSettings.mSavecsv);
        mOptSettings.mFinalEval = parseDomBooleanNode(settingsElement.firstChildElement(hmf::optimization::finaleval), mOptSettings.mFinalEval);
        mOptSettings.mlogPar = parseDomBooleanNode(settingsElement.firstChildElement(hmf::optimization::logpar), mOptSettings.mlogPar);
    }

    QDomElement parametersElement = rDomElement.firstChildElement(hmf::parameters);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement = parametersElement.firstChildElement(hmf::parameter::root);
        while (!parameterElement.isNull())
        {
            OptParameter parameter;
            parameter.mComponentName = parameterElement.firstChildElement(hmf::component).text();
            parameter.mParameterName = parameterElement.firstChildElement(hmf::parameter::root).text();
            parseDomValueNode2(parameterElement.firstChildElement(hmf::sensitivityanalysis::minmax), parameter.mMin, parameter.mMax);
            mOptSettings.mParamters.append(parameter);

            parameterElement = parameterElement.nextSiblingElement(hmf::parameter::root);
        }
    }

    QDomElement objectivesElement = rDomElement.firstChildElement(hmf::optimization::objectives);
    if(!objectivesElement.isNull())
    {
        QDomElement objElement = objectivesElement.firstChildElement(hmf::optimization::objective);
        while (!objElement.isNull())
        {
            Objectives objectives;

            objectives.mFunctionName = objElement.firstChildElement(hmf::optimization::functionname).text();
            objectives.mWeight = objElement.firstChildElement(hmf::optimization::weight).text().toDouble();
            objectives.mNorm = objElement.firstChildElement(hmf::optimization::norm).text().toDouble();
            objectives.mExp = objElement.firstChildElement(hmf::optimization::exp).text().toDouble();

            QDomElement variablesElement = objElement.firstChildElement(hmf::sensitivityanalysis::plotvariables);
            if(!variablesElement.isNull())
            {
                QDomElement varElement = variablesElement.firstChildElement(hmf::sensitivityanalysis::plotvariable);
                while (!varElement.isNull())
                {
                    QStringList variableInfo;

                    variableInfo.append(varElement.firstChildElement(hmf::component).text());
                    variableInfo.append(varElement.firstChildElement(hmf::port).text());
                    variableInfo.append(varElement.firstChildElement(hmf::sensitivityanalysis::plotvariable).text());

                    objectives.mVariableInfo.append(variableInfo);

                    varElement = varElement.nextSiblingElement(hmf::sensitivityanalysis::plotvariable);
                }
            }

            QDomElement dataElement = objElement.firstChildElement(hmf::optimization::data);
            if(!dataElement.isNull())
            {
                QDomElement parElement = dataElement.firstChildElement(hmf::parameter::root);
                while (!parElement.isNull())
                {
                    objectives.mData.append(parElement.text());

                    parElement = parElement.nextSiblingElement(hmf::parameter::root);
                }
            }

            objElement = objElement.nextSiblingElement(hmf::optimization::objective);

            mOptSettings.mObjectives.append(objectives);
        }
    }
}


void SystemObject::getSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    sensSettings = mSensSettings;
}


void SystemObject::setSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    mSensSettings = sensSettings;
}


void SystemObject::getOptimizationSettings(OptimizationSettings &optSettings)
{
    optSettings = mOptSettings;
}


void SystemObject::setOptimizationSettings(OptimizationSettings &optSettings)
{
    mOptSettings = optSettings;
}


//! @brief Saves the System specific GUI data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
QDomElement SystemObject::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    QDomElement guiStuff = ModelObject::saveGuiDataToDomElement(rDomElement);

    //Save animation disabled setting
    QDomElement animationElement = guiStuff.firstChildElement(hmf::animation);
    animationElement.setAttribute(hmf::appearance::disabled, bool2str(mAnimationDisabled));

    //Should we try to append appearancedata stuff, we don't want this in external systems as they contain their own appearance
    if (mLoadType!="EXTERNAL")
    {
        //Append system meta info
        QString author, email, affiliation, description;
        getModelInfo(author, email, affiliation, description);
        if (!(author.isEmpty() && email.isEmpty() && affiliation.isEmpty() && description.isEmpty()))
        {
            QDomElement infoElement = appendDomElement(guiStuff, hmf::modelinfo::root);
            appendDomTextNode(infoElement, hmf::modelinfo::author, author);
            appendDomTextNode(infoElement, hmf::modelinfo::email, email);
            appendDomTextNode(infoElement, hmf::modelinfo::affiliation, affiliation);
            appendDomTextNode(infoElement, hmf::modelinfo::description, description);
        }

        GraphicsViewPort vp = this->getGraphicsViewport();
        appendViewPortTag(guiStuff, vp.mCenter.x(), vp.mCenter.y(), vp.mZoom);

        QDomElement portsHiddenElement = appendDomElement(guiStuff, hmf::ports);
        portsHiddenElement.setAttribute("hidden", !mShowSubComponentPorts);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, hmf::names);
        namesHiddenElement.setAttribute("hidden", !mShowSubComponentNames);

        QString gfxType = "iso";
        if(mGfxType == UserGraphics)
            gfxType = "user";
        QDomElement gfxTypeElement = appendDomElement(guiStuff, hmf::appearance::graphics);
        gfxTypeElement.setAttribute("type", gfxType);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentSystemObject() != 0)
        {
            this->mModelObjectAppearance.setBasePath(this->getParentSystemObject()->getAppearanceData()->getBasePath());
        }
        this->mModelObjectAppearance.saveToDomElement(xmlApp);
    }

    saveOptimizationSettingsToDomElement(guiStuff);
    saveSensitivityAnalysisSettingsToDomElement(guiStuff);

    //Save undo stack if setting is activated
    if(mSaveUndoStack)
    {
        guiStuff.appendChild(mpUndoStack->toXml());
    }

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void SystemObject::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    //qDebug() << "Saving to dom node in: " << this->mModelObjectAppearance.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, getHmfTagName());

    //! @todo maybe use enums instead of strings
    //! @todo should not need to set this here
    if (mpParentSystemObject==0)
    {
        mLoadType = "ROOT"; //!< @todo this is a temporary hack for the xml save function (see bellow)
    }
    else if (!mModelFileInfo.filePath().isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    // Save Core related stuff
    this->saveCoreDataToDomElement(xmlSubsystem, contents);

    if(contents==FullModel)
    {
        // Save gui object stuff
        this->saveGuiDataToDomElement(xmlSubsystem);
    }

    //Replace volunector with connectors and component
    QList<Connector*> volunectorPtrs;
    QList<Connector*> tempConnectorPtrs;  //To be removed later
    QList<ModelObject*> tempComponentPtrs; //To be removed later
    for(int i=0; i<mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->isVolunector())
        {
            Connector *pVolunector = mSubConnectorList[i];
            volunectorPtrs.append(pVolunector);
            mSubConnectorList.removeAll(pVolunector);
            --i;

            tempComponentPtrs.append(pVolunector->getVolunectorComponent());

            tempConnectorPtrs.append(new Connector(this));
            tempConnectorPtrs.last()->setStartPort(pVolunector->getStartPort());
            tempConnectorPtrs.last()->setEndPort(tempComponentPtrs.last()->getPort("P1"));
            QVector<QPointF> points;
            QStringList geometries;
            points.append(pVolunector->mapToScene(pVolunector->getLine(0)->line().p1()));
            for(int j=0; j<pVolunector->getNumberOfLines(); ++j)
            {
                points.append(pVolunector->mapToScene(pVolunector->getLine(j)->line().p2()));
                if(pVolunector->getGeometry(j) == Horizontal)
                    geometries.append("horizontal");
                else if(pVolunector->getGeometry(j) == Vertical)
                    geometries.append("vertical");
                else
                    geometries.append("diagonal");
            }
            tempConnectorPtrs.last()->setPointsAndGeometries(points, geometries);

            tempConnectorPtrs.append(new Connector(this));
            tempConnectorPtrs.last()->setStartPort(tempComponentPtrs.last()->getPort("P2"));
            tempConnectorPtrs.last()->setEndPort(pVolunector->getEndPort());
        }

        for(int j=0; j<tempComponentPtrs.size(); ++j)
        {
            mModelObjectMap.insert(tempComponentPtrs[j]->getName(), tempComponentPtrs[j]);
        }
    }
    mSubConnectorList.append(tempConnectorPtrs);

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save subcomponents and subsystems
        QDomElement xmlObjects = appendDomElement(xmlSubsystem, hmf::objects);
        ModelObjectMapT::iterator it;
        for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        {
            // TODO dont save containerports if parameters only
            it.value()->saveToDomElement(xmlObjects, contents);
            if(tempComponentPtrs.contains(it.value()))
            {
                xmlObjects.lastChildElement().setAttribute("volunector", "true");
            }
        }

        if(contents==FullModel)
        {
                //Save all widgets
            QMap<size_t, Widget *>::iterator itw;
            for(itw = mWidgetMap.begin(); itw!=mWidgetMap.end(); ++itw)
            {
                itw.value()->saveToDomElement(xmlObjects);
            }

                //Save the connectors
            QDomElement xmlConnections = appendDomElement(xmlSubsystem, hmf::connections);
            for(int i=0; i<mSubConnectorList.size(); ++i)
            {
                mSubConnectorList[i]->saveToDomElement(xmlConnections);
            }
        }
    }

    //Remove temporary connectors/components and re-add volunectors
    for(int i=0; i<tempConnectorPtrs.size(); ++i)
    {
        mSubConnectorList.removeAll(tempConnectorPtrs[i]);

        Connector *pConnector = tempConnectorPtrs[i];
        Port *pStartPort = pConnector->getStartPort();
        ModelObject *pStartComponent = pStartPort->getParentModelObject();
        Port *pEndPort = pConnector->getEndPort();
        ModelObject *pEndComponent = pEndPort->getParentModelObject();

        pStartPort->forgetConnection(pConnector);
        pStartComponent->forgetConnector(pConnector);
        pEndPort->forgetConnection(pConnector);
        pEndComponent->forgetConnector(pConnector);

        delete(tempConnectorPtrs[i]);
    }
    for(int i=0; i<volunectorPtrs.size(); ++i)
    {
        mSubConnectorList.append(volunectorPtrs[i]);
    }
    for(int i=0; i<tempComponentPtrs.size(); ++i)
    {
        mModelObjectMap.remove(tempComponentPtrs[i]->getName());
    }

}

//! @brief Loads a System from an XML DOM Element
//! @param[in] rDomElement The element to load from
void SystemObject::loadFromDomElement(QDomElement domElement)
{
    // Loop back up to root level to get version numbers
    QString hmfFormatVersion = domElement.ownerDocument().firstChildElement(hmf::root).attribute(hmf::version::hmf, "0");
    QString coreHmfVersion = domElement.ownerDocument().firstChildElement(hmf::root).attribute(hmf::version::hopsancore, "0");

    // Check if the subsystem is external or internal, and load appropriately
    QString external_path = domElement.attribute(hmf::externalpath);
    if (external_path.isEmpty())
    {
        // Load embedded subsystem
        // 0. Load core and gui stuff
        //! @todo might need some error checking here in case some fields are missing
        // Now load the core specific data, might need inherited function for this
        this->setName(domElement.attribute(hmf::name));

        // Load the NumHop script
        setNumHopScript(parseDomStringNode(domElement.firstChildElement(hmf::numhopscript), ""));

        // Begin loading GUI stuff like appearance data and viewport
        QDomElement guiStuff = domElement.firstChildElement(hmf::hopsangui);
        mModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(caf::root).firstChildElement(caf::modelobject));
        refreshDisplayName(); // This must be done because in some occasions the loadAppearanceData line above will overwrite the correct name

        QDomElement animationElement = guiStuff.firstChildElement(hmf::animation);
        bool animationDisabled = false;
        if(!animationElement.isNull())
        {
            animationDisabled = parseAttributeBool(animationElement, hmf::appearance::disabled, false);
        }
        setAnimationDisabled(animationDisabled);

        // Load system/model info
        QDomElement infoElement = domElement.parentNode().firstChildElement(hmf::modelinfo::root); //!< @deprecated info tag is in the system from 0.7.5 an onwards, this line loads from old models
        if (infoElement.isNull())
        {
            infoElement = guiStuff.firstChildElement(hmf::modelinfo::root);
        }
        if(!infoElement.isNull())
        {
            QString author, email, affiliation, description;
            QDomElement authorElement = infoElement.firstChildElement(hmf::modelinfo::author);
            if(!authorElement.isNull())
            {
                author = authorElement.text();
            }
            QDomElement emailElement = infoElement.firstChildElement(hmf::modelinfo::email);
            if(!emailElement.isNull())
            {
                email = emailElement.text();
            }
            QDomElement affiliationElement = infoElement.firstChildElement(hmf::modelinfo::affiliation);
            if(!affiliationElement.isNull())
            {
                affiliation = affiliationElement.text();
            }
            QDomElement descriptionElement = infoElement.firstChildElement(hmf::modelinfo::description);
            if(!descriptionElement.isNull())
            {
                description = descriptionElement.text();
            }

            this->setModelInfo(author, email, affiliation, description);
        }

        // Now lets check if the icons were loaded successfully else we may want to ask the library widget for the graphics (components saved as subsystems)
        if (!mModelObjectAppearance.iconValid(UserGraphics) || !mModelObjectAppearance.iconValid(ISOGraphics))
        {
            SharedModelObjectAppearanceT pApp = gpLibraryHandler->getModelObjectAppearancePtr(mModelObjectAppearance.getTypeName(), mModelObjectAppearance.getSubTypeName());
            if (pApp)
            {
                // If our user graphics is invalid but library has valid data then set from library
                if (!mModelObjectAppearance.iconValid(UserGraphics) && pApp->iconValid(UserGraphics))
                {
                    setIconPath(pApp->getIconPath(UserGraphics, Absolute), UserGraphics, Absolute);
                }

                // If our iso graphics is invalid but library has valid data then set from library
                if (!mModelObjectAppearance.iconValid(ISOGraphics) && pApp->iconValid(ISOGraphics))
                {
                    setIconPath(pApp->getIconPath(ISOGraphics, Absolute), ISOGraphics, Absolute);
                }
            }
        }

        // Continue loading GUI stuff like appearance data and viewport
        this->mShowSubComponentNames = !parseAttributeBool(guiStuff.firstChildElement(hmf::names),"hidden",true);
        this->mShowSubComponentPorts = !parseAttributeBool(guiStuff.firstChildElement(hmf::ports),"hidden",true);
        QString gfxType = guiStuff.firstChildElement(hmf::appearance::graphics).attribute("type");
        if(gfxType == "user") { mGfxType = UserGraphics; }
        else if(gfxType == "iso") { mGfxType = ISOGraphics; }
        //! @todo these two should not be set here
        gpToggleNamesAction->setChecked(mShowSubComponentNames);
        gpTogglePortsAction->setChecked(mShowSubComponentPorts);
        double x = guiStuff.firstChildElement(hmf::appearance::viewport).attribute("x").toDouble();
        double y = guiStuff.firstChildElement(hmf::appearance::viewport).attribute("y").toDouble();
        double zoom = guiStuff.firstChildElement(hmf::appearance::viewport).attribute("zoom").toDouble();

        bool dontClearUndo = false;
        if(!guiStuff.firstChildElement(hmf::undo).isNull())
        {
            QDomElement undoElement = guiStuff.firstChildElement(hmf::undo);
            mpUndoStack->fromXml(undoElement);
            dontClearUndo = true;
            mSaveUndoStack = true;      //Set save undo stack setting to true if loading a hmf file with undo stack saved
        }

        // Only set viewport and zoom if the system being loaded is the one shown in the view
        // But make system remember the setting anyway
        this->setGraphicsViewport(GraphicsViewPort(x,y,zoom));
        if (mpModelWidget->getViewContainerObject() == this)
        {
            mpModelWidget->getGraphicsView()->setViewPort(GraphicsViewPort(x,y,zoom));
        }

        //Load simulation time
        QString startT,stepT,stopT;
        bool inheritTs;
        parseSimulationTimeTag(domElement.firstChildElement(hmf::simulationtime), startT, stepT, stopT, inheritTs);
        this->setTimeStep(stepT.toDouble());
        mpCoreSystemAccess->setInheritTimeStep(inheritTs);

        // Load number of log samples
        parseLogSettingsTag(domElement.firstChildElement(hmf::simulationlogsettings), mLogStartTime, mNumberOfLogSamples);
        //! @deprecated 20131002 we keep this below for backwards compatibility for a while
        if(domElement.hasAttribute(hmf::logsamples))
        {
            mNumberOfLogSamples = domElement.attribute(hmf::logsamples).toInt();
        }

        // Only set start stop time for the top level system
        if (mpParentSystemObject == 0)
        {
            mpModelWidget->setTopLevelSimulationTime(startT,stepT,stopT);
        }

        // Update system wide model properties
        updateHmfSystemProperties(domElement, hmfFormatVersion, coreHmfVersion);

        //1. Load global parameters
        QDomElement xmlParameters = domElement.firstChildElement(hmf::parameters);
        QDomElement xmlSubObject = xmlParameters.firstChildElement(hmf::parameter::root);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, true, hmfFormatVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::parameter::root);
        }

        //2. Load all sub-components
        QList<ModelObject*> volunectorObjectPtrs;
        QDomElement xmlSubObjects = domElement.firstChildElement(hmf::objects);
        xmlSubObject = xmlSubObjects.firstChildElement(hmf::component);
        while (!xmlSubObject.isNull())
        {
            updateHmfComponentProperties(xmlSubObject, hmfFormatVersion, coreHmfVersion);
            ModelObject* pObj = loadModelObject(xmlSubObject, this, NoUndo);
            if(pObj == nullptr)
            {
                gpMessageHandler->addErrorMessage(QString("Model contains component from a library that has not been loaded. TypeName: ") +
                                                                    xmlSubObject.attribute(hmf::typenametag) + QString(", Name: ") + xmlSubObject.attribute(hmf::name));

                // Insert missing component dummy instead
                QString typeName = xmlSubObject.attribute(hmf::typenametag);
                xmlSubObject.setAttribute(hmf::typenametag, "MissingComponent");
                xmlSubObject.setAttribute(hmf::subtypename, "");
                pObj = loadModelObject(xmlSubObject, this, NoUndo);
                xmlSubObject.setAttribute(hmf::typenametag, typeName);
                pObj->setFallbackDomElement(xmlSubObject);
            }
            else
            {



                //! @deprecated This StartValue load code is only kept for up converting old files, we should keep it here until we have some other way of up converting old formats
                //Load start values //Is not needed, start values are saved as ordinary parameters! This code snippet can probably be removed.
                QDomElement xmlStartValues = xmlSubObject.firstChildElement(hmf::startvalues);
                QDomElement xmlStartValue = xmlStartValues.firstChildElement(hmf::startvalue);
                while (!xmlStartValue.isNull())
                {
                    loadStartValue(xmlStartValue, pObj, NoUndo);
                    xmlStartValue = xmlStartValue.nextSiblingElement(hmf::startvalue);
                }
            }
            if(xmlSubObject.attribute("volunector") == "true")
            {
                volunectorObjectPtrs.append(pObj);
            }

            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::component);
        }

        //3. Load all text box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(hmf::widget::textboxwidget);
        while (!xmlSubObject.isNull())
        {
            loadTextBoxWidget(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::widget::textboxwidget);
        }

        //4. Load all image widgets
        xmlSubObject = xmlSubObjects.firstChildElement(hmf::widget::imagewidget);
        while (!xmlSubObject.isNull())
        {
            loadImageWidget(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::widget::imagewidget);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(hmf::system);
        while (!xmlSubObject.isNull())
        {
            loadModelObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::system);
        }

        //6. Load all system ports
        xmlSubObject = xmlSubObjects.firstChildElement(hmf::systemport);
        while (!xmlSubObject.isNull())
        {
            loadSystemPortObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::systemport);
        }

        //7. Load all connectors
        QDomElement xmlConnections = domElement.firstChildElement(hmf::connections);
        xmlSubObject = xmlConnections.firstChildElement(hmf::connector::root);
        QList<QDomElement> failedConnections;
        while (!xmlSubObject.isNull())
        {
            if(!loadConnector(xmlSubObject, this, NoUndo))
            {
//                failedConnections.append(xmlSubObject);
            }
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::connector::root);
        }
//        //If some connectors failed to load, it could mean that they were loaded in wrong order.
//        //Try again until they work, or abort if number of attempts are greater than maximum possible for success.
//        int stop=failedConnections.size()*(failedConnections.size()+1)/2;
//        int i=0;
//        while(!failedConnections.isEmpty())
//        {
//            if(!loadConnector(failedConnections.first(), this, NoUndo))
//            {
//                failedConnections.append(failedConnections.first());
//            }
//            failedConnections.removeFirst();
//            ++i;
//            if(i>stop) break;
//        }


        //8. Load system parameters again in case we need to reregister system port start values
        xmlParameters = domElement.firstChildElement(hmf::parameters);
        xmlSubObject = xmlParameters.firstChildElement(hmf::parameter::root);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, false, hmfFormatVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::parameter::root);
        }

        //9. Load plot variable aliases
        QDomElement xmlAliases = domElement.firstChildElement(hmf::aliases);
        QDomElement xmlAlias = xmlAliases.firstChildElement(hmf::alias);
        while (!xmlAlias.isNull())
        {
            loadPlotAlias(xmlAlias, this);
            xmlAlias = xmlAlias.nextSiblingElement(hmf::alias);
        }

        //9.1 Load plot variable aliases
        //! @deprecated Remove in the future when hmf format stabilized and everyone has upgraded
        xmlSubObject = xmlParameters.firstChildElement(hmf::alias);
        while (!xmlSubObject.isNull())
        {
            loadPlotAlias(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(hmf::alias);
        }

        //10. Load optimization settings
        xmlSubObject = guiStuff.firstChildElement(hmf::optimization::root);
        loadOptimizationSettingsFromDomElement(xmlSubObject);

        //11. Load sensitivity analysis settings
        xmlSubObject = guiStuff.firstChildElement(hmf::sensitivityanalysis::root);
        loadSensitivityAnalysisSettingsFromDomElement(xmlSubObject);


        //Replace volunector components with volunectors
        for(int i=0; i<volunectorObjectPtrs.size(); ++i)
        {
            if(volunectorObjectPtrs[i]->getPort("P1")->isConnected() &&
                volunectorObjectPtrs[i]->getPort("P2")->isConnected())
            {


                Port *pP1 = volunectorObjectPtrs[i]->getPort("P1");
                Port *pP2 = volunectorObjectPtrs[i]->getPort("P2");
                Connector *pVolunector = pP1->getAttachedConnectorPtrs().first();
                Connector *pExcessiveConnector = pP2->getAttachedConnectorPtrs().first();
                Port *pEndPort = pExcessiveConnector->getEndPort();
                ModelObject *pEndComponent = pEndPort->getParentModelObject();
                ModelObject *pVolunectorObject = volunectorObjectPtrs[i];


                //Forget and remove excessive connector
                mSubConnectorList.removeAll(pExcessiveConnector);
                pVolunectorObject->forgetConnector(pExcessiveConnector);    //Start component
                pEndComponent->forgetConnector(pExcessiveConnector);        //Start port
                pP2->forgetConnection(pExcessiveConnector);                 //End component
                pEndPort->forgetConnection(pExcessiveConnector);            //End port
                delete(pExcessiveConnector);

                //Disconnect volunector from volunector component
                pVolunectorObject->forgetConnector(pVolunector);
                pP1->forgetConnection(pVolunector);

                //Re-connect volunector with end component
                pVolunector->setEndPort(pEndPort);

                //Make the connector a volunector
                pVolunector->makeVolunector(dynamic_cast<Component*>(pVolunectorObject));

                //Remove volunector object parent container object
                mModelObjectMap.remove(pVolunectorObject->getName());
                pVolunectorObject->setParent(0);

                //Re-draw connector object
                pVolunector->drawConnector();
            }
        }


        //Refresh the appearance of the subsystem and create the GUIPorts based on the loaded portappearance information
        //! @todo This is a bit strange, refreshAppearance MUST be run before create ports or create ports will not know some necessary stuff
        this->refreshAppearance();
        this->refreshExternalPortsAppearanceAndPosition();
        //this->createPorts();

        //Deselect all components
        this->deselectAll();
        if(!dontClearUndo)
        {
            this->mpUndoStack->clear();
        }
        //Only do this for the root system
        //! @todo maybe can do this for subsystems to (even if we don't see them right now)
        if (this->mpParentSystemObject == nullptr)
        {
            //mpParentModelWidget->getGraphicsView()->centerView();
            mpModelWidget->getGraphicsView()->updateViewPort();
        }
        this->mpModelWidget->setSaved(true);

        emit systemParametersChanged(); // Make sure we refresh the syspar widget
        emit checkMessages();
    }
    else
    {
        gpMessageHandler->addErrorMessage("A system you tried to load is tagged as an external system, but the ContainerSystem load function only loads embedded systems");
    }
}







//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
//! @param[in] relModelPath Relative filepath to parent model file (model asset path)
void SystemObject::setModelFileInfo(QFile &rFile, const QString relModelPath)
{
    mModelFileInfo.setFile(rFile);
    if (!relModelPath.isEmpty())
    {
        getCoreSystemAccessPtr()->setExternalModelFilePath(relModelPath);
    }
}

void SystemObject::loadParameterValuesFromFile(QString parameterFile)
{
    if(parameterFile.isEmpty()) {
        QString openLocation = gpConfig->getStringSetting(cfg::dir::parameterimport);
        if (openLocation.isEmpty()) {
            openLocation = gpConfig->getStringSetting(cfg::dir::loadmodel);
        }
        parameterFile = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Parameter File"),
                                                     openLocation,
                                                     tr("Hopsan Parameter Files (*.hpf *.xml)"));
    }

    if(!parameterFile.isEmpty()) {
        auto numChanged = getCoreSystemAccessPtr()->loadParameterFile(parameterFile);
        if (numChanged > 0) {
            mpModelWidget->hasChanged();
            // Trigger system parameter widget refresh, regardless if any system parameters actually changed
            emit systemParametersChanged();
        }
        gpConfig->setStringSetting(cfg::dir::parameterimport,  QFileInfo(parameterFile).absolutePath());
    }
    emit checkMessages();
}

void SystemObject::getAllParametersAndValuesRecursively(QString prefix, QStringList &names, QStringList &values, QStringList &dataTypes, QStringList &quantities, QStringList &units)
{
    QVector<CoreParameterData> parameters;
    getParameters(parameters);
    for(const CoreParameterData &parameter : parameters) {
        names.append(prefix+parameter.mName);
        values.append(parameter.mValue);
        dataTypes.append(parameter.mType);
        quantities.append(parameter.mQuantity);
        units.append(parameter.mUnit);
    }

    for(ModelObject *modelObject : getModelObjects()) {
        if(modelObject->getTypeName() == "Subsystem") {
            qobject_cast<SystemObject*>(modelObject)->getAllParametersAndValuesRecursively(prefix+modelObject->getName()+"|", names, values, dataTypes, quantities, units);
        }
        else {
            QVector<CoreParameterData> parameters;
            modelObject->getParameters(parameters);
            for(const CoreParameterData &parameter : parameters) {
                names.append(prefix+modelObject->getName()+"."+parameter.mName);
                values.append(parameter.mValue);
                dataTypes.append(parameter.mType);
                quantities.append(parameter.mQuantity);
                units.append(parameter.mUnit);
            }
        }
    }
}

//! @brief Function to set the time step of the current system
void SystemObject::setTimeStep(const double timeStep)
{
    mpCoreSystemAccess->setDesiredTimeStep(timeStep);
    this->hasChanged();
}

void SystemObject::setVisibleIfSignal(bool visible)
{
    if(this->getTypeCQS() == "S")
    {
        this->setVisible(visible);
    }
}

//! @brief Returns the time step value of the current project.
double SystemObject::getTimeStep()
{
    return mpCoreSystemAccess->getDesiredTimeStep();
}

//! @brief Check if the system inherits timestep from its parent
bool SystemObject::doesInheritTimeStep()
{
    return mpCoreSystemAccess->doesInheritTimeStep();
}


//! @brief Returns the number of samples value of the current project.
//! @see setNumberOfLogSamples(double)
size_t SystemObject::getNumberOfLogSamples()
{
    return mNumberOfLogSamples;
}


//! @brief Sets the number of samples value for the current project
//! @see getNumberOfLogSamples()
void SystemObject::setNumberOfLogSamples(size_t nSamples)
{
    mNumberOfLogSamples = nSamples;
}

double SystemObject::getLogStartTime() const
{
    return mLogStartTime;
}

void SystemObject::setLogStartTime(const double logStartT)
{
    mLogStartTime = logStartT;
}


OptimizationSettings::OptimizationSettings()
{
    // Default values
    mScriptFile = QString();
    mNiter=100;
    mNsearchp=8;
    mRefcoeff=1.3;
    mRandfac=.3;
    mForgfac=0.0;
    mPartol=.0001;
    mPlot=true;
    mSavecsv=false;
    mFinalEval=true;
    mlogPar = false;
}


SensitivityAnalysisSettings::SensitivityAnalysisSettings()
{
    nIter = 100;
    distribution = UniformDistribution;
}
