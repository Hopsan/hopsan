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
//! @file   GUIModelObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#include "GUIModelObject.h"

#include "GUISystem.h"
#include "Widgets/ProjectTabWidget.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/LibraryWidget.h"
#include "UndoStack.h"
#include "Configuration.h"
#include "MainWindow.h"

//! @brief Constructor for GUI Objects
//! @param position Initial scene coordinates where object shall be placed
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to appearance data object
//! @param startSelected Initial selection status
//! @param gfxType Initial graphics type (user or iso)
//! @param system Pointer to the parent system
//! @param parent Pointer to parent object (not mandatory)
ModelObject::ModelObject(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, selectionStatus startSelected, graphicsType gfxType, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, startSelected, pParentContainer, pParent)
{
        //Initialize variables
    mName="no_name_set_yet";
    mpIcon = 0;
    mpNameText = 0;
    mTextOffset = 5.0;
    mDragCopying = false;

        //Set the hmf save tag name
    mHmfTagName = HMF_OBJECTTAG; //!< @todo change this

        //Make a local copy of the appearance data (that can safely be modified if needed)
    if (pAppearanceData != 0)
    {
        mModelObjectAppearance = *pAppearanceData;
        mName = mModelObjectAppearance.getDisplayName(); //Default name to the appearance data display name
    }

        //Setup appearance
    this->setIcon(gfxType);
    this->refreshAppearance();
    this->setCenterPos(position);
    this->setZValue(MODELOBJECT_Z);
    this->setSelected(startSelected);

        //Create the textbox containing the name
    mpNameText = new ModelObjectDisplayName(this);
    mpNameText->setFlag(QGraphicsItem::ItemIsSelectable, false); //To minimize problems when move after copy and so on
    if(this->mpParentContainerObject != 0)
    {
        this->setNameTextScale(mpParentContainerObject->mpParentProjectTab->mpGraphicsView->getZoomFactor());
    }
    this->setNameTextPos(0); //Set initial name text position
    if(pParentContainer != 0 && pParentContainer->areSubComponentNamesHidden())
    {
        this->hideName(NOUNDO);
    }
    else
    {
        this->showName(NOUNDO);
    }

        //Create connections
    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(snapNameTextPosition(QPointF)));
    if(mpParentContainerObject != 0)
    {
        connect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(zoomChange(qreal)), this, SLOT(setNameTextScale(qreal)));
//        connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
        connect(mpParentContainerObject, SIGNAL(hideAllNameText()), this, SLOT(hideName()));
        connect(mpParentContainerObject, SIGNAL(showAllNameText()), this, SLOT(showName()));
        connect(mpParentContainerObject, SIGNAL(setAllGfxType(graphicsType)), this, SLOT(setIcon(graphicsType)));
    }
    else
    {
        //maybe something different
    }

    mpLossesDisplay = new QGraphicsTextItem(this);
    mpLossesDisplay->setFlags(QGraphicsItem::ItemIgnoresTransformations);
    mpLossesDisplay->setVisible(false);
}


//! @brief Destructor for GUI Objects
ModelObject::~ModelObject()
{
    emit objectDeleted();
}

