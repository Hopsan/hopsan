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
//! @file   GUIModelObject.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIModelObject class (The baseclass for all objects representing model parts)
//!
//$Id$

#include <QGraphicsColorizeEffect>
//! @todo figure out a way not to need to include everything here (qtGui / qtWidgets) due to multiplication of qtransform and qpointf
#if QT_VERSION >= 0x050000
    #include <QtWidgets>
#else
    #include <QtGui>
#endif


#include "GUIModelObject.h"

#include "GUISystem.h"
#include "Widgets/ModelWidget.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "MessageHandler.h"
#include "LibraryHandler.h"
#include "UndoStack.h"
#include "Configuration.h"
#include "global.h"

#include <cassert>

//! @brief Constructor for GUI Objects
//! @param position Initial scene coordinates where object shall be placed
//! @param rotation Initial rotation of the object
//! @param pAppearanceData Pointer to appearance data object
//! @param startSelected Initial selection status
//! @param gfxType Initial graphics type (user or iso)
//! @param system Pointer to the parent system
//! @param parent Pointer to parent object (not mandatory)
ModelObject::ModelObject(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType, ContainerObject *pParentContainer, QGraphicsItem *pParent)
        : WorkspaceObject(position, rotation, startSelected, pParentContainer, pParent)
{
    // Initialize variables
    mName="no_name_set_yet";
    mpIcon = 0;
    mpNameText = 0;
    mTextOffset = 5.0;
    mDragCopying = false;
    mNameTextAlwaysVisible = false;
    mNameTextVisible = false;
    mpDialogParentWidget = new QWidget(gpMainWindowWidget);
    mIsLocked = false;

    // Make a local copy of the appearance data (that can safely be modified if needed)
    if (pAppearanceData != 0)
    {
        mModelObjectAppearance = *pAppearanceData;
        mName = mModelObjectAppearance.getDisplayName(); //Default name to the appearance data display name
    }

    // Setup appearance
    setIcon(gfxType);
    refreshAppearance();
    setCenterPos(position);
    rememberPos();
    setZValue(ModelobjectZValue);
    setSelected(startSelected);

        //Create the textbox containing the name
    mpNameText = new ModelObjectDisplayName(this);
    mpNameText->setFlag(QGraphicsItem::ItemIsSelectable, false); //To minimize problems when move after copy and so on
    if(mpParentContainerObject)
    {
        setNameTextScale(mpParentContainerObject->mpModelWidget->mpGraphicsView->getZoomFactor());
    }
    setNameTextPos(0); //Set initial name text position
    if(pParentContainer && pParentContainer->areSubComponentNamesShown())
    {
        showName(NoUndo);
    }
    else
    {
        hideName(NoUndo);
    }

    // Create connections
    connect(mpNameText, SIGNAL(textMoved(QPointF)), SLOT(snapNameTextPosition(QPointF)));
    if(mpParentContainerObject != 0)
    {
        connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(zoomChange(double)), this, SLOT(setNameTextScale(double)));
//        connect(mpParentContainerObject, SIGNAL(selectAllGUIObjects()), this, SLOT(select()));
        connect(mpParentContainerObject, SIGNAL(hideAllNameText()), this, SLOT(hideName()));
        connect(mpParentContainerObject, SIGNAL(showAllNameText()), this, SLOT(showName()));
        connect(mpParentContainerObject, SIGNAL(setAllGfxType(GraphicsTypeEnumT)), this, SLOT(setIcon(GraphicsTypeEnumT)));
        connect(this, SIGNAL(quantityChanged(QString,QString)), mpParentContainerObject->mpModelWidget, SIGNAL(quantityChanged(QString,QString)));
    }
    else
    {
        //maybe something different
    }

    mpLossesDisplay = new QGraphicsTextItem(this);
    mpLossesDisplay->setFlags(QGraphicsItem::ItemIgnoresTransformations);
    mpLossesDisplay->setVisible(false);

    mnCppInputs = 0;
    mnCppOutputs = 0;
}


//! @brief Destructor for GUI Objects
ModelObject::~ModelObject()
{
    emit objectDeleted();
}

