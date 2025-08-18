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
//! @file   GUIComponent.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI class representing Components
//!
//$Id$

//Qt includes
#include <QDrag>
#include <QFileDialog>
#include <QGraphicsSceneContextMenuEvent>

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "Dialogs/ComponentPropertiesDialog3.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/ModelWidget.h"
#include "LibraryHandler.h"


Component::Component(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, SystemObject *pParentSystem, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentSystem, pParentSystem)
{
    // Create the object in core, and get its default core name
    mName = mpParentSystemObject->getCoreSystemAccessPtr()->createComponent(mModelObjectAppearance.getTypeName(), mModelObjectAppearance.getDisplayName(), mModelObjectAppearance.getSubTypeName());
    refreshDisplayName(); //Make sure name window is correct size for center positioning

    // Sets the ports
    createPorts();

    // Component shall be hidden when toggle signals is deactivated, if it is of signal type and has no power ports (= is a sensor)
    //! @todo not hardcoded Sensor check for typename
    if(getTypeCQS() == "S")
    {
        connect(mpParentSystemObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisible(bool)));
    }

    //! @todo maybe set default param values for ALL ModelObjects
    QStringList defaultParameterNames = getParameterNames();
    for(int i=0; i<defaultParameterNames.size(); ++i)
    {
        mDefaultParameterValues.insert(defaultParameterNames.at(i), getParameterValue(defaultParameterNames.at(i)));
    }


    if(pAppearanceData)
    {
        QMapIterator<QString,QString> it(pAppearanceData->getOverridedDefaultParameters());
        while(it.hasNext())
        {
            it.next();
            mDefaultParameterValues.insert(it.key(),it.value());
            setParameterValue(it.key(),it.value(),true);
        }
    }
}

void Component::deleteInHopsanCore()
{
    //Remove in core
    //! @todo maybe change to delte instead of remove with dodelete yes
    mpParentSystemObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
}


//! @brief Returns whether or not the component has at least one power port
bool Component::hasPowerPorts()
{
    bool retval = false;
    for(int i=0; i<mPortListPtrs.size(); ++i)
    {
        if(mPortListPtrs.at(i)->getNodeType() != "NodeSignal")
        {
            retval = true;
        }
    }
    return retval;
}


//! @brief Event when double clicking on component icon.
void Component::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    if(!mpParentSystemObject->mpModelWidget->getGraphicsView()->isCtrlKeyPressed())
    {
        openPropertiesDialog();
    }
}


//! @brief Returns a string with the component type.
QString Component::getTypeName() const
{
    return mModelObjectAppearance.getTypeName();
}

//! @brief Returns the component CQS type
QString Component::getTypeCQS() const
{
    return mpParentSystemObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(this->getName());
}