void ModelObject::setParentContainerObject(ContainerObject *pParentContainer)
{
    WorkspaceObject::setParentContainerObject(pParentContainer);

    //Refresh the port signals and slots connections
    for (int i=0; i<mPortListPtrs.size(); ++i)
    {
        //mPortListPtrs[i]->refreshParentContainerSigSlotConnections();
    }
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int ModelObject::type() const
{
    return Type;
}


void ModelObject::getLosses(double &total, double &hydraulic, double &mechanic)
{
    total = mTotalLosses;
    hydraulic = mHydraulicLosses;
    mechanic = mMechanicLosses;
}


//! @brief Updates name text position
//! @param pos Position where name text was dropped
void ModelObject::snapNameTextPosition(QPointF pos)
{
    QVector<QPointF> pts;
    this->calcNameTextPositions(pts);

    QPointF  mtp_pos = mpNameText->mapToParent(pos);
    if ( dist(mtp_pos, pts[0]) < dist(mtp_pos, pts[1]) )
    {
        //We dont use this.setnamepos here as that would recaluclate the positions again
        mpNameText->setPos(pts[0]);
        mNameTextPos = 0;
    }
    else
    {
        //We dont use this.setnamepos here as that would recaluclate the positions again
        mpNameText->setPos(pts[1]);
        mNameTextPos = 1;
    }

    if(mpParentContainerObject != 0)
    {
        mpParentContainerObject->mpParentProjectTab->getGraphicsView()->updateViewPort();
    }
}

void ModelObject::calcNameTextPositions(QVector<QPointF> &rPts)
{
    rPts.clear();

    QPointF pt0, pt1, tWH;
    QPointF localCenter = this->boundingRect().center();
    QPointF localWH = this->boundingRect().bottomRight();

    //Create the transformation, and transform points
    QTransform transf;
    transf.rotate(-(this->rotation()));
    if (this->mIsFlipped)
    {
        transf.scale(-1.0,1.0);
    }

    //First we transform the height and width, (also take absolute values for width and height)
    tWH = transf*localWH;
    tWH.setX(fabs(tWH.x()));
    tWH.setY(fabs(tWH.y()));

    //qDebug() <<  " width: " << this->boundingRect().width() << "height: " << this->boundingRect().height()  << " lWH: " << localWH << " tWH: " << tWH;
    //Now we transforme the name text posistions
    //pt0 = top, pt1 = bottom, pts relative loacal center on object
    pt0.rx() = -mpNameText->boundingRect().width()/2.0;
    pt0.ry() = -(tWH.y()/2.0 + mpNameText->boundingRect().height() + mTextOffset);

    pt1.rx() = -mpNameText->boundingRect().width()/2.0;
    pt1.ry() = tWH.y()/2.0 + mTextOffset;

    //    qDebug() << "pt0: " << pt0;
    //    qDebug() << "pt1: " << pt1;
    pt0 = transf * pt0;
    pt1 =  transf * pt1;
    //    qDebug() << "tpt0: " << pt0;
    //    qDebug() << "tpt1: " << pt1;

    //Store transformed positions relative current local origo
    rPts.append(localCenter+pt0);
    rPts.append(localCenter+pt1);

//    qDebug() << "rPts0: " << rPts[0];
//    qDebug() << "rPts1: " << rPts[1];
//    qDebug() << "\n";
}


//! @brief Slot that scales the name text
void ModelObject::setNameTextScale(qreal scale)
{
    this->mpNameText->setScale(scale);
}


//! @brief Stores a connector pointer in the connector list
//! @param item Pointer to connector that shall be stored
void ModelObject::rememberConnector(Connector *item)
{
    //Only append if new connector, prevents double registration if we connect to ourselves
    if ( !mConnectorPtrs.contains(item) )
    {
        mConnectorPtrs.append(item);
        connect(this, SIGNAL(objectMoved()), item, SLOT(drawConnector()));
    }
}


//! @brief Removes a connector pointer from the connector list
//! @param item Pointer to connector that shall be forgotten
void ModelObject::forgetConnector(Connector *item)
{
    mConnectorPtrs.removeAll(item);
    disconnect(this, SIGNAL(objectMoved()), item, SLOT(drawConnector()));
}

//! @param Returns a copy of the list with pointers to the connecetors connected to the object
QList<Connector*> ModelObject::getConnectorPtrs()
{
    return mConnectorPtrs;
}


//! @brief Refreshes the displayed name from the actual name
//! @param [in] overrideName If this is non empty the name of the component in the gui will be forced to a specific value. DONT USE THIS UNLESS YOU HAVE TO
void ModelObject::refreshDisplayName(const QString overrideName)
{
    QString oldName = mModelObjectAppearance.getDisplayName();

    if (!overrideName.isEmpty())
    {
        mName = overrideName;
    }

    mModelObjectAppearance.setDisplayName(mName);
    if (mpNameText != 0)
    {
        mpNameText->setPlainText(mName);
        mpNameText->setSelected(false);
        //Adjust the position of the text
        this->snapNameTextPosition(mpNameText->pos());
    }

    if (oldName != mName)
    {
        emit nameChanged();
    }
}


//! @brief Returns the name of the object
QString ModelObject::getName()
{
    return mName;
}

void ModelObject::setName(QString /*name*/)
{
    //Must be overloaded
    assert(false);
}



//! @brief Returns a list with pointers to the ports in the object
QList<Port*> &ModelObject::getPortListPtrs()
{
    return mPortListPtrs;
}


//! @brief Updates the icon of the object to user or iso style
//! @param gfxType Graphics type that shall be used
void ModelObject::setIcon(graphicsType gfxType)
{
    QString iconPath;
    qreal iconScale;
    if ( (gfxType == ISOGRAPHICS) && mModelObjectAppearance.hasIcon(ISOGRAPHICS) )
    {
        iconPath = mModelObjectAppearance.getFullAvailableIconPath(ISOGRAPHICS);
        iconScale = mModelObjectAppearance.getIconScale(ISOGRAPHICS);
        mIconType = ISOGRAPHICS;
    }
    else
    {
        iconPath = mModelObjectAppearance.getFullAvailableIconPath(USERGRAPHICS);
        iconScale = mModelObjectAppearance.getIconScale(USERGRAPHICS);
        mIconType = USERGRAPHICS;
    }

    //Avoid swappping icon if same as before, we swap also if scale changes
    if  ( (mLastIconPath != iconPath) || !fuzzyEqual(mLastIconScale, iconScale, 0.001) )
    {
        if (mpIcon != 0)
        {
            mpIcon->deleteLater(); //Shedule previous icon for deletion
            disconnect(this->getParentContainerObject()->mpParentProjectTab->getGraphicsView(), SIGNAL(zoomChange(qreal)), this, SLOT(setIconZoom(qreal)));
        }

        mpIcon = new QGraphicsSvgItem(iconPath, this);
        mpIcon->setFlags(QGraphicsItem::ItemStacksBehindParent);
        mpIcon->setScale(iconScale);

        this->prepareGeometryChange();
        this->resize(mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale);  //Resize modelobject
        mpSelectionBox->setSize(0.0, 0.0, mpIcon->boundingRect().width()*iconScale, mpIcon->boundingRect().height()*iconScale); //Resize selection box

        this->setTransformOriginPoint(this->boundingRect().center());

        if(mModelObjectAppearance.getIconRotationBehaviour(mIconType) == "ON")
        {
            mIconRotation = true;
            mpIcon->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
        }
        else
        {
            mIconRotation = false;
            mpIcon->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            if (this->getParentContainerObject() != 0)
            {
                mpIcon->setScale(this->getParentContainerObject()->mpParentProjectTab->getGraphicsView()->getZoomFactor()*iconScale);
                connect(this->getParentContainerObject()->mpParentProjectTab->getGraphicsView(), SIGNAL(zoomChange(qreal)), this, SLOT(setIconZoom(qreal)), Qt::UniqueConnection);
            }
            //! @todo we need to disconnect this also at some point, when swapping between systems or groups
        }

        mLastIconPath = iconPath;
        mLastIconScale = iconScale;
    }
}

void ModelObject::refreshIconPosition()
{
    //Only move when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setPos( this->mapFromScene(this->getCenterPos() - mpIcon->boundingRect().center() ));
        qDebug() << "Setting positon to: " << mpIcon->pos();
    }
}

