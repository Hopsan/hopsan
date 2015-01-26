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
//! @file   GUIComponent.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI class representing Components
//!
//$Id$

//Qt includes
#include <QDrag>

//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "Dialogs/ComponentPropertiesDialog.h"
#include "Dialogs/ComponentPropertiesDialog3.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "ModelicaLibrary.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/ModelWidget.h"
#include "MessageHandler.h"
#include "LibraryHandler.h"


Component::Component(QPointF position, double rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    // Create the object in core, and get its default core name
    mName = mpParentContainerObject->getCoreSystemAccessPtr()->createComponent(mModelObjectAppearance.getTypeName(), mModelObjectAppearance.getDisplayName());
    refreshDisplayName(); //Make sure name window is correct size for center positioning

    // Sets the ports
    createPorts();

    // Component shall be hidden when toggle signals is deactivated, if it is of signal type and has no power ports (= is a sensor)
    //! @todo not hardcoded Sensor check for typename
    if(getTypeCQS() == "S")
    {
        connect(mpParentContainerObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisible(bool)));
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
    mpParentContainerObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
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
    if(!mpParentContainerObject->mpModelWidget->isEditingEnabled())
        return;

    QGraphicsWidget::mouseDoubleClickEvent(event);
    openPropertiesDialog();
}


//! @brief Returns a string with the component type.
QString Component::getTypeName() const
{
    return mModelObjectAppearance.getTypeName();
}

//! @brief Returns the component CQS type
QString Component::getTypeCQS()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(this->getName());
}


//! @brief Set a parameter value to be mapped to a System parameter
bool Component::setParameterValue(QString name, QString value, bool force)
{
    bool retval =  mpParentContainerObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), name, value, force);

    //Special code for setting parameters to Modelica components. Should maybe be somewhere else.
    if(this->getTypeName() == MODELICATYPENAME && name == "model")
    {
        if(gpModelicaLibrary->hasModel(value))
        {
            QStringList alreadyExistingParameters = this->getParameterNames();  //Remember existing parameters for later

            QStringList ports;
            QStringList parameterNames;
            QStringList parameterDefaults;
            QString cafFilePath, icon;
            QStringList portPosNames;
            QList<QList<double> > portPos;
            ModelicaModel model = gpModelicaLibrary->getModel(value);
            QList<ModelicaVariable> variables = model.getVariables();
            foreach(const ModelicaVariable &var, variables)
            {
                foreach(const QString &connector, gpModelicaLibrary->getConnectorNames())
                {
                    if(var.getType() == connector)
                    {
                        ports.append(var.getName());
                    }
                }
            }

            QList<ModelicaVariable> parameters = model.getParameters();
            foreach(const ModelicaVariable &par, parameters)
            {

                parameterNames.append(par.getName());
                parameterDefaults.append(QString::number(par.getDefaultValue()));
            }

            QString annotations = model.getAnnotations();
            if(!annotations.isEmpty())
            {
                cafFilePath = annotations.section("cafFile",1,1).section("\"",1,1).section("\"",0,0);
                icon = annotations.section("hopsanIcon",1,1).section("\"",1,1).section("\"",0,0);
                int nPorts=annotations.count("portPos");
                for(int i=0; i<nPorts; ++i)
                {
                    QStringList portPosStr = annotations.section("portPos",i+1,i+1).section("\"",1,1).section("\"",0,0).split(",");
                    if(portPosStr.size() != 4)
                        continue;
                    portPosNames.append(portPosStr.first());
                    portPos.append(QList<double>() << portPosStr[1].toDouble() << portPosStr[2].toDouble() << portPosStr[3].toDouble());
                }
            }

            this->setParameterValue("ports", ports.join(","));
            this->setParameterValue("parameters", parameterNames.join(","));
            this->setParameterValue("defaults", parameterDefaults.join(","));

            qDebug() << "Setting icon: " << icon;

            mModelObjectAppearance.setIconPath(icon, UserGraphics, Absolute);
            for(int i=0; i<portPosNames.size(); ++i)
            {
                PortAppearance *pPortAppearance = &mModelObjectAppearance.getPortAppearanceMap().find(portPosNames[i]).value();
                pPortAppearance->x = portPos[i][0];
                pPortAppearance->y = portPos[i][1];
                pPortAppearance->rot = portPos[i][2];

            }
            if(!cafFilePath.isEmpty())
            {
                QFile cafFile(cafFilePath);
                if (!cafFile.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    gpMessageHandler->addErrorMessage("Failed to open file or not a text file: " + cafFilePath);
                }
                else
                {
                    QDomDocument domDocument;
                    QDomElement cafRoot = loadXMLDomDocument(cafFile, domDocument, CAF_ROOT);
                    cafFile.close();
                    if(!cafRoot.isNull())
                    {
                        //Read appearance data from the caf xml file
                        QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearance objects from same file, also not hardcode tagnames
                        mModelObjectAppearance.setBasePath(QFileInfo(cafFile).absolutePath()+"/");
                        mModelObjectAppearance.readFromDomElement(xmlModelObjectAppearance);
                        mModelObjectAppearance.setTypeName(MODELICATYPENAME);
                        mModelObjectAppearance.cacheIcons();
                    }
                }
            }
            this->refreshAppearance();

            //Set default parameter values (for new parameters only)
            foreach(const ModelicaVariable &par, parameters)
            {
                if(!alreadyExistingParameters.contains(par.getName()))
                    this->setParameterValue(par.getName(), QString::number(par.getDefaultValue()));
            }
        }
    }
    else if(this->getTypeName() == MODELICATYPENAME && name == "ports" && !value.isEmpty())
    {
        QStringList portNames = value.split(",");

        //Remove port appearance if port does not exist in list of new ports
        PortAppearanceMapT::Iterator it;
        for(it=mModelObjectAppearance.getPortAppearanceMap().begin(); it!=mModelObjectAppearance.getPortAppearanceMap().end(); ++it)
        {
            if(!portNames.contains(it.key()))
            {
                QString key = it.key();
                mModelObjectAppearance.erasePortAppearance(key);
                this->removeExternalPort(key);
                it=mModelObjectAppearance.getPortAppearanceMap().begin();
            }
        }

        //Add new port appearance for new ports that do not already exist
        for(int i=0; i<portNames.size(); ++i)
        {
            QString portName = portNames.at(i);
            if(!mModelObjectAppearance.getPortAppearance(portName))
            {
                PortAppearance *pPortAppearance = new PortAppearance();
                pPortAppearance->selectPortIcon("Q", "PowerPort", "NodeModelica");
                mModelObjectAppearance.addPortAppearance(portName, pPortAppearance);
                mModelObjectAppearance.getPortAppearance(portName)->x = (double)rand() / (double)RAND_MAX;
                mModelObjectAppearance.getPortAppearance(portName)->y = (double)rand() / (double)RAND_MAX;
                mModelObjectAppearance.getPortAppearance(portName)->rot = 0;
                this->createRefreshExternalPort(portName);
            }

            //! @todo Remove no longer existing ports (and disconnect them if they are connected)
        }
        this->refreshAppearance();
    }

    return retval;
}