void ModelObject::deleteInHopsanCore()
{
    //Needs to be overloaded
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

QStringList ModelObject::getParentSystemNameHieararchy() const
{
    if (mpParentContainerObject)
    {
        return mpParentContainerObject->getSystemNameHieararchy();
    }
    return QStringList();
}

QStringList ModelObject::getSystemNameHieararchy() const
{
    return getParentSystemNameHieararchy();
}


//! @brief Returns the type of the object (object, component, systemport, group etc)
int ModelObject::type() const
{
    return Type;
}

QString ModelObject::getHmfTagName() const
{
    return HMF_OBJECTTAG; //!< @todo change this
}


void ModelObject::getLosses(double &total, QMap<QString, double> domainSpecificLosses)
{
    total = mTotalLosses;
    domainSpecificLosses = mDomainSpecificLosses;
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
        //We don't use this.setnamepos here as that would recalculate the positions again
        mpNameText->setPos(pts[0]);
        mNameTextPos = 0;
    }
    else
    {
        //We don't use this.setnamepos here as that would recalculate the positions again
        mpNameText->setPos(pts[1]);
        mNameTextPos = 1;
    }

    if(mpParentContainerObject != 0)
    {
        mpParentContainerObject->mpModelWidget->getGraphicsView()->updateViewPort();
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
    //Now we transform the name text positions
    //pt0 = top, pt1 = bottom, pts relative local center on object
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
void ModelObject::setNameTextScale(double scale)
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

//! @param Returns a copy of the list with pointers to the connectors connected to the object
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
QString ModelObject::getName() const
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
void ModelObject::setIcon(GraphicsTypeEnumT gfxType)
{
    QString iconPath;
    double iconScale;
    if ( (gfxType == ISOGraphics) && mModelObjectAppearance.hasIcon(ISOGraphics) )
    {
        iconPath = mModelObjectAppearance.getFullAvailableIconPath(ISOGraphics);
        iconScale = mModelObjectAppearance.getIconScale(ISOGraphics);
        mIconType = ISOGraphics;
    }
    else
    {
        iconPath = mModelObjectAppearance.getFullAvailableIconPath(UserGraphics);
        iconScale = mModelObjectAppearance.getIconScale(UserGraphics);
        mIconType = UserGraphics;
    }

    //Avoid swapping icon if same as before, we swap also if scale changes
    if  ( (mLastIconPath != iconPath) || !fuzzyEqual(mLastIconScale, iconScale, 0.001) )
    {
        if (mpIcon != 0)
        {
            mpIcon->deleteLater(); //Schedule previous icon for deletion
            disconnect(this->getParentContainerObject()->mpModelWidget->getGraphicsView(), SIGNAL(zoomChange(double)), this, SLOT(setIconZoom(double)));
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
                mpIcon->setScale(this->getParentContainerObject()->mpModelWidget->getGraphicsView()->getZoomFactor()*iconScale);
                connect(this->getParentContainerObject()->mpModelWidget->getGraphicsView(), SIGNAL(zoomChange(double)), this, SLOT(setIconZoom(double)), Qt::UniqueConnection);
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

void ModelObject::setIconZoom(const double zoom)
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

    QString unit = " J";
    double div = 1;
    if(mpParentContainerObject->mpAvgPwrRadioButton->isChecked())
    {
        unit = " W";
        div = mpParentContainerObject->getLogDataHandler()->copyTimeVector(-1).last();
    }

    if(getTypeCQS() == "S")
        return;

    int generation = mpParentContainerObject->getLogDataHandler()->getCurrentGenerationNumber();

    time.start();

    for(int p=0; p<mPortListPtrs.size(); ++p)
    {
        QString portType = mPortListPtrs[p]->getPortType();
        if(portType == "SystemPortType")
        {
            portType = mPortListPtrs[p]->getPortType(CoreSystemAccess::InternalPortType);
        }
        QStringList nodeTypes;
        NodeInfo::getNodeTypes(nodeTypes);
        Q_FOREACH(const QString &type, nodeTypes)
        {
            if(mPortListPtrs[p]->getNodeType() == type && portType != "ReadPortType")
            {
                //Power port, so we must cycle all connected ports and ask for their data
                if(mPortListPtrs[p]->getPortType() == "PowerMultiportType" || mPortListPtrs[p]->getPortType() == "SIGNALMULTIPORT")
                {
                    QVector<Port *> vConnectedPorts = mPortListPtrs[p]->getConnectedPorts();
                    for(int i=0; i<vConnectedPorts.size(); ++i)
                    {
                        if(vConnectedPorts.at(i)->getPortType() == "ReadPortType")
                        {
                            continue;
                        }
                        QString componentName = vConnectedPorts.at(i)->getParentModelObjectName();
                        QString portName = vConnectedPorts.at(i)->getName();
                        QStringList sysHieararchy = vConnectedPorts.at(i)->getParentModelObject()->getParentSystemNameHieararchy();
                        //! @todo Multiplying intensity with flow will give correct value for all nodes except pneumatics (that use massflow), figure out how to solve this
                        QVector<double> vIntensity = mpParentContainerObject->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(sysHieararchy, componentName, portName, NodeInfo(type).intensity), generation);
                        QVector<double> vFlow = mpParentContainerObject->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(sysHieararchy, componentName, portName, NodeInfo(type).flow), generation);
                        QVector<double> vTime = mpParentContainerObject->getLogDataHandler()->copyTimeVector(generation);
                        for(int s=0; s<vIntensity.size()-1; ++s) //Minus one because of integration method
                        {
                            //! @todo here and bellow there is a risk for slowdown when timevector is cached to disk, should copy the vector first (at is the same as peek)
                            mTotalLosses += vIntensity.at(s) * vFlow.at(s) * (vTime.at(s+1)-vTime.at(s));
                            mDomainSpecificLosses.insert(NodeInfo(type).niceName, vIntensity.at(s) * vFlow.at(s) * (vTime.at(s+1)-vTime.at(s)));
                        }
                    }
                }
                else    //Normal port!
                {
                    //! @todo Multiplying intensity with flow will give correct value for all nodes except pneumatics (that use massflow), figure out how to solve this
                    QVector<double> vIntensity = mpParentContainerObject->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(getParentSystemNameHieararchy(), getName(), mPortListPtrs[p]->getName(), NodeInfo(type).intensity), generation);
                    QVector<double> vFlow = mpParentContainerObject->getLogDataHandler()->copyVariableDataVector(makeFullVariableName(getParentSystemNameHieararchy(), getName(), mPortListPtrs[p]->getName(), NodeInfo(type).flow), generation);
                    QVector<double> vTime = mpParentContainerObject->getLogDataHandler()->copyTimeVector(generation);
                    for(int s=0; s<vIntensity.size()-1; ++s) //Minus one because of integration method
                    {
                        mTotalLosses += vIntensity.at(s) * vFlow.at(s) * (vTime.at(s+1) - vTime.at(s));
                        mDomainSpecificLosses.insert(NodeInfo(type).niceName, vIntensity.at(s) * vFlow.at(s) * (vTime.at(s+1) - vTime.at(s)));
                    }
                }
            }
        }
    }

    if(mTotalLosses != 0)
    {
        if(getTypeCQS() == "Q")     //Invert losses for Q components (because positive direction is defined as outwards for Q and inwards for C)
        {
            mTotalLosses *= -1;
            QMap<QString, double>::iterator it;
            for(it=mDomainSpecificLosses.begin(); it!=mDomainSpecificLosses.end(); ++it)
            {
                it.value() *= -1;
            }
        }
        QString totalString;
        totalString.setNum(mTotalLosses/div);
        QString totalAddedString;
        totalAddedString.setNum(-mTotalLosses/div);

        QString label;
        if(mTotalLosses > 0)
        {
            label = "<p><span style=\"background-color:lightyellow; color:red\"><b>&#160;&#160;Total losses: " + totalString + unit+"&#160;&#160;</b>";
        }
        else
        {
            QString added = "Added energy";
            if(unit == " W") added = "Added power";
            label = "<p><span style=\"background-color:lightyellow; color:green\">&#160;&#160;"+added+": <b>" + totalAddedString + unit+"</b>&#160;&#160;";
        }

        QMap<QString, double>::iterator it;
        for(it=mDomainSpecificLosses.begin(); it!=mDomainSpecificLosses.end(); ++it)
        {
            if(it.value() > 0 && it.value() != mTotalLosses)
            {
                label.append("<br>&#160;&#160;"+it.key()+" losses: <b>" + QString::number(it.value()/div) + unit+"</b>&#160;&#160;");
            }
            else if(it.value() < 0 && it.value() != mTotalLosses)
            {
                QString energyOrPower = "energy";
                if(unit == " W") energyOrPower = "power";
                label.append("<br><font color=\"green\">&#160;&#160;Added " + QString::number(it.value()/div) + " "+energyOrPower+": <b>" + QString::number(it.value()) + unit+"</b>&#160;&#160;</font>");
            }
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
        mpLossesDisplay->setZValue(LossesDisplayZValue);
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

void ModelObject::highlight()
{
    QGraphicsColorizeEffect *pEffect = new QGraphicsColorizeEffect(this);
    pEffect->setColor(QColor("orangered"));
    setGraphicsEffect(pEffect);
    if (getParentContainerObject())
    {
        connect(getParentContainerObject()->mpModelWidget->getGraphicsView(), SIGNAL(unHighlightAll()), this, SLOT(unHighlight()), Qt::UniqueConnection);
    }

}

void ModelObject::unHighlight()
{
    if(graphicsEffect())
    {
        graphicsEffect()->setEnabled(false);
        disconnect(getParentContainerObject()->mpModelWidget->getGraphicsView(), SIGNAL(unHighlightAll()), this, SLOT(unHighlight()));
    }
}

void ModelObject::setIsLocked(bool value)
{
    mIsLocked = value;
}


bool ModelObject::isLossesDisplayVisible()
{
    return mpLossesDisplay->isVisible();
}


//! @brief Get a pointer to the port with the specified name
//! @param [in] rName The port name
//! @returns A pointer to the port or 0 if no port was found
Port *ModelObject::getPort(const QString &rName)
{
    for (int i=0; i<mPortListPtrs.size(); ++i)
    {
        if (mPortListPtrs[i]->getName() == rName)
        {
            return mPortListPtrs[i];
        }
    }
    return 0;
}


//! @brief This method creates ONE external port. Or refreshes existing ports. If port appearance information for this port does not exist, it is created
//! @param[portName] The name of the port to create
Port *ModelObject::createRefreshExternalPort(QString portName)
{
    // Fetch port if we have it
    Port *pPort = this->getPort(portName);

    // Create new external port if it does not already exist (this is the usual case for components)
    if ( pPort == 0 )
    {
        PortAppearance *pPortApp = mModelObjectAppearance.getPortAppearance(portName);
        // If port appearance is not already existing then we create it
        if ( !pPortApp )
        {
            mModelObjectAppearance.addPortAppearance(portName);
            pPortApp = mModelObjectAppearance.getPortAppearance(portName);
        }

        //! @todo to minimize search time make a get porttype  and nodetype function, we need to search twice now
        QString nodeType = getParentContainerObject()->getCoreSystemAccessPtr()->getNodeType(this->getName(), portName);
        QString portType = getParentContainerObject()->getCoreSystemAccessPtr()->getPortType(this->getName(), portName);

        if (!portType.isEmpty())
        {
            pPortApp->selectPortIcon(getTypeCQS(), portType, nodeType);

            double x = pPortApp->x*boundingRect().width();
            double y = pPortApp->y*boundingRect().height();
            //qDebug() << "x,y: " << x << " " << y;

            pPort = new Port(portName, x, y, pPortApp, this);

            mPortListPtrs.append(pPort);
        }
        else
        {
            // Port does not exist in core, lets print warning message and remove its appearance data
            gpMessageHandler->addWarningMessage("Port:  "+portName+"  does not exist in Component:  "+getName()+"  when trying to create port graphics. Ignoring!");
            mModelObjectAppearance.erasePortAppearance(portName);
        }
    }
    else
    {
        // The external port already seems to exist, lets update it in case something has changed
        //! @todo Maybe need to have a refresh portappearance function, don't really know if this will ever be used though, will fix when it becomes necessary
        pPort->refreshPortGraphics();

        // Adjust the position
        pPort->setCenterPosByFraction(pPort->getPortAppearance()->x, pPort->getPortAppearance()->y);

        // In this case connections exist, also refresh any attached connectors, if types have changed
        //! @todo we always update, maybe we should be more smart and only update if changed, but I think this should be handled inside the connector class (the smartness)
        QVector<Connector*> connectors = pPort->getAttachedConnectorPtrs();
        for (int i=0; i<connectors.size(); ++i)
        {
            connectors[i]->refreshConnectorAppearance();
        }

        qDebug() << "--------------------------ExternalPort already exist refreshing its graphics: " << portName << " in: " << this->getName();
    }
    return pPort;
}

//! @brief Removes an external Port from a container object
//! @param[in] portName The name of the port to be removed
void ModelObject::removeExternalPort(QString portName)
{
    QList<Port*>::iterator plit;
    for (plit=mPortListPtrs.begin(); plit!=mPortListPtrs.end(); ++plit)
    {
        if ((*plit)->getName() == portName )
        {
            // Delete the GUIPort its post in the port list and its appearance data
            (*plit)->disconnectAndRemoveAllConnectedConnectors();
            (*plit)->deleteLater();
            mPortListPtrs.erase(plit);
            mModelObjectAppearance.erasePortAppearance(portName); // It is important that we remove the appearance data last, or some pointers will be dangling
            break;
        }
    }
}

//! @brief Get the default value of a parameter
//! @param [in] paramName The name of the parameter
//! @returns QString with default value for parameter, or empty QString if paramName not found
QString ModelObject::getDefaultParameterValue(const QString &rParamName) const
{
    if(mDefaultParameterValues.contains(rParamName))
    {
        return mDefaultParameterValues.find(rParamName).value();
    }
    else
    {
        return QString();
    }
}

//! @brief Returns a map with the variable names (key) and alias (value)
//! @param[in] rPortName The portname to filter on, if left blank all ports are considered
//! @returns A map with key = VarName if rPortName was specified, else a combo PortName#VarName and value = alias
QMap<QString, QString> ModelObject::getVariableAliases(const QString &rPortName) const
{
    QMap<QString, QString> results;
    QVector<CoreVariameterDescription> vds;
    getVariameterDescriptions(vds);
    for (int i=0; i<vds.size(); ++i)
    {
        if ( !vds[i].mAlias.isEmpty() && rPortName.isEmpty() && (gpConfig->getBoolSetting(CFG_SHOWHIDDENNODEDATAVARIABLES) || (vds[i].mVariabelType != "Hidden")) )
        {
            results.insert(vds[i].mPortName+"#"+vds[i].mName, vds[i].mAlias);
        }
        else if ( !vds[i].mAlias.isEmpty() && (rPortName == vds[i].mPortName)  && (gpConfig->getBoolSetting(CFG_SHOWHIDDENNODEDATAVARIABLES) || (vds[i].mVariabelType != "Hidden")) )
        {
            results.insert(vds[i].mName, vds[i].mAlias);
        }
    }
    return results;
}

//void ModelObject::getVariableDataDescriptions(QVector<CoreVariableData> &rVarDataDescriptions)
//{
//    rVarDataDescriptions.clear();
//    if (mpParentContainerObject)
//    {
//        QList<Port*>::iterator pit;
//        for (pit=mPortListPtrs.begin(); pit!=mPortListPtrs.end(); ++pit)
//        {
//            QVector<CoreVariableData> varDescs;
//            mpParentContainerObject->getCoreSystemAccessPtr()->getVariableDescriptions(this->getName(), (*pit)->getName(), varDescs);
//            for (int i=0; i<varDescs.size(); ++i)
//            {
//                rVarDataDescriptions.push_back(varDescs[i]);
//            }
//        }
//    }
//}

void ModelObject::getVariameterDescriptions(QVector<CoreVariameterDescription> &rVariameterDescriptions) const
{
    rVariameterDescriptions.clear();
    if (mpParentContainerObject)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->getVariameters(this->getName(), rVariameterDescriptions);
    }
}

void ModelObject::setInvertPlotVariable(const QString &rName, bool tf)
{
    if (tf)
    {
        mRegisteredInvertPlotVariables.insert(rName, tf);
    }
    else
    {
        // No point in keeping negative values, so actually this could be a string list
        mRegisteredInvertPlotVariables.remove(rName);
    }
}

bool ModelObject::getInvertPlotVariable(const QString &rName) const
{
    return mRegisteredInvertPlotVariables.value(rName, false);
}

void ModelObject::setVariablePlotLabel(const QString &rName, const QString &rLabel)
{
    // If label is empty that means Remove
    if (rLabel.isEmpty())
    {
        mRegisteredPlotLabels.remove(rName);
    }
    else
    {
        mRegisteredPlotLabels.insert(rName, rLabel);
    }
}

QString ModelObject::getVariablePlotLabel(const QString &rName) const
{
    return mRegisteredPlotLabels.value(rName, "");
}


bool ModelObject::setModifyableSignalQuantity(const QString &rVariablePortDataName, const QString &rQuantity)
{
    bool rc = getParentContainerObject()->getCoreSystemAccessPtr()->setModifyableSignalQuantity(this->getName()+"#"+rVariablePortDataName, rQuantity);
    if (rc)
    {
        emit quantityChanged(makeFullVariableName(getSystemNameHieararchy(),"","",getName()+"#"+rVariablePortDataName), rQuantity);
    }
    return rc;
}

QString ModelObject::getModifyableSignalQuantity(const QString &rVariablePortDataName)
{
    return getParentContainerObject()->getCoreSystemAccessPtr()->getModifyableSignalQuantity(this->getName()+"#"+rVariablePortDataName);
}

void ModelObject::loadFromDomElement(QDomElement domElement)
{
    Q_UNUSED(domElement)
    //! @todo we use separate lode functions right now, but maybe the objects should be able to load themselves
}

//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString ModelObject::getParameterValue(const QString paramName)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getParameterValue(this->getName(), paramName);
}

bool ModelObject::hasParameter(const QString &rParamName)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->hasParameter(this->getName(), rParamName);
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
    return false;
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
//! @deprecated
bool ModelObject::setStartValue(QString /*portName*/, QString /*variable*/, QString /*sysParName*/)
{
    //cout << "This function should only be available in GUIComponent and  GUISubsystem" << endl;
    assert(false);
    return false;
}

bool ModelObject::registerCustomParameterUnitScale(QString name, UnitConverter us)
{
    if (us.isEmpty())
    {
        unregisterCustomParameterUnitScale(name);
    }
    else
    {
        mRegisteredCustomParameterUnitScales.insert(name, us);
    }
    return true;
}

bool ModelObject::unregisterCustomParameterUnitScale(QString name)
{
    return (mRegisteredCustomParameterUnitScales.remove(name) > 0);
}

bool ModelObject::getCustomParameterUnitScale(QString name, UnitConverter &rUs)
{
    QMap<QString, UnitConverter>::iterator it = mRegisteredCustomParameterUnitScales.find(name);
    if (it != mRegisteredCustomParameterUnitScales.end())
    {
        rUs = it.value();
        return true;
    }
    return false;
}

void ModelObject::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    if (contents == FullModel)
    {
        QDomElement xmlObject = appendDomElement(rDomElement, getHmfTagName());
        saveCoreDataToDomElement(xmlObject);
        saveGuiDataToDomElement(xmlObject);
    }
    else
    {
        QDomElement xmlObject = appendDomElement(rDomElement, getHmfTagName());
        saveCoreDataToDomElement(xmlObject, contents);
    }
}

void ModelObject::setModelFileInfo(QFile &rFile, const QString relModelPath)
{
    Q_UNUSED(rFile)
    Q_UNUSED(relModelPath)
    // Does nothing by default, should be overloaded
}

void ModelObject::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    if(contents==FullModel)
    {
        rDomElement.setAttribute(HMF_TYPENAME, getTypeName());
        rDomElement.setAttribute(HMF_SUBTYPENAME, getSubTypeName());
    }
    rDomElement.setAttribute(HMF_NAMETAG, getName());
    rDomElement.setAttribute(HMF_CQSTYPE, getTypeCQS());
}