void ModelObject::setIconZoom(const qreal zoom)
{
    //Only scale when we have disconnected the icon from transformations
    if (!mIconRotation)
    {
        mpIcon->setScale(mModelObjectAppearance.getIconScale(mIconType)*zoom);
    }
}


void ModelObject::showLosses()
{
    QTime time;

    mTotalLosses = 0.0;
    mHydraulicLosses = 0.0;
    mMechanicLosses = 0.0;

    if(getTypeCQS() == "S")
        return;

    int generation = mpParentContainerObject->getNumberOfPlotGenerations()-1;

    time.start();

    for(int p=0; p<mPortListPtrs.size(); ++p)
    {
        QString portType = mPortListPtrs[p]->getPortType();
        if(portType == "SYSTEMPORT")
        {
            portType = mPortListPtrs[p]->getPortType(CoreSystemAccess::INTERNALPORTTYPE);
        }
        if(mPortListPtrs[p]->getNodeType() == "NodeHydraulic" && portType != "READPORT")
        {
            //Power port, so we must cycle all connected ports and ask for their data
            if(mPortListPtrs[p]->getPortType() == "POWERMULTIPORT" || mPortListPtrs[p]->getPortType() == "SIGNALMULTIPORT")
            {
                QVector<Port *> vConnectedPorts = mPortListPtrs[p]->getConnectedPorts();
                for(int i=0; i<vConnectedPorts.size(); ++i)
                {
                    if(vConnectedPorts.at(i)->getPortType() == "READPORT")
                    {
                        continue;
                    }
                    QString componentName = vConnectedPorts.at(i)->mpParentGuiModelObject->getName();
                    QString portName = vConnectedPorts.at(i)->getPortName();
                    QVector<double> vPressure = mpParentContainerObject->getPlotData(generation, componentName, portName, "Pressure");
                    QVector<double> vFlow = mpParentContainerObject->getPlotData(generation, componentName, portName, "Flow");
                    for(int s=0; s<vPressure.size()-1; ++s) //Minus one because of integration method
                    {
                        mTotalLosses += vPressure.at(s) * vFlow.at(s) * (mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s+1)-mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s));
                        mHydraulicLosses += vPressure.at(s) * vFlow.at(s) * (mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s+1)-mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s));
                    }
                }
            }
            else    //Normal port!
            {
                QVector<double> vPressure = mpParentContainerObject->getPlotData(generation, getName(), mPortListPtrs[p]->getPortName(), "Pressure");
                QVector<double> vFlow = mpParentContainerObject->getPlotData(generation, getName(), mPortListPtrs[p]->getPortName(), "Flow");

                for(int s=0; s<vPressure.size()-1; ++s) //Minus one because of integration method
                {
                    mTotalLosses += vPressure.at(s) * vFlow.at(s) *
                                    (mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s+1) -
                                     mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s));
                    mHydraulicLosses += vPressure.at(s) * vFlow.at(s) *
                                        (mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s+1) -
                                         mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s));
                }
            }
        }
        else if(mPortListPtrs[p]->getNodeType() == "NodeMechanic")
        {
            //Power port, so we must cycle all connected ports and ask for their data
            if(mPortListPtrs[p]->getPortType() == "POWERMULTIPORT" || mPortListPtrs[p]->getPortType() == "SIGNALMULTIPORT")
            {
                QVector<Port *> vConnectedPorts = mPortListPtrs[p]->getConnectedPorts();
                for(int i=0; i<vConnectedPorts.size(); ++i)
                {
                    QString componentName = vConnectedPorts.at(i)->mpParentGuiModelObject->getName();
                    QString portName = vConnectedPorts.at(i)->getPortName();
                    QVector<double> vForce = mpParentContainerObject->getPlotData(generation, componentName, portName, "Force");
                    QVector<double> vVelocity = mpParentContainerObject->getPlotData(generation, componentName, portName, "Velocity");
                    for(int s=0; s<vForce.size()-1; ++s) //Minus one because of integration method
                    {
                        mTotalLosses += vForce.at(s) * vVelocity.at(s) *
                                        (mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s+1) -
                                         mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s));
                        mMechanicLosses += vForce.at(s) * vVelocity.at(s) *
                                           (mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s+1) -
                                            mpParentContainerObject->getTimeVector(generation, componentName, portName).at(s));
                    }
                }
            }
            else    //Normal port!
            {
                QVector<double> vForce = mpParentContainerObject->getPlotData(generation, getName(), mPortListPtrs[p]->getPortName(), "Force");
                QVector<double> vVelocity = mpParentContainerObject->getPlotData(generation, getName(), mPortListPtrs[p]->getPortName(), "Velocity");
                for(int s=0; s<vForce.size()-1; ++s)
                {
                    mTotalLosses += vForce.at(s) * vVelocity.at(s) *
                                    (mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s+1) -
                                     mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s));
                    mMechanicLosses += vForce.at(s) * vVelocity.at(s) *
                                       (mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s+1) -
                                        mpParentContainerObject->getTimeVector(generation, getName(), mPortListPtrs[p]->getPortName()).at(s));
                }
            }
        }
        else
        {
            //Do something else?!
        }
    }

    if(mTotalLosses != 0)
    {
        if(getTypeCQS() == "Q")     //Invert losses for Q components (because positive direction is defined as outwards for Q and inwards for C)
        {
            mTotalLosses *= -1;
            mHydraulicLosses *= -1;
            mMechanicLosses *= -1;
        }
        QString totalString;
        totalString.setNum(mTotalLosses);
        QString totalAddedString;
        totalAddedString.setNum(-mTotalLosses);
        QString hydraulicString;
        hydraulicString.setNum(mHydraulicLosses);
        QString hydraulicAddedString;
        hydraulicAddedString.setNum(-mHydraulicLosses);
        QString mechanicString;
        mechanicString.setNum(mMechanicLosses);
        QString mechanicAddedString;
        mechanicAddedString.setNum(-mMechanicLosses);

        QString label;
        if(mTotalLosses > 0)
        {
            label = "<p><span style=\"background-color:lightyellow; color:red\"><b>&#160;&#160;Total losses: " + totalString + " J&#160;&#160;</b>";
        }
        else
        {
            label = "<p><span style=\"background-color:lightyellow; color:green\">&#160;&#160;Added energy: <b>" + totalAddedString + " J</b>&#160;&#160;";
        }
        if(mHydraulicLosses > 0 && mHydraulicLosses != mTotalLosses)
        {
            label.append("<br>&#160;&#160;Hydraulic losses: <b>" + hydraulicString + " J</b>&#160;&#160;");
        }
        else if(mHydraulicLosses < 0 && mHydraulicLosses != mTotalLosses)
        {
            label.append("<br><font color=\"green\">&#160;&#160;Added hydraulic energy: <b>" + hydraulicAddedString + " J</b>&#160;&#160;</font>");
        }
        if(mMechanicLosses > 0 && mMechanicLosses != mTotalLosses)
        {
            label.append("<br>&#160;&#160;Mechanic losses: <b>" + mechanicString + " J</b>&#160;&#160;");
        }
        else if(mMechanicLosses < 0 && mMechanicLosses != mTotalLosses)
        {
            label.append("<br><font color=\"green\">&#160;&#160;Added mechanic energy: <b>" + mechanicAddedString + " J</b>&#160;&#160;</font>");
        }
        label.append("</span></p>");
        mpLossesDisplay->setHtml(label);

        QPointF pt;
        QPointF localCenter = this->boundingRect().center();
        QTransform transf;
        transf.rotate(-(this->rotation()));
        if (this->mIsFlipped)
            transf.scale(-1.0,1.0);
        pt.rx() = -mpLossesDisplay->boundingRect().width()/2.0;
        pt.ry() = -mpLossesDisplay->boundingRect().height()/2.0;
        pt = transf*pt;
        mpLossesDisplay->setPos(localCenter + pt);
        mpLossesDisplay->setVisible(true);
        mpLossesDisplay->setZValue(LOSSESDISPLAY_Z);
    }
}


