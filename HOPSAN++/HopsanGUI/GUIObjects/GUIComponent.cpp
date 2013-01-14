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

#include <QDrag>

#include "Configuration.h"
#include "GraphicsView.h"
#include "GUIPort.h"
#include "PlotWindow.h"
#include "Dialogs/ComponentPropertiesDialog.h"
#include "Dialogs/ComponentPropertiesDialog2.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/ProjectTabWidget.h"
#include "PlotTab.h"

#include "PlotHandler.h"


Component::Component(QPointF position, qreal rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
    : ModelObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    //Set the hmf save tag name
    mHmfTagName = HMF_COMPONENTTAG;

    //Create the object in core, and get its default core name
    mName = mpParentContainerObject->getCoreSystemAccessPtr()->createComponent(mModelObjectAppearance.getTypeName(), mModelObjectAppearance.getDisplayName());
    refreshDisplayName(); //Make sure name window is correct size for center positioning

    //Sets the ports
    createPorts();

    //Component shall be hidden when toggle signals is deactivated, if it is of signal type and has no power ports (= is a sensor)
    if(this->getTypeCQS() == "S" && !this->hasPowerPorts())
    {
        connect(mpParentContainerObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisible(bool)));
    }

    //! @todo maybe set default param values for ALL ModelObjects
    QStringList defaultParameterNames = getParameterNames();
    for(int i=0; i<defaultParameterNames.size(); ++i)
    {
        mDefaultParameterValues.insert(defaultParameterNames.at(i), getParameterValue(defaultParameterNames.at(i)));
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


//! Event when double clicking on component icon.
//! @todo Fix the sink component so it works with this
void Component::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpParentContainerObject->mpParentProjectTab->isEditingEnabled())
        return;

    QGraphicsWidget::mouseDoubleClickEvent(event);

    // If this is a sink component that has plot data, plot it instead of showing the dialog
    if( (this->getTypeName() == "SignalSink") &&
        !mpParentContainerObject->getLogDataHandler()->isEmpty() &&
        !mpParentContainerObject->isCreatingConnector() )   //Not very nice code, but a nice feature...
    {
        QString plotName = "";
        for(int i=0; i<getPort("in")->getConnectedPorts().size(); ++i)
        {
            QString fullName = makeConcatName(getPort("in")->getConnectedPorts().at(i)->getGuiModelObjectName(),
                                              getPort("in")->getConnectedPorts().at(i)->getPortName(),"Value");
            plotName = getParentContainerObject()->getLogDataHandler()->plotVariable(plotName, fullName, -1, 0);
        }
        for(int i=0; i<getPort("in_right")->getConnectedPorts().size(); ++i)
        {
            QString fullName = makeConcatName(getPort("in_right")->getConnectedPorts().at(i)->getGuiModelObjectName(),
                                              getPort("in_right")->getConnectedPorts().at(i)->getPortName(),"Value");
            plotName = getParentContainerObject()->getLogDataHandler()->plotVariable(plotName, fullName, -1, 1);
        }

        //! @todo try to solve this without needding the actual plotwindow pointer
        PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow(plotName);
        if(this->getPort("in_bottom")->isConnected() && pPlotWindow)
        {
            VariableDescription varDesc;
            varDesc.mComponentName = getPort("in_bottom")->getConnectedPorts().at(0)->mpParentGuiModelObject->getName();
            varDesc.mPortName = getPort("in_bottom")->getConnectedPorts().at(0)->getPortName();
            varDesc.mDataName = "Value";
            varDesc.mDataUnit = gConfig.getDefaultUnit(varDesc.mDataName);

            pPlotWindow->changeXVector(mpParentContainerObject->getLogDataHandler()->getPlotDataValues(varDesc.getFullName(), -1), varDesc);
        }

        // No plot window was opened, so it is a non-connected sink - open properties instead
        if(!pPlotWindow)
        {
            openPropertiesDialog();
        }
    }
    else
    {
        openPropertiesDialog();
    }
}


//! Returns a string with the component type.
QString Component::getTypeName()
{
    return mModelObjectAppearance.getTypeName();
}

QString Component::getTypeCQS()
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(this->getName());
}


//! @brief Set a parameter value to be mapped to a System parameter
bool Component::setParameterValue(QString name, QString value, bool force)
{
    return mpParentContainerObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), name, value, force);
}

//! @brief Set a start value to be mapped to a System parameter
//! @deprecated
bool Component::setStartValue(QString portName, QString /*variable*/, QString sysParName)
{
    QString dataName;
    dataName = portName + QString("::Value");
    return mpParentContainerObject->getCoreSystemAccessPtr()->setParameterValue(this->getName(), dataName, sysParName);
}


////! @brief Set a start value to be mapped to a System parameter
//QString Component::getStartValueTxt(QString portName, QString variable)
//{
//    QVector<QString> vVariable, vSysParName, vUnit;
//    this->getPort(portName)->getStartValueDataNamesValuesAndUnits(vVariable, vSysParName, vUnit);
//    int idx = vVariable.indexOf(variable);
//    if(idx < 0)
//        return "";
//    else
//        return vSysParName[idx];
//}


//! @brief Slot that opens the parameter dialog for the component
void Component::openPropertiesDialog()
{
    ComponentPropertiesDialog dialog(this, gpMainWindow);
    dialog.setModal(false);
    //ComponentPropertiesDialog2 dialog(this, gpMainWindow);
    dialog.show();
    dialog.exec();
}


//! @brief Help function to create ports in the component when it is created
//! @todo duplicate implementation with createExternalPort, maybe remove this and only use the other, slighlty lower speed though but probably better
void Component::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (not really necessary in guicomponents)
    QString cqsType = mpParentContainerObject->getCoreSystemAccessPtr()->getSubComponentTypeCQS(getName());
    PortAppearanceMapT::iterator i;
    for (i = mModelObjectAppearance.getPortAppearanceMap().begin(); i != mModelObjectAppearance.getPortAppearanceMap().end(); ++i)
    {
        QString nodeType = mpParentContainerObject->getCoreSystemAccessPtr()->getNodeType(this->getName(), i.key());
        QString portType = mpParentContainerObject->getCoreSystemAccessPtr()->getPortType(this->getName(), i.key());
        i.value().selectPortIcon(cqsType, portType, nodeType);

        qreal x = i.value().x * this->boundingRect().width();
        qreal y = i.value().y * this->boundingRect().height();

        Port *pNewPort = new Port(i.key(), x, y, &(i.value()), this);
        mPortListPtrs.append(pNewPort);
    }
}



int Component::type() const
{
    return Type;
}



void Component::setVisible(bool visible)
{
    this->mpIcon->setVisible(visible);
    for(int i=0; i<mPortListPtrs.size(); ++i)
    {
        mPortListPtrs.at(i)->showIfNotConnected(!mpParentContainerObject->areSubComponentPortsHidden());
    }
}


//! @brief Save component coredata to XML Dom Element
//! @param[in] rDomElement The dom element to save to
void Component::saveCoreDataToDomElement(QDomElement &rDomElement)
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

        /*if(this->isParameterMappedToSystemParameter(*pit))
        {
            xmlParam.setAttribute(HMF_SYSTEMPARAMETERTAG, this->getSystemParameterKey(*pit));
        }*/
    }
}

QDomElement Component::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    ModelObject::saveGuiDataToDomElement(rDomElement);
    QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);
    mModelObjectAppearance.saveSpecificPortsToDomElement(xmlApp, mActiveDynamicParameterPortNames);

    return rDomElement;
}