QDomElement ModelObject::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    if (!getSubTypeName().isEmpty())
    {
        rDomElement.setAttribute(HMF_SUBTYPENAME, getSubTypeName());
    }

    // Save GUI related stuff
    QDomElement xmlGuiStuff = appendDomElement(rDomElement,HMF_HOPSANGUITAG);

    // Save center pos in parent coordinates (same as scene coordinates for model objects)
    QPointF cpos = this->getCenterPos();
    appendPoseTag(xmlGuiStuff, cpos.x(), cpos.y(), rotation(), this->mIsFlipped, 10);

    // Save the text displaying the component name
    QDomElement nametext = appendDomElement(xmlGuiStuff, HMF_NAMETEXTTAG);
    nametext.setAttribute("position", getNameTextPos());
    nametext.setAttribute("visible", mNameTextAlwaysVisible);

    // Save any custom selected parameter scales
    if (!mRegisteredCustomParameterUnitScales.isEmpty())
    {
        QDomElement plotscales = appendDomElement(xmlGuiStuff, HMF_PARAMETERSCALES);
        QMap<QString, UnitConverter>::iterator psit;
        for (psit=mRegisteredCustomParameterUnitScales.begin(); psit!=mRegisteredCustomParameterUnitScales.end(); ++psit)
        {
            UnitConverter &us = psit.value();
            QDomElement plotscale = appendDomElement(plotscales, HMF_PARAMETERSCALE);
            plotscale.setAttribute(HMF_PARAMETERSCALEPARAMNAME, psit.key());
            plotscale.setAttribute(HMF_PARAMETERSCALEUNIT, us.mUnit);
            plotscale.setAttribute(HMF_PARAMETERSCALESCALE, us.mScale);
            if (!us.mOffset.isEmpty())
            {
                plotscale.setAttribute(HMF_PARAMETERSCALEOFFSET, us.mOffset);
            }
            if (!us.mQuantity.isEmpty())
            {
                plotscale.setAttribute(HMF_PARAMETERSCALEQUANTITY, us.mQuantity);
            }
            CoreParameterData data;
            getParameter(psit.key(), data);
            plotscale.setAttribute(HMF_PARAMETERSCALEVALUE, us.convertFromBase(data.mValue));
        }
    }

    // Save custom selected plot settings
    if (!mRegisteredInvertPlotVariables.isEmpty())
    {
        QDomElement plotsettings = appendDomElement(xmlGuiStuff, HMF_VARIABLEPLOTSETTINGS);
        QList<QString> invkeys = mRegisteredInvertPlotVariables.keys();
        QList<QString> labelkeys = mRegisteredPlotLabels.keys();
        for (QString &invkey : invkeys)
        {
            QDomElement plotsetting = appendDomElement(plotsettings, HMF_VARIABLEPLOTSETTING);
            plotsetting.setAttribute("name", invkey);
            plotsetting.setAttribute("invert", mRegisteredInvertPlotVariables.value(invkey));
            if (labelkeys.contains(invkey))
            {
                plotsetting.setAttribute("label", mRegisteredPlotLabels.value(invkey));
                labelkeys.removeAll(invkey);
            }
        }
        for (QString &labelkey : labelkeys)
        {
            QDomElement plotsetting = appendDomElement(plotsettings, HMF_VARIABLEPLOTSETTING);
            plotsetting.setAttribute("name", labelkey);
            plotsetting.setAttribute("label", mRegisteredPlotLabels.value(labelkey));
        }
    }