//! @brief Set a start value to be mapped to a System parameter
//! @deprecated
bool Component::setStartValue(QString portName, QString /*variable*/, QString sysParName)
{
    QString dataName;
    dataName = portName + QString("::Value");
    return mpParentContainerObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), dataName, sysParName);
}


//! @brief Slot that opens the parameter dialog for the component
void Component::openPropertiesDialog()
{
    //ComponentPropertiesDialog dialog(this, gpMainWindow);
    ComponentPropertiesDialog3 dialog(this, mpDialogParentWidget);

    if(getTypeName() != QString(MODELICATYPENAME)+" NOT" && getTypeName() != "CppComponent") //! @todo DEBUG
    {
        connect(this, SIGNAL(objectDeleted()), &dialog, SLOT(reject()));
        //! @todo should we have delete on close
        dialog.setModal(false);
        dialog.show();
        dialog.exec();
    }
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
    return Type;
}

QString Component::getHmfTagName() const
{
    return HMF_COMPONENTTAG;
}


//! @brief Decide if component should be visible or not
//! @details We do not want to run the Qt setVisble function because the item should not be hidden, only the visual parts should be hidden, mouse events should still be processed
//! @param[in] visible True or False
void Component::setVisible(bool visible)
{
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
        mPortListPtrs.at(i)->showIfNotConnected(mpParentContainerObject->areSubComponentPortsShown());
    }
}