//! @brief Set a parameter value to be mapped to a System parameter
bool Component::setParameterValue(QString name, QString value, bool force)
{
    if(((this->getTypeName() == "FMIWrapper" || this->getTypeName() == "FMIWrapperQ") && (name == "path" || name == "portspecs")) ||
       (this->getTypeName() == "DcpComponent" && name == "variables")) {
        //Remove old ports
        QList<Port*> ports = this->getPortListPtrs();
        for(const auto port : ports) {
            mConnectionsBeforeReconfigure.insert(port->getName(), port->getAttachedConnectorPtrs());
            this->removeExternalPort(port->getName(), true);
        }
        this->getAppearanceData()->getPortAppearanceMap().clear();
    }

    bool retval =  mpParentSystemObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), name, value, force);

    if(((this->getTypeName() == "FMIWrapper" || this->getTypeName() == "FMIWrapperQ") && (name == "path" || name == "portspecs")) ||
       (this->getTypeName() == "DcpComponent" && name == "variables")) {
        //Get lists of input and output ports from core component
        QStringList visibleOutputs = getParameterValue("visibleOutputs").split(",");
        QStringList inputs, outputs, powerPorts;
        for(const auto &port: mpParentSystemObject->getCoreSystemAccessPtr()->getPortNames(this->getName())) {
            QString type = mpParentSystemObject->getCoreSystemAccessPtr()->getPortType(this->getName(), port);
            SharedPortAppearanceT app(new PortAppearance());
            if("ReadPortType" == type) {
                inputs << port;
            }
            else if(this->getTypeName() == "DcpComponent" && "WritePortType" == type) {
                outputs << port;
            }
            else if("WritePortType" == type && visibleOutputs.contains(port)) {
                outputs << port;
            }
        }

        //Auto-place input ports
        for(int i=0; i<inputs.size(); ++i) {
            SharedPortAppearanceT app(new PortAppearance());
            app->x = 0.0;
            app->y = double(1+i)/double(1+inputs.size());
            app->rot = 180;
            app->mEnabled = true;
            this->getAppearanceData()->addPortAppearance(inputs[i],app);
            this->createRefreshExternalPort(inputs[i]);
        }

        //Auto-place output ports
        for(int i=0; i<outputs.size(); ++i) {
            SharedPortAppearanceT app(new PortAppearance());
            app->x = 1.0;
            app->y = double(1+i)/double(1+outputs.size());
            app->rot = 0;
            app->mEnabled = true;
            this->getAppearanceData()->addPortAppearance(outputs[i],app);
            this->createRefreshExternalPort(outputs[i]);
        }

        QStringList portSpecs = getParameterValue("portspecs").remove(' ').split(";");
        for(int i=0; i<portSpecs.size(); ++i) {
            if(!portSpecs[i].isEmpty()) {
                SharedPortAppearanceT app(new PortAppearance());
                app->y = 0.0;
                app->x = double(1+i)/double(1+portSpecs.size());
                app->rot = 270;
                app->mEnabled = true;
                QString portName;
                if(portSpecs[i].contains(':')) {
                    portName = portSpecs[i].split(',').at(0).split(':').at(1);
                }
                else {
                    portName = "P"+QString::number(i+1);
                }
                this->getAppearanceData()->addPortAppearance(portName, app);
                this->createRefreshExternalPort(portName);
            }
        }

        //Reconnect dangling connectors after reconfiguration
        QMapIterator<QString,QVector<Connector*> > it(mConnectionsBeforeReconfigure);
        while(it.hasNext()) {
            it.next();
            for(const auto &con : it.value()) {
                if(con != nullptr && con->getStartPort() == nullptr && con->getEndPort() != nullptr) {
                    auto port = this->getPort(it.key());
                    if(port != nullptr) {
                        con->setStartPort(this->getPort(it.key())); //The start port was disconnected
                    }
                }
                if(con != nullptr && con->getEndPort() == nullptr && con->getStartPort() != nullptr) {
                    auto port = this->getPort(it.key());
                    if(port != nullptr) {
                        con->setEndPort(this->getPort(it.key()));   //The end port was disconneted
                    }
                }

                con->finishCreation(false);  //Re-establish connection
            }
        }


        //Adjust icon scale
        this->getAppearanceData()->setIconScale(qMax(qMax(inputs.size(),outputs.size())/3.0,1.0), UserGraphics);

        mpParentSystemObject->renameModelObject(this->getName(), QFileInfo(getParameterValue("path")).baseName());

        //Refresh appearance
        this->refreshAppearance();

        QStringList defaultParameterNames = getParameterNames();
        for(int i=0; i<defaultParameterNames.size(); ++i) {
            mDefaultParameterValues.insert(defaultParameterNames.at(i), getParameterValue(defaultParameterNames.at(i)));
        }
    }

    return retval;
}

//! @brief Set a start value to be mapped to a System parameter
//! @deprecated
bool Component::setStartValue(QString portName, QString variable, QString sysParName)
{
    Q_UNUSED(variable)
    QString dataName;
    dataName = portName + QString("::Value");
    return mpParentSystemObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), dataName, sysParName);
}

void Component::loadParameterValuesFromFile(QString parameterFile)
{
    if(parameterFile.isEmpty()) {
        parameterFile = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Parameter File"),
                                                     gpConfig->getStringSetting(cfg::dir::loadmodel),
                                                     tr("Hopsan Parameter Files (*.hpf *.xml)"));
    }

    if(!parameterFile.isEmpty()) {
        size_t numChanged = 0;
        auto pCore = mpParentSystemObject->getCoreSystemAccessPtr();
        if (pCore) {
            numChanged = pCore->loadParameterFile(getName(), parameterFile);
        }
        if (numChanged > 0) {
            mpParentSystemObject->mpModelWidget->hasChanged();
        }
        gpConfig->setStringSetting(cfg::dir::loadmodel,  QFileInfo(parameterFile).absolutePath());
    }
#if QT_VERSION_MAJOR < 5
    QMetaObject::invokeMethod(mpParentSystemObject,"checkMessages");