//    if (!mRegisteredCustomPlotUnitsOrScales.isEmpty())
//    {
//        QDomElement plotscales = appendDomElement(xmlGuiStuff, HMF_PLOTSCALES);
//        QMap<QString, UnitScale>::iterator psit;
//        for (psit=mRegisteredCustomPlotUnitsOrScales.begin(); psit!=mRegisteredCustomPlotUnitsOrScales.end(); ++psit)
//        {
//            UnitScale &val = psit.value();
//            QDomElement plotscale = appendDomElement(plotscales, HMF_PLOTSCALE);
//            plotscale.setAttribute(HMF_PLOTSCALEPORTDATANAME, psit.key());
//            plotscale.setAttribute(HMF_PLOTSCALEDESCRIPTION, val.mUnit);
//            plotscale.setAttribute(HMF_PLOTSCALEVALUE, val.mScale);
//        }
//    }

    // Save animation settings
    QDomElement animationElement = appendDomElement(xmlGuiStuff, "animation");
    mModelObjectAppearance.getAnimationDataPtr()->saveToDomElement(animationElement);

    // Return dom node with appended gui contents
    return xmlGuiStuff;
}


void ModelObject::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsItem::contextMenuEvent(event);

    qDebug() << "ModelObject::contextMenuEvent";

    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
        return;

    QMenu menu;
    this->buildBaseContextMenu(menu, event);
}