//! @brief Save component coredata to XML Dom Element
//! @param[in] rDomElement The dom element to save to
void Component::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement);

    //Save parameters (also core related)
    QDomElement xmlParameters = appendDomElement(rDomElement, HMF_PARAMETERS);
    QVector<CoreParameterData> paramDataVec;
    getParameters(paramDataVec);
    for(int i=0; i<paramDataVec.size(); ++i)
    {
        QDomElement xmlParam = appendDomElement(xmlParameters, HMF_PARAMETERTAG);
        xmlParam.setAttribute(HMF_NAMETAG, paramDataVec[i].mName);
        xmlParam.setAttribute(HMF_VALUETAG, paramDataVec[i].mValue);
        xmlParam.setAttribute(HMF_TYPE, paramDataVec[i].mType);
        xmlParam.setAttribute(HMF_UNIT, paramDataVec[i].mUnit);

        /*if(this->isParameterMappedToSystemParameter(*pit))
        {
            xmlParam.setAttribute(HMF_SYSTEMPARAMETERTAG, this->getSystemParameterKey(*pit));
        }*/
    }

    if(contents==FullModel)
    {
        //Implementation of Feature #698 - Save nodetype in HMF
        QDomElement xmlPorts = appendDomElement(rDomElement, HMF_PORTSTAG);
        QList<Port*>::Iterator it;
        for (it=mPortListPtrs.begin(); it!=mPortListPtrs.end(); ++it)
        {
            QDomElement xmlPort = appendDomElement(xmlPorts, "port");
            xmlPort.setAttribute(HMF_NAMETAG, (*it)->getName());
            xmlPort.setAttribute("nodetype", (*it)->getNodeType());
        }
    }
}

QDomElement Component::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    ModelObject::saveGuiDataToDomElement(rDomElement);
    QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

    // Save those ports that have changed appearance (position)
    QStringList ports;
    for (int i=0; i<mPortListPtrs.size(); ++i)
    {
        const PortAppearance *app = mPortListPtrs[i]->getPortAppearance();
        if (app->mEnabled && app->mPoseModified) //!< @todo need to rethink this condition
        {
            ports.append(mPortListPtrs[i]->getName());
        }
    }
    mModelObjectAppearance.saveSpecificPortsToDomElement(xmlApp, ports);

    return rDomElement;
}


void ScopeComponent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpParentContainerObject->mpModelWidget->isEditingEnabled())
        return;

    QGraphicsWidget::mouseDoubleClickEvent(event);

    // If this is a sink component that has plot data, plot it instead of showing the dialog
    // Not very nice code, but a nice feature...
    if( !mpParentContainerObject->getLogDataHandler()->isEmpty() && !mpParentContainerObject->isCreatingConnector() &&
            (getPort("in")->isConnected() || getPort("in_right")->isConnected()) )
    {
        // If we don't have valid plotwindow then create one
        if (mpPlotWindow.isNull())
        {
            mpPlotWindow = gpPlotHandler->createNewUniquePlotWindow(getName());

            for(int i=0; i<getPort("in")->getConnectedPorts().size(); ++i)
            {
                QString fullName = makeConcatName(getPort("in")->getConnectedPorts().at(i)->getParentModelObjectName(),
                                                  getPort("in")->getConnectedPorts().at(i)->getName(),"Value");
                getParentContainerObject()->getLogDataHandler()->plotVariable(mpPlotWindow, fullName, -1, 0);
            }
            for(int i=0; i<getPort("in_right")->getConnectedPorts().size(); ++i)
            {
                QString fullName = makeConcatName(getPort("in_right")->getConnectedPorts().at(i)->getParentModelObjectName(),
                                                  getPort("in_right")->getConnectedPorts().at(i)->getName(),"Value");
                getParentContainerObject()->getLogDataHandler()->plotVariable(mpPlotWindow, fullName, -1, 1);
            }

            if(this->getPort("in_bottom")->isConnected() && mpPlotWindow && mpPlotWindow->getCurrentPlotTab())
            {
                QString fullName = makeConcatName(getPort("in_bottom")->getConnectedPorts().at(0)->getParentModelObjectName(),
                                                  getPort("in_bottom")->getConnectedPorts().at(0)->getName(),"Value");
                mpPlotWindow->getCurrentPlotTab()->setCustomXVectorForAll(getParentContainerObject()->getLogDataHandler()->getHopsanVariable(fullName, -1));
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



ScopeComponent::ScopeComponent(QPointF position, double rotation, ModelObjectAppearance *pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : Component(position, rotation, pAppearanceData, pParentContainer, startSelected, gfxType)
{
    // Nothing special
}

int ScopeComponent::type() const
{
    return Type;
}