#else
    emit mpParentSystemObject->checkMessages();
#endif
}



//! @brief Help function to create ports in the component when it is created
void Component::createPorts()
{
    QList<QString> names = mModelObjectAppearance.getPortAppearanceMap().keys();
    for (int i=0; i<names.size(); ++i)
    {
        createRefreshExternalPort(names[i]);
    }
}



int Component::type() const
{
    return ComponentType;
}

QString Component::getHmfTagName() const
{
    return hmf::component;
}


//! @brief Decide if component should be visible or not
//! @details We do not want to run the Qt setVisble function because the item should not be hidden, only the visual parts should be hidden, mouse events should still be processed
//! @param[in] visible True or False
void Component::setVisible(bool visible)
{
    visible |= mAlwaysVisible;

    // Hide show icon
    mpIcon->setVisible(visible);

    // The name text should always be hidden with the component
    if (visible == false)
    {
        mpNameText->setVisible(false);
    }
    // But when we turn it back on, it should only become visible if it is supposed to be visible
    else if (mNameTextVisible)
    {
        mpNameText->setVisible(true);
    }

    for(int i=0; i<mPortListPtrs.size(); ++i)
    {
        mPortListPtrs.at(i)->showIfNotConnected(mpParentSystemObject->areSubComponentPortsShown());
    }
}

void Component::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
        return;

    //! @todo might move this to a ModelObjectWithParamters class ,to avoid duplication in SystemContainer (or put it in ModelObject with a condition)
    QMenu menu;
    QAction *saveParameterValuesAction = menu.addAction(tr("Save parameter values to file"));
    QAction *loadParameterValuesAction = menu.addAction(tr("Load parameter values from file"));

    QAction *pAction = this->buildBaseContextMenu(menu, event);
    if (pAction == saveParameterValuesAction)
    {
        this->saveParameterValuesToFile();
    }
    else if (pAction == loadParameterValuesAction)
    {
        this->loadParameterValuesFromFile();
    }
}


//! @brief Save component coredata to XML Dom Element
//! @param[in] rDomElement The dom element to save to
void Component::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement, contents);

    //Save parameters (also core related)
    QDomElement xmlParameters = appendDomElement(rDomElement, hmf::parameters);
    QVector<CoreParameterData> paramDataVec;
    getParameters(paramDataVec);
    for(int i=0; i<paramDataVec.size(); ++i)
    {
        QDomElement xmlParam = appendDomElement(xmlParameters, hmf::parameter::root);
        xmlParam.setAttribute(hmf::name, paramDataVec[i].mName);
        xmlParam.setAttribute(hmf::value, paramDataVec[i].mValue);
        xmlParam.setAttribute(hmf::type, paramDataVec[i].mType);
        xmlParam.setAttribute(hmf::unit, paramDataVec[i].mUnit);
        xmlParam.setAttribute(hmf::internal, bool2str(paramDataVec[i].mInternal));

        /*if(this->isParameterMappedToSystemParameter(*pit))
        {
            xmlParam.setAttribute(hmf::systemparameter, this->getSystemParameterKey(*pit));
        }*/
    }

    if(contents==FullModel)
    {
        //Implementation of Feature #698 - Save nodetype in HMF
        QDomElement xmlPorts = appendDomElement(rDomElement, hmf::ports);

        // Note! we cant loop local ports since non-enabled ports will not exist at all in the GUI
        //       Instead we ask Core for all "variameters" and extract port info from there
        QVector<CoreVariameterDescription> descs;
        getVariameterDescriptions(descs);

        QString currentPort;
        for (CoreVariameterDescription &desc : descs)
        {
            if (desc.mPortName != currentPort)
            {
                currentPort = desc.mPortName;
                QDomElement xmlPort = appendDomElement(xmlPorts, "port");
                xmlPort.setAttribute(hmf::name, desc.mPortName);
                xmlPort.setAttribute("nodetype", desc.mNodeType);
                Port *pPort = this->getPort(desc.mPortName);
                if (pPort)
                {
                    xmlPort.setAttribute("porttype", pPort->getPortType());
                }
                if (desc.mNodeType == "NodeSignal")
                {
                    QString q = this->getModifyableSignalQuantity(desc.mPortName+"#Value");
                    if (!q.isEmpty())
                    {
                        xmlPort.setAttribute("signalquantity", q);
                    }
                }
            }
        }
    }
}