//! @brief Defines what happens when mouse starts hovering the object
void ModelObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverEnterEvent(event);
    this->setZValue(HoveredModelobjectZValue);
    this->showPorts(true);
    mpNameText->show();
}


//! @brief Defines what happens when mouse stops hovering the object
void ModelObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    WorkspaceObject::hoverLeaveEvent(event);
    this->setZValue(ModelobjectZValue);
    this->showPorts(false);
    // OK now lets hide the name text if text should be hidden or if icon is hidden
    // (if icon is hidden we are likely a hidden signal component)
    if ((!mNameTextVisible && !mNameTextAlwaysVisible) || !mpIcon->isVisible() )
    {
        mpNameText->hide();
    }
}


//! @brief Defines what happens if a mouse key is pressed while hovering an object
void ModelObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpParentContainerObject)
    {
        return;
    }

    // Forward the mouse press event
    WorkspaceObject::mousePressEvent(event);

    // If not locked then check for drag copy
    if (!isLocallyLocked() && (getModelLockLevel() == NotLocked))
    {
        if(event->button() == Qt::RightButton)
        {
            connect(&mDragCopyTimer, SIGNAL(timeout()), this, SLOT(setDragCopying()), Qt::UniqueConnection);
            mDragCopyTimer.setSingleShot(true);
            mDragCopyTimer.start(100);
        }
    }
    qDebug() << "ModelObject::mousePressEvent(), button = " << event->button();
}



