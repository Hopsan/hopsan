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
#include "Dialogs/ComponentPropertiesDialog3.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "Widgets/ProjectTabWidget.h"
#include "PlotTab.h"

#include "PlotHandler.h"


Component::Component(QPointF position, qreal rotation, ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
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


//! @brief Event when double clicking on component icon.
void Component::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!mpParentContainerObject->mpParentProjectTab->isEditingEnabled())
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
    //ComponentPropertiesDialog dialog(this, gpMainWindow);
    ComponentPropertiesDialog3 dialog(this, mpDialogParentWidget);
    connect(this, SIGNAL(objectDeleted()), &dialog, SLOT(reject()));
    //! @todo should we have delete on close
    dialog.setModal(false);
    dialog.show();
    dialog.exec();
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
    if(!mpParentContainerObject->mpParentProjectTab->isEditingEnabled())
        return;

    QGraphicsWidget::mouseDoubleClickEvent(event);

    // If this is a sink component that has plot data, plot it instead of showing the dialog
    // Not very nice code, but a nice feature...
    if( !mpParentContainerObject->getLogDataHandler()->isEmpty() && !mpParentContainerObject->isCreatingConnector() )
    {
        // If we dont have valid plotwindow then create one
        if (mpPlotWindow.isNull())
        {
            mpPlotWindow = gpPlotHandler->createNewOrReplacePlotwindow(getName());

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

            if(this->getPort("in_bottom")->isConnected() && mpPlotWindow)
            {
                QString fullName = makeConcatName(getPort("in_bottom")->getConnectedPorts().at(0)->getParentModelObjectName(),
                                                  getPort("in_bottom")->getConnectedPorts().at(0)->getName(),"Value");
                mpPlotWindow->setCustomXVector(getParentContainerObject()->getLogDataHandler()->getPlotData(fullName, -1));
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

void ScopeComponent::rotate(qreal /*angle*/, UndoStatusEnumT /*undoSettings*/)
{
    // Overloaded to do nothing
}

void ScopeComponent::flipVertical(UndoStatusEnumT /*undoSettings*/)
{
    // Overloaded to do nothing
}

void ScopeComponent::flipHorizontal(UndoStatusEnumT /*undoSettings*/)
{
    // Overloaded to do nothing
}



ScopeComponent::ScopeComponent(QPointF position, qreal rotation, ModelObjectAppearance *pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : Component(position, rotation, pAppearanceData, pParentContainer, startSelected, gfxType)
{
    // Nothing special
}

int ScopeComponent::type() const
{
    return Type;
}