void ModelObject::hideLosses()
{
    mpLossesDisplay->setVisible(false);
}


void ModelObject::redrawConnectors()
{
    for(int i=0; i<mConnectorPtrs.size(); ++i)
    {
        mConnectorPtrs.at(i)->drawConnector();
    }
}


bool ModelObject::isLossesDisplayVisible()
{
    return mpLossesDisplay->isVisible();
}


//! @brief Returns a pointer to the port with the specified name
Port *ModelObject::getPort(QString name)
{
    //qDebug() << "Trying to find GUIPort with name: " << name;
    //! @todo use a guiport map instead   (Is this really a good idea? The number of ports is probably too small to make it beneficial, and it would slow down everything else...)
    for (int i=0; i<mPortListPtrs.size(); ++i)
    {
        if (mPortListPtrs[i]->getPortName() == name)
        {
            return mPortListPtrs[i];
        }
        //qDebug() << mPortListPtrs[i]->getName() << " != " << name;
    }
    qDebug() << "Did NOT find GUIPort with name: " << name << " in: " << this->getName() << " returning NULL ptr";

    return 0;
}


Port *ModelObject::createRefreshExternalDynamicParameterPort(QString portName)
{
    mActiveDynamicParameterPortNames.append(portName);
    return createRefreshExternalPort(portName);
}

//! @brief This method creates ONE external port. Or refreshes existing ports. It assumes that port appearance information for this port exists
//! @param[portName] The name of the port to create
//! @todo maybe defualt create that info if it is missing
Port *ModelObject::createRefreshExternalPort(QString portName)
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
        QString nodeType = getParentContainerObject()->getCoreSystemAccessPtr()->getNodeType(this->getName(), it.key());
        QString portType = getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(this->getName(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        qreal x = it.value().x*boundingRect().width();
        qreal y = it.value().y*boundingRect().height();
        qDebug() << "x,y: " << x << " " << y;

        if (this->type() == GROUPCONTAINER)
        {
            pPort = new GroupPort(it.key(), x, y, &(it.value()), this);
        }
        else
        {
            pPort = new Port(it.key(), x, y, &(it.value()), this);
        }


        mPortListPtrs.append(pPort);

        pPort->refreshPortGraphics();
    }
    else
    {
        // The external port already seems to exist, lets update it incase something has changed
        //! @todo Maybe need to have a refresh portappearance function, dont really know if this will ever be used though, will fix when it becomes necessary
        pPort->refreshPortGraphics();

        // In this case connections exist, also refresh any attached connectors, if types have changed
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

//! @brief Removes an external Port from a container object
//! @param[in] portName The name of the port to be removed
//! @todo maybe we should use a map instead to make delete more efficient, (may not amtter usually not htat many external ports)
void ModelObject::removeExternalPort(QString portName)
{
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
    QList<Port*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getPortName() == portName )
        {
            //Delete the GUIPort its post in the portlist and its appearance data
            mActiveDynamicParameterPortNames.removeAll(portName);
            (*plit)->disconnectAndRemoveAllConnectedConnectors();
            (*plit)->deleteLater();
            mPortListPtrs.erase(plit);
            mModelObjectAppearance.erasePortAppearance(portName); // It is important that we remove the appearance data last, or some pointers will be dangling
            break;
        }
    }
    //qDebug() << "mPortListPtrs.size(): " << mPortListPtrs.size();
}