void ModelObject::setDragCopying()
{
    mDragCopying = true;
}



void ModelObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

    //qDebug() << "mouseMoveEvent(), button = " << event->button();

    if(mpParentContainerObject && mDragCopying)
    {
        //qDebug() << "Drag copying";
        mpParentContainerObject->deselectAll();
        this->select();
        mpParentContainerObject->copySelected(mpParentContainerObject->getDragCopyStackPtr());

        QMimeData *mimeData = new QMimeData;
        mimeData->setText("HOPSANDRAGCOPY");

        QDrag *drag = new QDrag(mpParentContainerObject->mpModelWidget->getGraphicsView());
        drag->setMimeData(mimeData);
        drag->setPixmap(QIcon(QPixmap(this->mModelObjectAppearance.getIconPath(mIconType, Absolute))).pixmap(40,40));
        drag->setHotSpot(QPoint(20, 20));
        drag->exec(Qt::CopyAction | Qt::MoveAction);

        mDragCopying = false;
    }
}


//! @brief Defines what happens if a mouse key is released while hovering an object
void ModelObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mDragCopyTimer.stop();

    if(mpParentContainerObject == 0)
    {
        return;
    }

    mDragCopying = false;

    qDebug() << "ModelObject mouseReleaseEvent()";

    //! @todo It would be better if this was handled in some other way,  one particular object should not be responsible for registering moves from other object
    // Loop through all selected objects and register changed positions in undo stack
    bool alreadyClearedRedo = false;
    QList<ModelObject *> selectedObjects = mpParentContainerObject->getSelectedModelObjectPtrs();
    QList<ModelObject *>::iterator it;
    for(it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
    {
        if(((*it)->mPreviousPos != (*it)->pos()) && (event->button() == Qt::LeftButton))
        {
            emit objectMoved();
            // This check makes sure that only one undo post is created when moving several objects at once
            if(!alreadyClearedRedo)
            {
                if(mpParentContainerObject->getSelectedModelObjectPtrs().size() > 1)
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost(UNDO_MOVEDMULTIPLE);
                }
                else
                {
                    mpParentContainerObject->getUndoStackPtr()->newPost();
                }
                mpParentContainerObject->mpModelWidget->hasChanged();
                alreadyClearedRedo = true;
            }
            mpParentContainerObject->getUndoStackPtr()->registerMovedObject((*it)->mPreviousPos, (*it)->pos(), (*it)->getName());
        }
    }

    //Open the context menu (this will force it to open on mouse release regardless of operation system - not verified in Linux yet)
    if(event->button() == Qt::RightButton)
    {
        QGraphicsSceneContextMenuEvent *test = new QGraphicsSceneContextMenuEvent(QGraphicsSceneContextMenuEvent::ContextMenu);
        test->setScenePos(event->scenePos());
        test->setScreenPos(event->screenPos());
        // This ugly hack avoids the context menue in the graphicsview from showing up after this event is processed (is only an issue on windows appearantly)
        mpParentContainerObject->mpModelWidget->mpGraphicsView->setIgnoreNextContextMenuEvent();
        this->contextMenuEvent(test);
    }

    WorkspaceObject::mouseReleaseEvent(event);
}


