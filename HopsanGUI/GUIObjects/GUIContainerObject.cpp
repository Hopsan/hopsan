/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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
#include "GUISystem.h"
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
#include "Widgets/PyDockWidget.h"
#include "Widgets/QuickNavigationWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/DataExplorer.h"
#include "Widgets/FindWidget.h"
#include "Widgets/MessageWidget.h"
#include "PlotHandler.h"
#include "Utilities/HelpPopUpWidget.h"

//! @brief Constructor for container objects.
//! @param position Initial position where container object is to be placed in its parent container
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to the appearance data object
//! @param startSelected Tells whether or not the object is initially selected
//! @param gfxType Tells whether the initial graphics shall be user or ISO
//! @param pParentContainer Pointer to the parent container object (leave empty if not a sub container)
//! @param pParent Pointer to parent object
ContainerObject::ContainerObject(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParent)
{
    // Initialize
    mIsCreatingConnector = false;
    mShowSubComponentPorts = gpMainWindow->mpTogglePortsAction->isChecked();
    mShowSubComponentNames = gpMainWindow->mpToggleNamesAction->isChecked();
    mSignalsHidden = gpMainWindow->mpToggleSignalsAction->isChecked();
    mLossesVisible = false;
    mUndoDisabled = false;
    mGfxType = UserGraphics;

    mPasteOffset = -30;

    // Create the scene
    mpScene = new QGraphicsScene(this);
    mGraphicsViewPort = GraphicsViewPort(2500, 2500, 1.0); // Default should be centered

    // Create the undostack
    mpUndoStack = new UndoStack(this);
    mpUndoStack->clear();

    mpDragCopyStack = new CopyStack();

    // Establish connections that should always remain
    connect(this, SIGNAL(checkMessages()), gpMessageHandler, SLOT(collectHopsanCoreMessages()), Qt::UniqueConnection);
}


//! @brief Destructor for container object
ContainerObject::~ContainerObject()
{
    //qDebug() << ",,,,,,,,,,,,GUIContainer destructor";
}

bool ContainerObject::isTopLevelContainer() const
{
    return (mpParentContainerObject==0);
}

QStringList ContainerObject::getSystemNameHieararchy() const
{
    QStringList parentSystemNames;
    // Note! This wil lreturn empty lsit for top-level system, and that is OK, it is supposed to do that
    if (mpParentContainerObject)
    {
        parentSystemNames = mpParentContainerObject->getSystemNameHieararchy();
        parentSystemNames << this->getName();
    }
    return parentSystemNames;
}

//! @brief Notify the parent project tab that changes has occurred
void ContainerObject::hasChanged()
{
    mpModelWidget->hasChanged();
}