QDomElement Component::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    ModelObject::saveGuiDataToDomElement(rDomElement);
    QDomElement guiStuff = rDomElement.firstChildElement(hmf::hopsangui);
    QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

    SharedModelObjectAppearanceT pLibraryAppearance = gpLibraryHandler->getModelObjectAppearancePtr(getTypeName(), getSubTypeName());

    // Save those ports that have changed appearance (position)
    QStringList ports;
    for (Port *pPort : mPortListPtrs)
    {
        bool differentPortEnabled=false;
        const SharedPortAppearanceT pPortAppearance = pPort->getPortAppearance();
        // Check if "port enabled" different from default state in library
        if (pLibraryAppearance)
        {
            SharedPortAppearanceT pLibraryPortAppearance = pLibraryAppearance->getPortAppearance(pPort->getName());
            if (pLibraryPortAppearance && pPortAppearance)
            {
                differentPortEnabled = pLibraryPortAppearance->mEnabled != pPortAppearance->mEnabled;
            }
        }
        if (differentPortEnabled || pPortAppearance->mPoseModified)
        {
            ports.append(pPort->getName());
        }
    }
    mModelObjectAppearance.saveSpecificPortsToDomElement(xmlApp, ports);
    return rDomElement;
}


void ScopeComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);

    if(mpParentSystemObject->mpModelWidget->getGraphicsView()->isCtrlKeyPressed())
        return;

    // If this is a sink component that has plot data, plot it instead of showing the dialog
    // Not very nice code, but a nice feature...
    if( !mpParentSystemObject->getLogDataHandler()->isEmpty() && !mpParentSystemObject->isCreatingConnector() &&
            (getPort("in")->isConnected() || getPort("in_right")->isConnected()) )
    {
        // If we don't have valid plotwindow then create one
        if (mpPlotWindow.isNull())
        {
            mpPlotWindow = gpPlotHandler->createNewUniquePlotWindow(getName());

            for(int i=0; i<getPort("in")->getConnectedPorts().size(); ++i)
            {
                QString fullName = makeFullVariableName(getParentSystemNameHieararchy(), getPort("in")->getConnectedPorts().at(i)->getParentModelObjectName(),
                                                        getPort("in")->getConnectedPorts().at(i)->getName(),"Value");
                getParentSystemObject()->getLogDataHandler()->plotVariable(mpPlotWindow, fullName, -1, 0);
            }
            for(int i=0; i<getPort("in_right")->getConnectedPorts().size(); ++i)
            {
                QString fullName = makeFullVariableName(getParentSystemNameHieararchy(), getPort("in_right")->getConnectedPorts().at(i)->getParentModelObjectName(),
                                                        getPort("in_right")->getConnectedPorts().at(i)->getName(),"Value");
                getParentSystemObject()->getLogDataHandler()->plotVariable(mpPlotWindow, fullName, -1, 1);
            }

            if(this->getPort("in_bottom")->isConnected() && mpPlotWindow && mpPlotWindow->getCurrentPlotTab())
            {
                QString fullName = makeFullVariableName(getParentSystemNameHieararchy(), getPort("in_bottom")->getConnectedPorts().at(0)->getParentModelObjectName(),
                                                        getPort("in_bottom")->getConnectedPorts().at(0)->getName(),"Value");
                mpPlotWindow->getCurrentPlotTab()->setCustomXVectorForAll(getParentSystemObject()->getLogDataHandler()->getVectorVariable(fullName, -1));
            }
        }
        mpPlotWindow->showNormal();
    }

    // No plot window was opened, so it is a non-connected sink - open properties instead
    if(mpPlotWindow.isNull())
    {
        openPropertiesDialog();
    }
}

void ScopeComponent::rotate(double angle, UndoStatusEnumT undoSettings)
{
    Q_UNUSED(angle)
    Q_UNUSED(undoSettings)
    // Overloaded to do nothing
}

void ScopeComponent::flipVertical(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    // Overloaded to do nothing
}

void ScopeComponent::flipHorizontal(UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    // Overloaded to do nothing
}



ScopeComponent::ScopeComponent(QPointF position, double rotation, ModelObjectAppearance *pAppearanceData, SystemObject *pParentSystem, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : Component(position, rotation, pAppearanceData, pParentSystem, startSelected, gfxType)
{
    // Nothing special
}

int ScopeComponent::type() const
{
    return ScopeComponentType;
}