QAction *ModelObject::buildBaseContextMenu(QMenu &rMenu, QGraphicsSceneContextMenuEvent* pEvent)
{
    bool allowFullEditing = (!isLocallyLocked() && (getModelLockLevel() == NotLocked));
    bool allowLimitedEditing = (!isLocallyLocked() && (getModelLockLevel() <= LimitedLock));
    rMenu.addSeparator();

    QAction *pShowNameAction = rMenu.addAction(tr("Always show name"));
    QAction *pExportComponentParam = 0;//rMenu.addAction(tr("Export Component Parameters"));
    QAction *pRotateRightAction=0, *pRotateLeftAction=0, *pFlipVerticalAction=0, *pFlipHorizontalAction=0;
    QAction *pLockedAction=0;
    if (type() != ScopeComponentType)
    {
        pRotateRightAction = rMenu.addAction(tr("Rotate Clockwise (Ctrl+R)"));
        pRotateLeftAction = rMenu.addAction(tr("Rotate Counter-Clockwise (Ctrl+E)"));
        pFlipVerticalAction = rMenu.addAction(tr("Flip Vertically (Ctrl+D)"));
        pFlipHorizontalAction = rMenu.addAction(tr("Flip Horizontally (Ctrl+F)"));
        pRotateRightAction->setEnabled(allowFullEditing);
        pRotateLeftAction->setEnabled(allowFullEditing);
        pFlipVerticalAction->setEnabled(allowFullEditing);
        pFlipHorizontalAction->setEnabled(allowFullEditing);
    }

    //QStringList replacements = this->getAppearanceData()->getReplacementObjects();
    QStringList replacements = gpLibraryHandler->getReplacements(this->getTypeName());
    qDebug() << "Replacements = " << replacements;
    QList<QAction *> replaceActionList;
    if(!replacements.isEmpty())
    {
        QMenu *replaceMenu = rMenu.addMenu(tr("Replace component"));
        for(int i=0; i<replacements.size(); ++i)
        {
            if(gpLibraryHandler->getModelObjectAppearancePtr(replacements.at(i)))
            {
                QAction *replaceAction = replaceMenu->addAction(gpLibraryHandler->getModelObjectAppearancePtr(replacements.at(i))->getDisplayName());
                replaceActionList.append(replaceAction);
            }
        }
        replaceMenu->setEnabled(allowFullEditing);
    }
    pShowNameAction->setCheckable(true);
    pShowNameAction->setChecked(mNameTextAlwaysVisible);
    pShowNameAction->setEnabled(allowLimitedEditing);

    pLockedAction = rMenu.addAction("Locked");
    pLockedAction->setCheckable(true);
    pLockedAction->setChecked(mIsLocked);

    rMenu.addSeparator();
    QAction *parameterAction = rMenu.addAction(tr("Properties"));
    QAction *selectedAction = rMenu.exec(pEvent->screenPos());

    // If the selected action is 0 then we abort before comparison, (some disabled actions could be 0 so do not want to compare as they may trigger)
    if (selectedAction == 0)
    {
        return 0;
    }

    // Now select which action was triggered
    if (selectedAction == parameterAction)
    {
        openPropertiesDialog();
    }
    else if (selectedAction == pRotateRightAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->rotate90cw();
    }
    else if (selectedAction == pExportComponentParam)
    {
//        QVector<CoreParameterData> paramDataVector;
//        this->getParameters(paramDataVector);
//        for(int i=0; i<paramDataVector.size(); ++i)
//        {
//            //mvParameterLayoutPtrs.push_back(new ParameterSettingsLayout(paramDataVector[i], mpContainerObject));
//        }
//        QDomDocument doc;

//        QFile File("fitxer");
//        if ( File.open( QIODevice::WriteOnly ) )
//        {
//            QTextStream TextStream(&File);
//            TextStream << doc.toString(paramDataVector) ;
//            File.close();
//        }
    }
    else if (selectedAction == pRotateLeftAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->rotate90ccw();
    }
    else if (selectedAction == pFlipVerticalAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->flipVertical();
    }
    else if (selectedAction == pFlipHorizontalAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        this->flipHorizontal();
    }
    else if (selectedAction == pShowNameAction)
    {
        mpParentContainerObject->getUndoStackPtr()->newPost();
        setNameTextAlwaysVisible(pShowNameAction->isChecked());
    }
    else if(selectedAction == pLockedAction)
    {
        this->setIsLocked(pLockedAction->isChecked());
    }
    else
    {
        for(int i=0; i<replaceActionList.size(); ++i)
        {
            if(selectedAction == replaceActionList.at(i))
            {
                mpParentContainerObject->replaceComponent(this->getName(), replacements.at(i));
                return 0;
            }
        }

        // If non of the above actions were triggered then we get here and should return the one that was
        // The menu may contain entries that were added one level up in the menu construction
        return selectedAction;
    }

    //Return 0 action if any of the above actions were triggered
    return 0;
}



//! @brief Defines what happens when object is selected, deselected or has moved
//! @param change Tells what it is that has changed
QVariant ModelObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(mpParentContainerObject && !mpParentContainerObject->hasModelObject(this->getName()))
        return value;

    WorkspaceObject::itemChange(change, value);   //This must be done BEFORE the snapping code to avoid an event loop. This is because snap uses "moveBy()", which triggers a new itemChange event.

    // Abort if we do not have a parent container object, the code below requires it
    if (!mpParentContainerObject)
        return value;

    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if(this->isSelected())
        {
            mpParentContainerObject->rememberSelectedModelObject(this);
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            connect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
        else
        {
            mpParentContainerObject->forgetSelectedModelObject(this);
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressShiftK()), this, SLOT(flipVertical()));
            disconnect(mpParentContainerObject->mpModelWidget->getGraphicsView(), SIGNAL(keyPressShiftL()), this, SLOT(flipHorizontal()));
        }
    }

    // Snap if objects have moved
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        // Restore position if locked
        if(isLocallyLocked() || (getModelLockLevel()!=NotLocked))
        {
            this->setPos(mPreviousPos);
            return value;
        }

        //! @todo maybe this should be omitted from the workspace object itemChanged() function
        emit objectMoved();  //This signal must be emitted  before the snap code, because it updates the connectors which is used to determine whether or not to snap.


        // Snap component if it only has one connector and is dropped close enough (horizontal or vertical) to adjacent component
        if( mEnableSnap && gpConfig->getSnapping() && !mpParentContainerObject->isCreatingConnector() &&
                (mpParentContainerObject->getSelectedModelObjectPtrs().size() == 1) && (mConnectorPtrs.size() == 1))
        {
            Connector *pFirstConnector = mConnectorPtrs.first();
            const int nl = pFirstConnector->getNumberOfLines();
            const QPointF diff = pFirstConnector->getEndPoint() - pFirstConnector->getStartPoint();
            const bool isFALD = pFirstConnector->isFirstAndLastDiagonal();
            const bool isFOLD = pFirstConnector->isFirstOrLastDiagonal();

            // Vertical snap
            const double dx = qAbs(diff.x());
            if( (nl<4) && !(isFALD && nl==2) && !(isFOLD && nl>1) && (dx<SNAPDISTANCE) && (dx>0.0001) )
            {
                // We turn of snapp here to avoid infinite snapping struggle if ctrl is pressed and component mOldPos is within snap distance
                //! @todo should find a better solution to this snap problem
                mEnableSnap = false;
                if(pFirstConnector->getStartPort()->getParentModelObject() == this)
                {
                    this->moveBy(diff.x(), 0);
                }
                else
                {
                    this->moveBy(-diff.x(), 0);
                }
                mEnableSnap = true;
            }

            // Horizontal snap
            const double dy = qAbs(diff.y());
            if( (nl < 4) && !(isFALD && nl==2) && !(isFOLD && nl>1) && (dy<SNAPDISTANCE) && (dy>0.0001) )
            {
                mEnableSnap = false;
                if(pFirstConnector->getStartPort()->getParentModelObject() == this)
                {
                    this->moveBy(0, diff.y());
                }
                else
                {
                    this->moveBy(0, -diff.y());
                }
                mEnableSnap = true;
            }
        }
    }

    return value;
}