//! @brief Get the default value of a parameter
//! @param [in] paramName The name of the parameter
//! @returns QString with default value for parameter, or empty QString if paramName not found
QString ModelObject::getDefaultParameterValue(const QString paramName) const
{
    if(mDefaultParameterValues.contains(paramName))
    {
        return mDefaultParameterValues.find(paramName).value();
    }
    else
    {
        return QString();
    }
}


//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString ModelObject::getParameterValue(const QString paramName)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValue(this->getName(), paramName);
}

//! @brief Get a vector contain data from all parameters
//! @param [out] rParameterDataVec A vector that will contain parameter data
void ModelObject::getParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    mpParentContainerObject->getCoreSystemAccessPtr()->getParameters(this->getName(), rParameterDataVec);
}


//! @brief Get a vector with the names of the available parameters
QStringList ModelObject::getParameterNames()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterNames(this->getName());
}

//! @brief Get parameter data for a specific parameter
//! @param [out] rData The parameter data
void ModelObject::getParameter(const QString paramName, CoreParameterData &rData)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameter(this->getName(), paramName, rData);
}


//! @brief Virtual function that sets specified parameter to specified system parameter
//! @param name Name of parameter
//! @param valueTxt System parameter
bool ModelObject::setParameterValue(QString /*name*/, QString /*valueTxt*/, bool /*force*/)
{
    //cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
}


//! @brief Virtual function that returns the specified start value
//! @param portName Name of the port to return value from
//! @param variable Name of the parameter to return value from
QString ModelObject::getStartValueTxt(QString /*portName*/, QString /*variable*/)
{
    //cout << "This function should only be available in GUIComponent" << endl;
    assert(false);
    return "";
}

//! @brief Virtual function that sets specified startValue to specified value
//! @param portName Name of port
//! @param variable Name of variable in port
//! @param sysParName System parameter name
bool ModelObject::setStartValue(QString /*portName*/, QString /*variable*/, QString /*sysParName*/)
{
    //cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
    return false;
}


void ModelObject::setModelFileInfo(QFile &/*rFile*/)
{
    //Only available in GUISystem for now
    assert(false);
}


void ModelObject::saveToDomElement(QDomElement &rDomElement)
{
    QDomElement xmlObject = appendDomElement(rDomElement, mHmfTagName);
    saveCoreDataToDomElement(xmlObject);
    saveGuiDataToDomElement(xmlObject);
}

void ModelObject::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    rDomElement.setAttribute(HMF_TYPENAME, getTypeName());
    rDomElement.setAttribute(HMF_NAMETAG, getName());
}

QDomElement ModelObject::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    //Save GUI realted stuff
    QDomElement xmlGuiStuff = appendDomElement(rDomElement,HMF_HOPSANGUITAG);

        //Save center pos in parent coordinates (same as scene coordinates for model objects)
    QPointF cpos = this->getCenterPos();

    appendPoseTag(xmlGuiStuff, cpos.x(), cpos.y(), rotation(), this->mIsFlipped, 10);
    QDomElement nametext = appendDomElement(xmlGuiStuff, HMF_NAMETEXTTAG);
    nametext.setAttribute("position", getNameTextPos());
    nametext.setAttribute("visible", mpNameText->isVisible());

    return xmlGuiStuff;
}


void ModelObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsItem::contextMenuEvent(event);

    qDebug() << "contextMenuEvent()";

    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
        return;

    if(!mpParentContainerObject->mpParentProjectTab->isEditingEnabled())
        return;

    QMenu menu;
    this->buildBaseContextMenu(menu, event);
}


//! @brief Defines what happens when mouse starts hovering the object
void ModelObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverEnterEvent(event);
    this->setZValue(HOVEREDMODELOBJECT_Z);
    this->showPorts(true);
}


//! @brief Defines what happens when mouse stops hovering the object
void ModelObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverLeaveEvent(event);
    this->setZValue(MODELOBJECT_Z);
    this->showPorts(false);
}


////! @brief Defines what happens if a mouse key is pressed while hovering an object
//void GUIModelObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//        //Store old positions for all components, in case more than one is selected
//    if(event->button() == Qt::LeftButton)
//    {
//        for(int i = 0; i < mpParentSystem->mSelectedGUIObjectsList.size(); ++i)
//        {
//            mpParentSystem->mSelectedGUIObjectsList[i]->mOldPos = mpParentSystem->mSelectedGUIObjectsList[i]->pos();
//        }
//    }

//        //Objects shall not be selectable while creating a connector
//    if(mpParentSystem->mIsCreatingConnector)
//    {
//        this->setSelected(false);
//        this->setActive(false);
//    }
//}


void ModelObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    WorkspaceObject::mousePressEvent(event);

    qDebug() << "mousePressEvent(), button = " << event->button();

    if(event->button() == Qt::RightButton)
    {
        mDragCopying = true;
    }

//    if(mpParentContainerObject != 0 && mpParentContainerObject->mpParentProjectTab->getGraphicsView()->isShiftKeyPressed())
//    {
//        mpParentContainerObject->deselectAll();
//        this->select();
//        mpParentContainerObject->copySelected(mpParentContainerObject->getDragCopyStackPtr());

//        QMimeData *mimeData = new QMimeData;
//        mimeData->setText("HOPSANDRAGCOPY");

//        QDrag *drag = new QDrag(mpParentContainerObject->mpParentProjectTab->getGraphicsView());
//        drag->setMimeData(mimeData);
//        drag->setPixmap(QIcon(QPixmap(this->mModelObjectAppearance.getIconPath(mIconType, ABSOLUTE))).pixmap(40,40));
//        drag->setHotSpot(QPoint(20, 20));
//        drag->exec(Qt::CopyAction | Qt::MoveAction);
//    }

}


void ModelObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

    //qDebug() << "mouseMoveEvent(), button = " << event->button();

    if(mpParentContainerObject != 0 && mDragCopying)
    {
        //qDebug() << "Drag copying";
        mpParentContainerObject->deselectAll();
        this->select();
        mpParentContainerObject->copySelected(mpParentContainerObject->getDragCopyStackPtr());

        QMimeData *mimeData = new QMimeData;
        mimeData->setText("HOPSANDRAGCOPY");

        QDrag *drag = new QDrag(mpParentContainerObject->mpParentProjectTab->getGraphicsView());
        drag->setMimeData(mimeData);
        drag->setPixmap(QIcon(QPixmap(this->mModelObjectAppearance.getIconPath(mIconType, ABSOLUTE))).pixmap(40,40));
        drag->setHotSpot(QPoint(20, 20));
        drag->exec(Qt::CopyAction | Qt::MoveAction);

        mDragCopying = false;
    }
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void ModelObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(mpParentContainerObject == 0)
    {
        return;
    }

    mDragCopying = false;

    qDebug() << "mouseReleaseEvent()";

    QList<ModelObject *>::iterator it;

        //Loop through all selected objects and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    QList<ModelObject *> selectedObjects = mpParentContainerObject->getSelectedModelObjectPtrs();
    for(it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
    {
        if(((*it)->mOldPos != (*it)->pos()) && (event->button() == Qt::LeftButton))
        {
            emit objectMoved();
                //This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentContainerObject->getSelectedModelObjectPtrs().size() > 1)
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost("movedmultiple");
                }
                else
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost();
                }
                mpParentContainerObject->mpParentProjectTab->hasChanged();
                alreadyClearedRedo = true;
            }
            mpParentContainerObject->getUndoStackPtr()->registerMovedObject((*it)->mOldPos, (*it)->pos(), (*it)->getName());
        }
    }

    //Open the context menu (this will force it to open on mouse release regardless of operation system - not verified in Linux yet)
    if(event->button() == Qt::RightButton)
    {
        QGraphicsSceneContextMenuEvent *test = new QGraphicsSceneContextMenuEvent(QGraphicsSceneContextMenuEvent::ContextMenu);
        test->setScenePos(event->scenePos());
        test->setScreenPos(event->screenPos());
        mpParentContainerObject->mpParentProjectTab->mpGraphicsView->setIgnoreNextContextMenuEvent();       //! @todo No idea why this is needed, but it is...
        this->contextMenuEvent(test);
    }

    WorkspaceObject::mouseReleaseEvent(event);
}

//    }

//    QGraphicsWidget::mouseReleaseEvent(event);

//        //Objects shall not be selectable while creating a connector
//    if(mpParentSystem->mIsCreatingConnector)
//    {
//        this->setSelected(false);
//        this->setActive(false);
//    }
//}

QAction *ModelObject::buildBaseContextMenu(QMenu &rMenu, QGraphicsSceneContextMenuEvent* pEvent)
{
    rMenu.addSeparator();
    QAction *groupAction;

    //! @todo Grouping is deactivated because it does not currently work!
    if (!this->scene()->selectedItems().empty())
        groupAction = rMenu.addAction(tr("Group components"));
    else
        groupAction = new QAction(this);

    QAction *showNameAction = rMenu.addAction(tr("Show name"));
    QAction *rotateRightAction = rMenu.addAction(tr("Rotate Clockwise (Ctrl+R)"));
    QAction *rotateLeftAction = rMenu.addAction(tr("Rotate Counter-Clockwise (Ctrl+E)"));
    QAction *flipVerticalAction = rMenu.addAction(tr("Flip Vertically (Ctrl+D)"));
    QAction *flipHorizontalAction = rMenu.addAction(tr("Flip Horizontally (Ctrl+F)"));
    //QStringList replacements = this->getAppearanceData()->getReplacementObjects();
    QStringList replacements = gpMainWindow->mpLibrary->getReplacements(this->getTypeName());
    qDebug() << "Replacements = " << replacements;
    QList<QAction *> replaceActionList;
    if(!replacements.isEmpty())
    {
        QMenu *replaceMenu = rMenu.addMenu(tr("Replace component"));
        for(int i=0; i<replacements.size(); ++i)
        {
            QAction *replaceAction = replaceMenu->addAction(gpMainWindow->mpLibrary->getAppearanceData(replacements.at(i))->getDisplayName());
            replaceActionList.append(replaceAction);
        }
    }
    showNameAction->setCheckable(true);
    showNameAction->setChecked(mpNameText->isVisible());
    rMenu.addSeparator();
    QAction *parameterAction = rMenu.addAction(tr("Properties"));
    QAction *selectedAction = rMenu.exec(pEvent->screenPos());


    if (selectedAction == parameterAction)
    {
        openPropertiesDialog();
    }
    else if (selectedAction == rotateRightAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->rotate90cw();
    }
    else if (selectedAction == rotateLeftAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->rotate90ccw();
    }
    else if (selectedAction == flipVerticalAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->flipVertical();
    }
    else if (selectedAction == flipHorizontalAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->flipHorizontal();
    }
    else if (selectedAction == showNameAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        if(mpNameText->isVisible())
        {
            this->hideName();
        }
        else
        {
            this->showName();
        }
    }
    else if (selectedAction == groupAction)
    {
        this->mpParentContainerObject->groupSelected(pEvent->scenePos());
    }
    else
    {
        for(int i=0; i<replaceActionList.size(); ++i)
        {
            if(selectedAction == replaceActionList.at(i))
            {
                this->mpParentContainerObject->replaceComponent(this->getName(), replacements.at(i));
                return 0;
            }
        }
        return selectedAction;
    }
    //Return 0 action if any of the above actions were triggered
    return 0;
}