//! @brief Connects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are switching what container we want the buttons to trigger actions in
void ContainerObject::makeMainWindowConnectionsAndRefresh()
{
    connect(gpMainWindow->mpUndoAction, SIGNAL(triggered()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpMainWindow->mpRedoAction, SIGNAL(triggered()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getUndoButton(),  SIGNAL(clicked()), this, SLOT(undo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getRedoButton(),  SIGNAL(clicked()), this, SLOT(redo()), Qt::UniqueConnection);
    connect(gpUndoWidget->getClearButton(), SIGNAL(clicked()), this, SLOT(clearUndo()), Qt::UniqueConnection);

    connect(gpMainWindow->mpTogglePortsAction,    SIGNAL(triggered(bool)),    this,     SLOT(showSubcomponentPorts(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleNamesAction,    SIGNAL(triggered(bool)),    this,     SLOT(toggleNames(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpToggleSignalsAction,  SIGNAL(triggered(bool)),    this,     SLOT(toggleSignals(bool)), Qt::UniqueConnection);
    connect(gpMainWindow->mpDisableUndoAction,    SIGNAL(triggered(bool)),    this,     SLOT(setUndoDisabled(bool)), Qt::UniqueConnection);
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

    // Update the main window toolbar action buttons that are system specific
    gpMainWindow->mpTogglePortsAction->setChecked(mShowSubComponentPorts);
    gpMainWindow->mpToggleNamesAction->setChecked(mShowSubComponentNames);
    gpMainWindow->mpToggleSignalsAction->setChecked(!mSignalsHidden);

    // Update main window widgets with data from this container
    gpFindWidget->setContainer(this);
    gpMainWindow->mpSystemParametersWidget->update(this);
    gpUndoWidget->refreshList();
    gpMainWindow->mpUndoAction->setDisabled(this->mUndoDisabled);
    gpMainWindow->mpRedoAction->setDisabled(this->mUndoDisabled);
}

//! @brief Disconnects all SignalAndSlot connections to the mainwindow buttons from this container
//! This is useful when we are switching what container we want the buttons to trigger actions in
void ContainerObject::unmakeMainWindowConnectionsAndRefresh()
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
//! @param[in] pt The position of this object, used to determine the center relative position
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
void ContainerObject::refreshExternalPortsAppearanceAndPosition()
{
    //refresh the external port poses
    ModelObjectMapT::iterator moit;
    double val;

    //Set the initial values to be overwritten by the if bellow
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
    //! @todo Find out if it is possible to ask the scene or view for this information instead of calculating it ourselves
    QPointF center = QPointF((xMax+xMin)/2.0, (yMax+yMin)/2.0);
    //qDebug() << "center max min: " << center << " " << xMin << " " << xMax << " " << yMin << " " << yMax;

    QMap<double, Port*> leftEdge, rightEdge, topEdge, bottomEdge;
    for(moit = mModelObjectMap.begin(); moit != mModelObjectMap.end(); ++moit)
    {
        if(moit.value()->type() == ContainerPortType)
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
                PortAppearanceMapT::iterator pamit = mModelObjectAppearance.getPortAppearanceMap().find(pPort->getName());
                pPort->setCenterPosByFraction(pamit.value().x, pamit.value().y);
                pPort->setRotation(pamit.value().rot);
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
void ContainerObject::refreshAppearance()
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
void ContainerObject::calcSubsystemPortPosition(const double w, const double h, const double angle, double &x, double &y)
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


//! @brief Returns a pointer to the CoreSystemAccess that this container represents
//! @returns Pointer the the CoreSystemAccess that this container represents
CoreSystemAccess *ContainerObject::getCoreSystemAccessPtr()
{
    //Should be overloaded
    return 0;
}


//! @brief Returns a pointer to the contained scene
QGraphicsScene *ContainerObject::getContainedScenePtr()
{
    return mpScene;
}

void ContainerObject::setGraphicsViewport(GraphicsViewPort vp)
{
    mGraphicsViewPort = vp;
}

GraphicsViewPort ContainerObject::getGraphicsViewport() const
{
    return mGraphicsViewPort;
}


//! @brief This method creates ONE external port. Or refreshes existing ports. It assumes that port appearance information for this port exists
//! @param[portName] The name of the port to create
//! @todo maybe default create that info if it is missing
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

        //! @todo to minimize search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = this->getCoreSystemAccessPtr()->getNodeType(it.key(), it.key());
        QString portType = this->getCoreSystemAccessPtr()->getPortType(it.key(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        double x = it.value().x;
        double y = it.value().y;
        //qDebug() << "x,y: " << x << " " << y;

        pPort = new Port(it.key(), x*boundingRect().width(), y*boundingRect().height(), &(it.value()), this);

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

        qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << it.key() << " in: " << this->getName();
    }

    return pPort;
}




//! @brief Renames an external GUIPort
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
ModelObject* ContainerObject::addModelObject(QString fullTypeName, QPointF position, double rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
    ModelObjectAppearance *pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(fullTypeName);

    if(!pAppearanceData)    //Not an existing component
    {
        return 0;       //No error message here, it depends on from where this function is called
    }
    else if(!pAppearanceData->getHmfFile().isEmpty())
    {
        QString hmfFile = pAppearanceData->getHmfFile();
        QString subTypeName = pAppearanceData->getSubTypeName();
        ContainerObject *pObj = dynamic_cast<ContainerObject*>(addModelObject("Subsystem", position, rotation, startSelected, nameStatus, undoSettings));
        pObj->setSubTypeName(subTypeName);
        //pObj->clearContents();

        QFile file(pAppearanceData->getBasePath()+hmfFile);

        QDomDocument domDocument;
        QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
        if (!hmfRoot.isNull())
        {
            //! @todo Check version numbers
            //! @todo check if we could load else give error message and don't attempt to load
            QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
            pObj->setModelFileInfo(file); //Remember info about the file from which the data was loaded
            QFileInfo fileInfo(file);
            pObj->setAppearanceDataBasePath(fileInfo.absolutePath());
            pObj->loadFromDomElement(systemElement);
            pObj->setIconPath(pAppearanceData->getIconPath(UserGraphics, Absolute), UserGraphics, Absolute);
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
ModelObject* ContainerObject::addModelObject(ModelObjectAppearance *pAppearanceData, QPointF position, double rotation, SelectionStatusEnumT startSelected, NameVisibilityEnumT nameStatus, UndoStatusEnumT undoSettings)
{
        //Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllConnectors();

    QString componentTypeName = pAppearanceData->getTypeName();
    if (componentTypeName == HOPSANGUISYSTEMTYPENAME || componentTypeName == HOPSANGUICONDITIONALSYSTEMTYPENAME)
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
        gpMessageHandler->addErrorMessage("Trying to add component with name: " + mpTempGUIModelObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
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
    else if (nameStatus == UseDefault)
    {
        if (areSubComponentNamesShown())
        {
            mpTempGUIModelObject->showName(NoUndo);
        }
        else
        {
            mpTempGUIModelObject->hideName(NoUndo);
        }
    }

    return mpTempGUIModelObject;
}


bool ContainerObject::areLossesVisible()
{
    return mLossesVisible;
}


TextBoxWidget *ContainerObject::addTextBoxWidget(QPointF position, UndoStatusEnumT undoSettings)
{
    return addTextBoxWidget(position, 0, undoSettings);
}

TextBoxWidget *ContainerObject::addTextBoxWidget(QPointF position, const int desiredWidgetId, UndoStatusEnumT undoSettings)
{
    TextBoxWidget *pNewTextBoxWidget;
    if (mWidgetMap.contains(desiredWidgetId))
    {
        pNewTextBoxWidget = new TextBoxWidget("Text", position, 0, Deselected, this, mWidgetMap.keys().last()+1);
    }
    else
    {
        pNewTextBoxWidget = new TextBoxWidget("Text", position, 0, Deselected, this, desiredWidgetId);
    }
    mWidgetMap.insert(pNewTextBoxWidget->getWidgetIndex(), pNewTextBoxWidget);

    if(undoSettings == Undo)
    {
        mpUndoStack->registerAddedWidget(pNewTextBoxWidget);
    }
    mpModelWidget->hasChanged();

    return pNewTextBoxWidget;
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

void ContainerObject::deleteWidget(const int id, UndoStatusEnumT undoSettings)
{
    Widget *pWidget = mWidgetMap.value(id, 0);
    if (pWidget)
    {
        deleteWidget(pWidget, undoSettings);
    }
}


//! @brief Delete ModelObject with specified name
//! @param rObjectName is the name of the component to delete
void ContainerObject::deleteModelObject(const QString &rObjectName, UndoStatusEnumT undoSettings)
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

        if (undoSettings == Undo && !mUndoDisabled)
        {
            // Register removal of model object in undo stack
            this->mpUndoStack->registerDeletedObject(pModelObject);
        }

        //! @todo maybe this should be handled somewhere else (not sure maybe this is the best place)
        if (pModelObject->type() == ContainerPortType )
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
            ModelObject* pModelObject = it.value();
            mModelObjectMap.erase(it);

            // Set new name, first in core then in gui object
            switch (pModelObject->type())
            {
            case ContainerPortType : //!< @todo What will happen when we try to rename a groupport
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
bool ContainerObject::hasModelObject(const QString &rName) const
{
    return (mModelObjectMap.count(rName) > 0);
}

//! @brief Takes ownership of supplied objects, widgets and connectors
//!
//! This method assumes that the previous owner have forgotten all about these objects, it however sets itself as new Qtparent, parentContainer and scene, overwriting the old values
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
            //! @todo what if name already taken, don't care for now as we shall only move into groups when they are created

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
        ModelObject *pTransPort = this->addModelObject(HOPSANGUICONTAINERPORTTYPENAME, portpos.toPoint(),0);

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

bool ContainerObject::setParameter(const CoreParameterData &rParameter, bool force)
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

bool ContainerObject::setOrAddParameter(const CoreParameterData &rParameter, bool force)
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

void ContainerObject::setNumHopScript(const QString &rScript)
{
    mNumHopScript = rScript;
    CoreSystemAccess *pCoreSys = getCoreSystemAccessPtr();
    if (pCoreSys)
    {
        pCoreSys->setNumHopScript(mNumHopScript);
    }
}

QString ContainerObject::getNumHopScript() const
{
    return mNumHopScript;
}

void ContainerObject::runNumHopScript(const QString &rScript, bool printOutput, QString &rOutput)
{
    CoreSystemAccess *pCoreSys = getCoreSystemAccessPtr();
    if (pCoreSys)
    {
        pCoreSys->runNumHopScript(rScript, printOutput, rOutput);
    }
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
ModelObject *ContainerObject::getModelObject(const QString &rModelObjectName)
{
    ModelObjectMapT::Iterator moit = mModelObjectMap.find(rModelObjectName);
    if (moit != mModelObjectMap.end())
    {
        return moit.value();
    }
    else
    {
        return nullptr;
    }
}

const QList<ModelObject *> ContainerObject::getModelObjects() const
{
    return mModelObjectMap.values();
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
    assert(!item == 0); // magse: strange? assert(item) ?
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


//! @brief Removes a specified connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void ContainerObject::removeSubConnector(Connector* pConnector, UndoStatusEnumT undoSettings)
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

            // Modelica component disconnect
            if(pStartP->getParentModelObject()->getTypeName() == MODELICATYPENAME ||
                    pEndP->getParentModelObject()->getTypeName() == MODELICATYPENAME)
            {
                success = true;
            }
            // Volunector disconnect
            else if ( pConnector->isVolunector() )
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
Connector* ContainerObject::createConnector(Port *pPort, UndoStatusEnumT undoSettings)
{
    // When clicking end port (finish creation of connector)
    if (mIsCreatingConnector)
    {
        bool success = false;
        if (mpTempConnector->isDangling() && pPort)
        {
            Port *pStartPort = mpTempConnector->getStartPort();
            Port *pEndPort = pPort;

            if(pStartPort->getNodeType() == "NodeModelica" || pEndPort->getNodeType() == "NodeModelica")
            {
                //! @todo Also make sure that the port in the Modelica code is correct physical type
                if((pStartPort->getNodeType() == "NodeModelica" && pEndPort->getNodeType() == "NodeModelica") ||
                   (pStartPort->getNodeType() == "NodeModelica" && pEndPort->getParentModelObject()->getTypeCQS() == "C") ||
                   (pEndPort->getNodeType() == "NodeModelica" && pStartPort->getParentModelObject()->getTypeCQS() == "C") ||
                   (pStartPort->getNodeType() == "NodeModelica" && pEndPort->getParentModelObject()->getTypeCQS() == "S") ||
                   (pEndPort->getNodeType() == "NodeModelica" && pStartPort->getParentModelObject()->getTypeCQS() == "S"))
                {
                    success = true;
                }
                else
                {
                    gpMessageHandler->addErrorMessage("Modelica ports can only be connected to other Modelica ports or C-type power ports.");
                    success = false;
                }
            }
#ifdef DEVELOPMENT
            else if(pStartPort->getNodeType() == "NodeHydraulic" &&                 //Connecting two Q-type hydraulic ports, add a "volunector"
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
#endif
            // Else treat as normal ports
            else
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
Connector* ContainerObject::createConnector(Port *pPort1, Port *pPort2, UndoStatusEnumT undoSettings)
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
void ContainerObject::cutSelected(CopyStack *xmlStack)
{
    this->copySelected(xmlStack);
    this->mpUndoStack->newPost(UNDO_CUT);
    emit deleteSelected();
    mpModelWidget->getGraphicsView()->updateViewPort();
}


//! @brief Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void ContainerObject::copySelected(CopyStack *xmlStack)
{
    // Don't copy if python widget or message widget as focus (they also use ctrl-c key sequence)
    if (!getContainedScenePtr()->hasFocus())
    {
        return;
    }

    // Check if we have any selected object, prevent clearing copy stack if nothing selected
    bool haveSelected = !mSelectedModelObjectsList.empty() || !mSelectedSubConnectorsList.empty() || !mSelectedWidgetsList.empty();
    if (!haveSelected)
    {
        return;
    }

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        gpCopyStack->clear();
        copyRoot = gpCopyStack->getCopyRoot();
    }
    else
    {
        xmlStack->clear();
        copyRoot = xmlStack->getCopyRoot();
    }

    // Store center point
    QPointF center = getCenterPointFromSelection();
    appendCoordinateTag(*copyRoot, center.x(), center.y());

    // Copy components
    QList<ModelObject *>::iterator it;
    for(it = mSelectedModelObjectsList.begin(); it!=mSelectedModelObjectsList.end(); ++it)
    {
        qDebug() << "Copying " << (*it)->getName();
        (*it)->saveToDomElement(*copyRoot, FullModel);

//        QString str;
//        QTextStream stream(&str);
//        QDomNode node = *copyRoot;
//        node.save(stream, 4);
//        qDebug() << str;

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

    // Copy connectors
    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if (mSubConnectorList[i]->isActive() && !mSubConnectorList[i]->isBroken())
        {
            Port *pStartPort = mSubConnectorList[i]->getStartPort();
            Port *pEndPort =  mSubConnectorList[i]->getEndPort();
            if (pStartPort && pEndPort)
            {
                if(pStartPort->getParentModelObject()->isSelected() && pEndPort->getParentModelObject()->isSelected())
                {
                    mSubConnectorList[i]->saveToDomElement(*copyRoot);
                }
            }
        }
    }

    // Copy widgets
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
    mpUndoStack->newPost(UNDO_PASTE);
    mpModelWidget->hasChanged();

    QDomElement *copyRoot;
    if(xmlStack == 0)
    {
        copyRoot = gpCopyStack->getCopyRoot();
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
        ModelObject *pObj = loadModelObject(objectElement, this);
        if (pObj)
        {
            //Apply offset to pasted object
            QPointF oldPos = pObj->pos();
            pObj->moveBy(xOffset, yOffset);
            mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());

            renameMap.insert(objectElement.attribute(HMF_NAMETAG), pObj->getName());
            //objectElement.setAttribute("name", renameMap.find(objectElement.attribute(HMF_NAMETAG)).value());
            objectElement = objectElement.nextSiblingElement("component");
        }
    }

        // Paste subsystems
    //! @todo maybe this subsystem loop can be merged with components above somehow. Basically the same code is used now after some cleanup, That way we could  have one loop for guimodelobjects, one for connector and after some cleanup one for widgets
    QDomElement systemElement = copyRoot->firstChildElement(HMF_SYSTEMTAG);
    while (!systemElement.isNull())
    {
        ModelObject* pObj = loadModelObject(systemElement, this, Undo);
        if (pObj)
        {
            renameMap.insert(systemElement.attribute(HMF_NAMETAG), pObj->getName());

            //Apply offset to pasted object
            QPointF oldPos = pObj->pos();
            pObj->moveBy(xOffset, yOffset);
            mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());
        }
        systemElement = systemElement.nextSiblingElement(HMF_SYSTEMTAG);
    }

        // Paste container ports
    QDomElement systemPortElement = copyRoot->firstChildElement(HMF_SYSTEMPORTTAG);
    while (!systemPortElement.isNull())
    {
        ModelObject* pObj = loadContainerPortObject(systemPortElement, this, Undo);
        if (pObj)
        {
            renameMap.insert(systemPortElement.attribute(HMF_NAMETAG), pObj->getName());

            //Apply offset to pasted object
            QPointF oldPos = pObj->pos();
            pObj->moveBy(xOffset, yOffset);
            mpUndoStack->registerMovedObject(oldPos, pObj->pos(), pObj->getName());
        }
        systemPortElement = systemPortElement.nextSiblingElement(HMF_SYSTEMPORTTAG);
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
        if (pWidget)
        {
            pWidget->setSelected(true);
            pWidget->moveBy(xOffset, yOffset);
            mpUndoStack->registerAddedWidget(pWidget);
        }
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
        mpUndoStack->newPost(UNDO_ALIGNX);
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
void ContainerObject::alignY()
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
        mpUndoStack->newPost(UNDO_ALIGNY);
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
void ContainerObject::distributeX()
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

        mpUndoStack->newPost(UNDO_DISTRIBUTEX);
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
void ContainerObject::distributeY()
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

        mpUndoStack->newPost(UNDO_DISTRIBUTEY);
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



void ContainerObject::replaceComponent(QString name, QString newType)
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

    deleteModelObject(name);

    qDebug() << "Name = " << name;

    ModelObject *newObj = addModelObject(newType, pos, rot);

    if(!newObj)
    {
        return; // Should never happen due to check above, but keep it just in case
    }

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
    mpUndoStack->newPost(UNDO_HIDEALLNAMES);
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! @brief Shows all component names.
//! @see hideNames()
void ContainerObject::showNames()
{
    mpUndoStack->newPost(UNDO_SHOWALLNAMES);
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
    mShowSubComponentNames = value;
}


void ContainerObject::toggleSignals(bool value)
{
    mSignalsHidden = !value;
    emit showOrHideSignals(value);
}

//! @brief Slot that sets hide ports flag to true or false
void ContainerObject::showSubcomponentPorts(bool doShowThem)
{
    mShowSubComponentPorts = doShowThem;
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

bool ContainerObject::setVariableAlias(const QString &rFullName, const QString &rAlias)
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

QString ContainerObject::getVariableAlias(const QString &rFullName)
{
    QString compName, portName, varName;
    QStringList dummy;
    splitFullVariableName(rFullName, dummy, compName, portName, varName);
    return getCoreSystemAccessPtr()->getVariableAlias(compName, portName, varName);
}

QString ContainerObject::getFullNameFromAlias(const QString alias)
{
    QString comp, port, var;
    getCoreSystemAccessPtr()->getFullVariableNameByAlias(alias, comp, port, var);
    return makeFullVariableName(getParentSystemNameHieararchy(), comp,port,var);
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
    Q_ASSERT(false);
    return 0;
}

void ContainerObject::setNumberOfLogSamples(size_t nSamples)
{
    Q_UNUSED(nSamples)
    //Needs to be overloaded
    Q_ASSERT(false);
}

double ContainerObject::getLogStartTime() const
{
    //Needs to be overloaded
    Q_ASSERT(false);
    return 0;
}

void ContainerObject::setLogStartTime(const double logStartT)
{
    Q_UNUSED(logStartT)
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
const QFileInfo &ContainerObject::getModelFileInfo() const
{
    return mModelFileInfo;
}

//! @brief Returns the file path to the model that this container belongs to
//! @details Will ask the parent if the container is an embedded container else returns the path to the external system model
QString ContainerObject::getModelFilePath() const
{
    if (mModelFileInfo.isFile())
    {
        return mModelFileInfo.canonicalFilePath();
    }
    else if (mpParentContainerObject)
    {
        return mpParentContainerObject->getModelFilePath();
    }
    else
    {
        return "";
    }
}

//! @brief Returns the path to the directory where the model that this container belongs to resides
//! @details Will ask the parent if the container is an embedded container else returns the path to the external system model
QString ContainerObject::getModelPath() const
{
    QFileInfo fi(getModelFilePath());
    return fi.absolutePath();
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
    return mWidgetMap.values();
}

Widget *ContainerObject::getWidget(const int id)
{
    return mWidgetMap.value(id, 0);
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
//! use only when transferring ownership of objects to an other container
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


//! @brief Removes one line from connector being created.
//! @param pos Position to redraw connector to after removing the line
void ContainerObject::removeOneConnectorLine(QPointF pos)
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
            if(gpModelHandler->getCurrentViewContainerObject() == this)      //Only modify main window actions if this is current container
            {
                gpMainWindow->mpUndoAction->setDisabled(true);
                gpMainWindow->mpRedoAction->setDisabled(true);
            }
        }
    }
    else
    {
        mUndoDisabled = false;
        if(gpModelHandler->getCurrentViewContainerObject() == this)      //Only modify main window actions if this is current container
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
bool ContainerObject::areSubComponentPortsShown()
{
    return mShowSubComponentPorts;
}


//! @brief Tells whether or not object names in container are hidden
bool ContainerObject::areSubComponentNamesShown()
{
    return mShowSubComponentNames;
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
    //gpMainWindow->mpShowLossesAction->setDisabled(mpLogDataHandler->isEmpty());
    //gpMainWindow->mpAnimateAction->setDisabled(mpNewPlotData->isEmpty());

    gpMainWindow->mpToggleNamesAction->setChecked(mShowSubComponentNames);
    gpMainWindow->mpTogglePortsAction->setChecked(mShowSubComponentPorts);
    gpMainWindow->mpShowLossesAction->setChecked(mLossesVisible);
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


////! @brief Defines the right click menu for container objects.
////! @todo Maybe should try to reduce multiple copies of same functions with other GUIObjects
//void ContainerObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
//{
//    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
//    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
//        return;

//    QMenu menu;
//    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
//    QAction *saveAction = menu.addAction(tr("Save Subsystem As"));
//    QAction *saveAsComponentAction = menu.addAction(tr("Save As Component"));
//    if(!mModelFileInfo.filePath().isEmpty())
//    {
//        loadAction->setDisabled(true);
//    }
//    if(isExternal())
//    {
//        saveAction->setDisabled(true);
//        saveAsComponentAction->setDisabled(true);
//    }

//    //qDebug() << "ContainerObject::contextMenuEvent";
//    QAction *pAction = this->buildBaseContextMenu(menu, event);
//    if (pAction == loadAction)
//    {
//        QDir fileDialog; QFile file;
//        QString modelFilePath = QFileDialog::getOpenFileName(gpMainWindow, tr("Choose Subsystem File"),
//                                                             gpConfig->getStringSetting(CFG_SUBSYSTEMDIR),
//                                                             tr("Hopsan Model Files (*.hmf)"));
//        if (!modelFilePath.isNull())
//        {
//            file.setFileName(modelFilePath);
//            QFileInfo fileInfo(file);
//            gpConfig->setStringSetting(CFG_SUBSYSTEMDIR, fileInfo.absolutePath());

//            bool doIt = true;
//            if (mModelObjectMap.size() > 0)
//            {
//                QMessageBox clearAndLoadQuestionBox(QMessageBox::Warning, tr("Warning"),tr("All current contents of the system will be replaced. Do you want to continue?"), 0, 0);
//                clearAndLoadQuestionBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
//                clearAndLoadQuestionBox.addButton(tr("&No"), QMessageBox::RejectRole);
//                clearAndLoadQuestionBox.setWindowIcon(gpMainWindow->windowIcon());
//                doIt = (clearAndLoadQuestionBox.exec() == QMessageBox::AcceptRole);
//            }

//            if (doIt)
//            {
//                this->clearContents();

//                QDomDocument domDocument;
//                QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
//                if (!hmfRoot.isNull())
//                {
//                    //! @todo Check version numbers
//                    //! @todo check if we could load else give error message and don't attempt to load
//                    QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
//                    this->setModelFileInfo(file); //Remember info about the file from which the data was loaded
//                    QFileInfo fileInfo(file);
//                    this->setAppearanceDataBasePath(fileInfo.absolutePath());
//                    this->loadFromDomElement(systemElement);
//                }
//            }
//        }
//    }
//    else if(pAction == saveAction)
//    {
//        //Get file name
//        QString modelFilePath;
//        modelFilePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Save Subsystem As"),
//                                                     gpConfig->getStringSetting(CFG_LOADMODELDIR),
//                                                     gpMainWindow->tr("Hopsan Model Files (*.hmf)"));

//        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
//        {
//            return;
//        }


//        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
//        QFile file(modelFilePath);   //Create a QFile object
//        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//        {
//            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
//            return;
//        }

//        //Save xml document
//        QDomDocument domDocument;
//        QDomElement rootElement;
//        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

//        // Save the required external lib names
//        QVector<QString> extLibNames;
//        CoreLibraryAccess coreLibAccess;
//        coreLibAccess.getLoadedLibNames(extLibNames);

//        QDomElement reqDom = appendDomElement(rootElement, "requirements");
//        for (int i=0; i<extLibNames.size(); ++i)
//        {
//            appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
//        }

//        //Save the model component hierarchy
//        this->saveToDomElement(rootElement, FullModel);

//        //Save to file
//        QFile xmlFile(modelFilePath);
//        if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//        {
//            gpMessageHandler->addErrorMessage("Could not save to file: " + modelFilePath);
//            return;
//        }
//        QTextStream out(&xmlFile);
//        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//        domDocument.save(out, XMLINDENTATION);

//        //Close the file
//        xmlFile.close();

//       // mpModelWidget->saveTo(modelFilePath, FullModel);
//    }
//    else if(pAction == saveAsComponentAction)
//    {
//        //Get file name
//        QString cafFilePath;
//        cafFilePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Save Subsystem As"),
//                                                     gpConfig->getStringSetting(CFG_LOADMODELDIR),
//                                                     gpMainWindow->tr("Hopsan Component Appearance Files (*.xml)"));

//        if(cafFilePath.isEmpty())     //Don't save anything if user presses cancel
//        {
//            return;
//        }

//        QString iconFileName = QFileInfo(getIconPath(UserGraphics, Absolute)).fileName();
//        QString modelFileName = QFileInfo(cafFilePath).baseName()+".hmf";

//        //! @todo wahy is graphics copied twice
//        QFile::copy(getIconPath(UserGraphics, Absolute), QFileInfo(cafFilePath).path()+"/"+iconFileName);
//        QFile::copy(getIconPath(UserGraphics, Absolute), getAppearanceData()->getBasePath()+"/"+iconFileName);

//        bool ok;
//        QString subtype = QInputDialog::getText(gpMainWindowWidget, tr("Decide a unique Subtype"),
//                                                tr("Decide a unique subtype name for this component:"), QLineEdit::Normal,
//                                                QString(""), &ok);
//        if (!ok || subtype.isEmpty())
//        {
//            gpMessageHandler->addErrorMessage("You must specify a subtype name. Aborting!");
//            return;
//        }

//        //! @todo it would be better if this xml would only include hmffile attribute and all otehr info loaded from there
//        QString cafStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
//        cafStr.append(QString("<hopsanobjectappearance version=\"0.3\">\n"));
//        cafStr.append(QString("    <modelobject hmffile=\"%1\" displayname=\"%2\" typename=\"%3\" subtypename=\"%4\">\n").arg(modelFileName).arg(getName()).arg("Subsystem").arg(subtype));
//        cafStr.append(QString("        <icons>\n"));
//        cafStr.append(QString("            <icon scale=\"1\" path=\"%1\" iconrotation=\"ON\" type=\"user\"/>\n").arg(iconFileName));
//        cafStr.append(QString("        </icons>\n"));
//        cafStr.append(QString("    </modelobject>\n"));
//        cafStr.append(QString("</hopsanobjectappearance>\n"));

//        QFile cafFile(cafFilePath);
//        if(!cafFile.open(QFile::Text | QFile::WriteOnly))
//        {
//            gpMessageHandler->addErrorMessage("Could not open the file: "+cafFile.fileName()+" for writing.");
//            return;
//        }
//        cafFile.write(cafStr.toUtf8());
//        cafFile.close();

//        QString modelFilePath = QFileInfo(cafFilePath).path()+"/"+QFileInfo(cafFilePath).baseName()+".hmf";

//        QString orgIconPath = this->getIconPath(UserGraphics, Relative);
//        this->setIconPath(iconFileName, UserGraphics, Relative);

//        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
//        QFile file(modelFilePath);   //Create a QFile object
//        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//        {
//            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
//            return;
//        }

//        //Save xml document
//        QDomDocument domDocument;
//        QDomElement rootElement;
//        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

//        // Save the required external lib names
//        QVector<QString> extLibNames;
//        CoreLibraryAccess coreLibAccess;
//        coreLibAccess.getLoadedLibNames(extLibNames);

//        QDomElement reqDom = appendDomElement(rootElement, "requirements");
//        for (int i=0; i<extLibNames.size(); ++i)
//        {
//            appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
//        }

//        //Save the model component hierarchy
//        QString old_subtype = this->getAppearanceData()->getSubTypeName();
//        this->getAppearanceData()->setSubTypeName(subtype);
//        this->saveToDomElement(rootElement, FullModel);
//        this->getAppearanceData()->setSubTypeName(old_subtype);

//        //Save to file
//        QFile xmlFile(modelFilePath);
//        if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//        {
//            gpMessageHandler->addErrorMessage("Could not save to file: " + modelFilePath);
//            return;
//        }
//        QTextStream out(&xmlFile);
//        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//        domDocument.save(out, XMLINDENTATION);

//        //Close the file
//        xmlFile.close();

//        this->setIconPath(orgIconPath, UserGraphics, Relative);

//        QFile::remove(getModelFilePath()+"/"+iconFileName);
//    }

//    //Don't call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
//    //GUIModelObject::contextMenuEvent(event);
//    ////QGraphicsItem::contextMenuEvent(event);
//}


////! @brief Defines the double click event for container objects (used to enter containers).
//void ContainerObject::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
//{
//    ModelObject::mouseDoubleClickEvent(event);
//    this->enterContainer();
//}


////! @brief Opens the properties dialog for container objects.
//void ContainerObject::openPropertiesDialog()
//{
//    //Do Nothing
//}


//! @brief Clears all of the contained objects (and deletes them).
//! This code cant be run in the destructor as this wold cause wired behaviour in the derived system class.
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
        (*mit)->setIsLocked(false);     //Must unlock object in order to remove it
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


//! @brief Enters a container object and makes the view represent it contents.
void ContainerObject::enterContainer()
{
    // First deselect everything so that buttons pressed in the view are not sent to objects in the previous container
    mpParentContainerObject->deselectAll(); //deselect myself and anyone else

    // Remember current viewport, before we switch to the new one
    mpParentContainerObject->setGraphicsViewport(mpModelWidget->getGraphicsView()->getViewPort());

    // Show this scene
    mpModelWidget->getGraphicsView()->setContainerPtr(this);
    mpModelWidget->getQuickNavigationWidget()->addOpenContainer(this);
    mpModelWidget->getGraphicsView()->setViewPort(getGraphicsViewport());

    // Disconnect parent system and connect new system with actions
    mpParentContainerObject->unmakeMainWindowConnectionsAndRefresh();
    this->makeMainWindowConnectionsAndRefresh();

    refreshInternalContainerPortGraphics();

    mpModelWidget->setExternalSystem((this->isExternal() && this != mpModelWidget->getTopLevelSystemContainer()) ||
                                     this->isAncestorOfExternalSubsystem());
}

//! @brief Exit a container object and make its the view represent its parents contents.
void ContainerObject::exitContainer()
{
    this->deselectAll();

    // Remember current viewport, before we set parents
    this->setGraphicsViewport(mpModelWidget->getGraphicsView()->getViewPort());

    // Go back to parent system
    mpModelWidget->getGraphicsView()->setContainerPtr(mpParentContainerObject);
    mpModelWidget->getGraphicsView()->setViewPort(mpParentContainerObject->getGraphicsViewport());

    mpModelWidget->setExternalSystem((mpParentContainerObject->isExternal() && mpParentContainerObject != mpModelWidget->getTopLevelSystemContainer()) ||
                                     mpParentContainerObject->isAncestorOfExternalSubsystem());

    // Disconnect this system and connect parent system with undo and redo actions
    this->unmakeMainWindowConnectionsAndRefresh();
    mpParentContainerObject->makeMainWindowConnectionsAndRefresh();

    // Refresh external port appearance
    //! @todo We only need to do this if ports have change, right now we always refresh, don't know if this is a big deal
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

    mpLossesDialog->setPalette(gpConfig->getPalette());

    mpLossesDialog->show();

    connect(pCancelButton, SIGNAL(clicked()), mpLossesDialog, SLOT(close()));
    connect(pNextButton, SIGNAL(clicked()), this, SLOT(showLossesFromDialog()));
    connect(pHelpAction, SIGNAL(triggered()), gpHelpPopupWidget, SLOT(openContextHelp()));
}


void ContainerObject::showLossesFromDialog()
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

    PlotWindow *pWindow = gpPlotHandler->createNewUniquePlotWindow("Energy Losses");
    pWindow->addPlotTab("Energy Losses", BarchartPlotType)->addBarChart(pItemModel);
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

    QDialog *pDialog = new QDialog(gpMainWindowWidget);
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

void ContainerObject::exportMesasuredSimulationTime()
{
    //! @todo Ask for filename
    QString pathStr = QFileDialog::getSaveFileName(gpMainWindowWidget, "Save measured simulation times", gpConfig->getStringSetting(CFG_PLOTDATADIR), "*.csv");

    if(pathStr.isEmpty())
        return; //User aborted

    gpConfig->setStringSetting(CFG_PLOTDATADIR, QFileInfo(pathStr).absolutePath());


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


//! @brief Returns a list with pointers to all sub-connectors in container
QList<Connector *> ContainerObject::getSubConnectorPtrs()
{
    return mSubConnectorList;
}



LogDataHandler2 *ContainerObject::getLogDataHandler()
{
    return mpModelWidget->getLogDataHandler();
    //return mpLogDataHandler;
}