//! @brief Shows or hides the port, depending on the input boolean and whether or not they are connected
//! @details Ports will always be shown if visible is true, but they will not always be hidden when false, this depends on other factors as well
//! @param visible Tells whether the ports shall be shown or hidden
void ModelObject::showPorts(bool visible)
{
    QList<Port*>::iterator it;
    if(visible)
    {
        for (it=mPortListPtrs.begin(); it!=mPortListPtrs.end(); ++it)
        {
            (*it)->show();
        }
    }
    else
    {
        for (it=mPortListPtrs.begin(); it!=mPortListPtrs.end(); ++it)
        {
            // Only hide ports if they are connected, are not supposed to be shown or if the MO icon is already hidden (such as for hidden signal components)
            if ((*it)->isConnected() || !mpParentContainerObject->areSubComponentPortsShown() || !mpIcon->isVisible())
            {
                (*it)->hide();
            }
        }
    }
}


//! @todo try to reuse the code in rotate guiobject,
void ModelObject::rotate(double angle, UndoStatusEnumT undoSettings)
{
    if (!isLocallyLocked() && (getModelLockLevel()==NotLocked))
    {
        mpParentContainerObject->mpModelWidget->hasChanged();

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

        if(undoSettings == Undo)
        {
            mpParentContainerObject->getUndoStackPtr()->registerRotatedObject(this->getName(), angle);
        }

        for(int i=0; i<mConnectorPtrs.size(); ++i)
        {
            mConnectorPtrs.at(i)->drawConnector(true);
        }
    }
}


//! @brief Slot that flips the object vertically
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipHorizontal()
//! @todo Fix name text position when flipping components
void ModelObject::flipVertical(UndoStatusEnumT undoSettings)
{
    if (!isLocallyLocked() && getModelLockLevel()==NotLocked)
    {
        this->flipHorizontal(NoUndo);
        this->rotate(180,NoUndo);
        if(undoSettings == Undo)
        {
            mpParentContainerObject->getUndoStackPtr()->registerVerticalFlip(this->getName());
        }
    }
}


//! @brief Slot that flips the object horizontally
//! @param undoSettings Tells whether or not this shall be registered in undo stack
//! @see flipVertical()
void ModelObject::flipHorizontal(UndoStatusEnumT undoSettings)
{
    if (!isLocallyLocked() && getModelLockLevel()==NotLocked)
    {
        if(mpParentContainerObject)
        {
            mpParentContainerObject->mpModelWidget->hasChanged();
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

        // Toggle isFlipped bool
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

        if(undoSettings == Undo)
        {
            mpParentContainerObject->getUndoStackPtr()->registerHorizontalFlip(this->getName());
        }
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
void ModelObject::hideName(UndoStatusEnumT undoSettings)
{
    // Ignore if set to always show
    if (!mNameTextAlwaysVisible)
    {
        bool previousStatus = mpNameText->isVisible();
        mpNameText->setVisible(false);
        mNameTextVisible = false;
        if(undoSettings == Undo && previousStatus == true)
        {
            mpParentContainerObject->getUndoStackPtr()->registerNameVisibilityChange(this->getName(), false);
        }
    }
}


//! @brief Slots that makes the name text of the object visible
void ModelObject::showName(UndoStatusEnumT undoSettings)
{
    bool previousStatus = mpNameText->isVisible();
    mpNameText->setVisible(true);
    mNameTextVisible = true;
    // Ignore setting undo if tagged as always visible
    //! @todo saving undo info for this is not really necessary
    if (!mNameTextAlwaysVisible)
    {
        if(undoSettings == Undo && previousStatus == false)
        {
            mpParentContainerObject->getUndoStackPtr()->registerNameVisibilityChange(this->getName(), true);
        }
    }
}



//! @brief Virtual dummy function that returns the type name of the object (must be reimplemented by children)
QString ModelObject::getTypeName() const
{
    assert(false);
    return "";
}

//! @brief Returns this modelobjects subtype
//! @todo maybe we should overload this in systems so that the parent can ask itself (useful in root systems)
QString ModelObject::getSubTypeName() const
{
//    if (mpParentContainerObject)
//    {
//        //! @todo should we really sync this info with core, core do not really care right now
//        return mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentSubTypeName(mName);
//    }
//    return QString();
    return mModelObjectAppearance.getSubTypeName();
}

//! @brief Set this modelobjects subtype
//! @todo maybe we should overload this in systems so that the parent can set itself (useful in root systems)
void ModelObject::setSubTypeName(const QString subTypeName)
{
//    if (mpParentContainerObject)
//    {
//        //! @todo should we really sync this info with core, core do not really care right now
//        mpParentContainerObject->getCoreSystemAccessPtr()->setSubComponentSubTypeName(mName, subTypeName);
//    }
    mModelObjectAppearance.setSubTypeName(subTypeName);
}


void ModelObject::deleteMe(UndoStatusEnumT undoSettings)
{
    if (isLocallyLocked() || (getModelLockLevel()!=NotLocked))
        return;

    mpParentContainerObject->deleteModelObject(getName(), undoSettings);
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

const ModelObjectAppearance *ModelObject::getLibraryAppearanceData() const
{
    return gpLibraryHandler->getModelObjectAppearancePtr(getTypeName());
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


const QString &ModelObject::getHelpPicture() const
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

void ModelObject::setNameTextAlwaysVisible(const bool isVisible)
{
    mNameTextAlwaysVisible = isVisible;
    // Update actual visibility depending on current system setting
    if (isVisible)
    {
        showName();
    }
    else if (!isVisible && !mpParentContainerObject->areSubComponentNamesShown())
    {
        hideName();
    }
}

const QString &ModelObject::getHelpText() const
{
    return mModelObjectAppearance.getHelpText();
}

const QStringList &ModelObject::getHelpLinks() const
{
    return mModelObjectAppearance.getHelpLinks();
}

QString ModelObject::getHelpHtmlPath() const
{
    return mModelObjectAppearance.getHelpHtmlPath();
}

//! @brief Constructor for the name text object
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