//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant ModelObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    WorkspaceObject::itemChange(change, value);   //This must be done BEFORE the snapping code to avoid an event loop. This is because snap uses "moveBy()", which triggers a new itemChange event.

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected() && mpParentContainerObject != 0)
        {
            mpParentContainerObject->rememberSelectedModelObject(this);
            connect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
        else if(mpParentContainerObject != 0)
        {
            mpParentContainerObject->forgetSelectedModelObject(this);
            disconnect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(mpParentContainerObject->mpParentProjectTab->getGraphicsView(), SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
    }

    //Snap if objects have moved
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        emit objectMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.

            //Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
        if(mpParentContainerObject != 0 && gConfig.getSnapping() &&
           !mpParentContainerObject->isCreatingConnector() && mpParentContainerObject->getSelectedModelObjectPtrs().size() == 1)
        {
                //Vertical snap
            if( (mConnectorPtrs.size() == 1) &&
                (mConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mConnectorPtrs.first()->isFirstAndLastDiagonal() && mConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mConnectorPtrs.first()->isFirstOrLastDiagonal() && mConnectorPtrs.first()->getNumberOfLines() > 1) &&
                (abs(mConnectorPtrs.first()->getStartPoint().x() - mConnectorPtrs.first()->getEndPoint().x()) < SNAPDISTANCE) &&
                (abs(mConnectorPtrs.first()->getStartPoint().x() - mConnectorPtrs.first()->getEndPoint().x()) > 0.0) )
            {
                if(this->mConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
                {
                    this->moveBy(mConnectorPtrs.first()->getEndPoint().x() - mConnectorPtrs.first()->getStartPoint().x(), 0);
                }
                else
                {
                    this->moveBy(mConnectorPtrs.first()->getStartPoint().x() - mConnectorPtrs.first()->getEndPoint().x(), 0);
                }
            }

                //Horizontal snap
            if( (mConnectorPtrs.size() == 1) &&
                (mConnectorPtrs.first()->getNumberOfLines() < 4) &&
                !(mConnectorPtrs.first()->isFirstAndLastDiagonal() && mConnectorPtrs.first()->getNumberOfLines() == 2) &&
                !(mConnectorPtrs.first()->isFirstOrLastDiagonal() && mConnectorPtrs.first()->getNumberOfLines() > 2) &&
                (abs(mConnectorPtrs.first()->getStartPoint().y() - mConnectorPtrs.first()->getEndPoint().y()) < SNAPDISTANCE) &&
                (abs(mConnectorPtrs.first()->getStartPoint().y() - mConnectorPtrs.first()->getEndPoint().y()) > 0.0) )
            {
                if(this->mConnectorPtrs.first()->getStartPort()->mpParentGuiModelObject == this)
                {
                    this->moveBy(0, mConnectorPtrs.first()->getEndPoint().y() - mConnectorPtrs.first()->getStartPoint().y());
                }
                else
                {
                    this->moveBy(0, mConnectorPtrs.first()->getStartPoint().y() - mConnectorPtrs.first()->getEndPoint().y());
                }
            }
        }
    }

    return value;
}


//! @brief Shows or hides the port, depending on the input boolean and whether or not they are connected
//! @param visible Tells whether the ports shall be shown or hidden
void ModelObject::showPorts(bool visible)
{
    QList<Port*>::iterator i;
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
            if ((*i)->isConnected() || mpParentContainerObject->areSubComponentPortsHidden())
            {
                (*i)->hide();
            }
        }
}


//! @todo try to reuse the code in rotate guiobject,
void ModelObject::rotate(qreal angle, undoStatus undoSettings)
{
    mpParentContainerObject->mpParentProjectTab->hasChanged();

    if(mIsFlipped)
    {
        angle *= -1;
    }
    this->setRotation(normDeg360(this->rotation()+angle));

    refreshIconPosition();

    int tempNameTextPos = mNameTextPos;
    this->snapNameTextPosition(mpNameText->pos());
    setNameTextPos(tempNameTextPos);

    for (int i=0; i < mPortListPtrs.size(); ++i)
    {
        mPortListPtrs[i]->refreshPortOverlayPosition();
    }

    if(undoSettings == UNDO)
    {
        mpParentContainerObject->getUndoStackPtr()->registerRotatedObject(this->getName(), angle);
    }

    for(int i=0; i<mConnectorPtrs.size(); ++i)
    {
        mConnectorPtrs.at(i)->drawConnector(true);
    }
}


//! @brief Slot that flips the object vertically
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipHorizontal()
//! @todo Fix name text position when flipping components
void ModelObject::flipVertical(undoStatus undoSettings)
{
    this->flipHorizontal(NOUNDO);
    this->rotate(180,NOUNDO);
    if(undoSettings == UNDO)
    {
        mpParentContainerObject->getUndoStackPtr()->registerVerticalFlip(this->getName());
    }
}


//! @brief Slot that flips the object horizontally
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipVertical()
void ModelObject::flipHorizontal(undoStatus undoSettings)
{
    if(mpParentContainerObject)
    {
        mpParentContainerObject->mpParentProjectTab->hasChanged();
    }
    QTransform transf;
    transf.scale(-1.0, 1.0);

    //Remember center pos
    QPointF cpos = this->getCenterPos();
    //Transform
    this->setTransform(transf,true); // transformationorigin point seems to have no effect here for some reason
    //Reset to center pos (as transform origin point was ignored)
    this->setCenterPos(cpos);

    // If the icon is (not rotating) its position will be refreshed
    refreshIconPosition();

    // Toggel isFlipped bool
    if(mIsFlipped)
    {
        mIsFlipped = false;
    }
    else
    {
        mIsFlipped = true;
    }

    //Refresh port overlay and nametext
    for (int i=0; i < mPortListPtrs.size(); ++i)
    {
        mPortListPtrs[i]->refreshPortOverlayPosition();
    }
    this->snapNameTextPosition(mpNameText->pos());

    if(undoSettings == UNDO)
    {
        mpParentContainerObject->getUndoStackPtr()->registerHorizontalFlip(this->getName());
    }
}


//! @brief Returns an number of the current name text position
//! @see setNameTextPos(int textPos)
//! @see fixTextPosition(QPointF pos)
int ModelObject::getNameTextPos()
{
    return mNameTextPos;
}


//! @brief Moves the name text to the specified name text position
//! @param textPos Number of the desired text position
//! @see getNameTextPos()
//! @see fixTextPosition(QPointF pos)
void ModelObject::setNameTextPos(int textPos)
{
    QVector<QPointF> pts;
    this->calcNameTextPositions(pts);
    mNameTextPos = textPos;
    mpNameText->setPos(pts[textPos]);
}


//! @brief Slots that hides the name text of the object
void ModelObject::hideName(undoStatus undoSettings)
{
    bool previousStatus = mpNameText->isVisible();
    mpNameText->setVisible(false);
    if(undoSettings == UNDO && previousStatus == true)
    {
        mpParentContainerObject->getUndoStackPtr()->registerNameVisibilityChange(this->getName(), false);
    }
}


//! @brief Slots that makes the name text of the object visible
void ModelObject::showName(undoStatus undoSettings)
{
    bool previousStatus = mpNameText->isVisible();
    mpNameText->setVisible(true);
    if(undoSettings == UNDO && previousStatus == false)
    {
        mpParentContainerObject->getUndoStackPtr()->registerNameVisibilityChange(this->getName(), true);
    }
}



//! @brief Virtual dummy function that returns the type name of the object (must be reimplemented by children)
QString ModelObject::getTypeName()
{
    assert(false);
    return "";
}


void ModelObject::deleteMe()
{
    //qDebug() << "deleteMe in " << this->getName();
    mpParentContainerObject->deleteModelObject(this->getName());
}

//! @brief Sets or updates the appearance data specific base path to which all icon paths should be relative
//! @todo Maybe this can be combined with the setModelFileInfo function
void ModelObject::setAppearanceDataBasePath(const QString basePath)
{
    mModelObjectAppearance.setBasePath(basePath);
}

//! @brief Returns a pointer to the appearance data object
ModelObjectAppearance* ModelObject::getAppearanceData()
{
    return &mModelObjectAppearance;
}

//! @brief Refreshes the appearance and position of ports on the model object
void ModelObject::refreshExternalPortsAppearanceAndPosition()
{
    //For model objects we assume for now that the appearance of ports will not change, but the position and hide/show might
    PortAppearanceMapT::Iterator it;
    for(it=mModelObjectAppearance.getPortAppearanceMap().begin(); it != mModelObjectAppearance.getPortAppearanceMap().end(); ++it)
    {
        Port *port = getPort(it.key());
        if(port != 0)
        {
            port->setCenterPosByFraction(it.value().x, it.value().y);
            port->setRotation(it.value().rot);
            port->setEnable(it.value().mEnabled);
        }
    }
    redrawConnectors();
}

//! @brief Refreshes the appearance of the object
void ModelObject::refreshAppearance()
{
    //! @todo should make sure we only do this if we really need to resize (after icon change)
    QPointF centerPos =  this->getCenterPos(); //Remember center pos for resize
    this->setIcon(mIconType);
    this->setCenterPos(centerPos); //Re-set center pos after resize

    this->refreshDisplayName();
    this->refreshExternalPortsAppearanceAndPosition();
}


QString ModelObject::getHelpPicture()
{
    return mModelObjectAppearance.getHelpPicture();
}


//! @brief Reimplementation - a model object is never "invisible", only its icon is
bool ModelObject::isVisible()
{
    return mpIcon->isVisible();
}


QGraphicsSvgItem *ModelObject::getIcon()
{
    return mpIcon;
}


QString ModelObject::getHelpText()
{
    return mModelObjectAppearance.getHelpText();
}


//! @brief Construtor for the name text object
//! @param pParent Pointer to the object which the name text belongs to
ModelObjectDisplayName::ModelObjectDisplayName(ModelObject *pParent)
    :   QGraphicsTextItem(pParent)
{
    mpParentModelObject = pParent;
    this->setTextInteractionFlags(Qt::NoTextInteraction);
    this->setFlags(QGraphicsItem::ItemIsFocusable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations);
}


//! @brief Defines what happens when a mouse button is released (used to update position when text has moved)
void ModelObjectDisplayName::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    emit textMoved(this->pos());
    QGraphicsTextItem::mouseReleaseEvent(event);
}


//! @brief Defines what happens when selection status of name text has changed
//! @param change Type of change (only ItemSelectedHasChanged is used)
QVariant ModelObjectDisplayName::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QGraphicsTextItem::itemChange(change, value);

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (this->isSelected())
        {
            mpParentModelObject->getParentContainerObject()->deselectSelectedNameText();
            connect(mpParentModelObject->getParentContainerObject(), SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
        else
        {
            disconnect(mpParentModelObject->getParentContainerObject(), SIGNAL(deselectAllNameText()),this,SLOT(deselect()));
        }
    }
    return value;
}


//! @brief Slot that deselects the name text
void ModelObjectDisplayName::deselect()
{
    this->setSelected(false);
}

