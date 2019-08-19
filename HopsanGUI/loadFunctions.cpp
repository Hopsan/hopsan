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
//! @file   loadFunctions.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains functions used when models are loaded from hmf
//!
//$Id$

#include "loadFunctions.h"

#include "global.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "LibraryHandler.h"
#include "UndoStack.h"
#include "MessageHandler.h"
#include "Utilities/GUIUtilities.h"
#include "Configuration.h"

#include <QMap>

//! @brief Loads a Connector from the supplied load data
//! @param[in] rDomElement The DOM element to load from
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Whether or not to register undo for the operation
bool loadConnector(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings)
{
    // -----First read from DOM element-----
    QString startComponentName, endComponentName, startPortName, endPortName;
    bool isDashed;
    QVector<QPointF> pointVector;
    QStringList geometryList;

    // Read core specific stuff
    startComponentName  = rDomElement.attribute(HMF_CONNECTORSTARTCOMPONENTTAG);
    startPortName       = rDomElement.attribute(HMF_CONNECTORSTARTPORTTAG);
    endComponentName    = rDomElement.attribute(HMF_CONNECTORENDCOMPONENTTAG);
    endPortName         = rDomElement.attribute(HMF_CONNECTORENDPORTTAG);
    //qDebug() << "loadConnector: " << startComponentName << " " << startPortName << " " << endComponentName << " " << endPortName;

    //! @todo To upconvert old models these are needed, may not be necessary later
    santizeName(startComponentName);
    santizeName(startPortName);
    santizeName(endComponentName);
    santizeName(endPortName);

    // Read gui specific stuff
    double x,y;
    QDomElement guiData         = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    QDomElement guiCoordinates  = guiData.firstChildElement(HMF_COORDINATES);
    QDomElement coordTag        = guiCoordinates.firstChildElement(HMF_COORDINATETAG);
    while (!coordTag.isNull())
    {
        parseCoordinateTag(coordTag, x, y);
        pointVector.push_back(QPointF(x,y));
        coordTag = coordTag.nextSiblingElement(HMF_COORDINATETAG);
    }
    QDomElement guiGeometries   = guiData.firstChildElement(HMF_GEOMETRIES);
    QDomElement geometryTag     = guiGeometries.firstChildElement(HMF_GEOMETRYTAG);
    while (!geometryTag.isNull())
    {
        geometryList.append(geometryTag.text());
        geometryTag = geometryTag.nextSiblingElement(HMF_GEOMETRYTAG);
    }
    QDomElement styleTag = guiData.firstChildElement(HMF_STYLETAG);
    if(!styleTag.isNull())
    {
        isDashed = (styleTag.text() == "dashed");
    }
    else
    {
        isDashed = false;
    }
    QDomElement colorTag = guiData.firstChildElement(HMF_COLORTAG);
    QColor color =  QColor();
    if(!colorTag.isNull())
    {
        QString rgb = colorTag.text();
        double r, g, b;
        parseRgbString(rgb, r, g, b);
        color.setRgb(r, g, b);
    }

    // -----Now establish the connection-----
    bool success=false;
    Port *startPort = pContainer->getModelObjectPort(startComponentName, startPortName);
    Port *endPort = pContainer->getModelObjectPort(endComponentName, endPortName);

    if (startPort && endPort)
    {
        Connector* pConn = pContainer->createConnector(startPort, endPort, NoUndo);
        if (pConn)
        {
            if(pointVector.isEmpty() && !pConn->isDangling() && !pConn->isBroken())   //Create a diagonal connector if no points were loaded from HMF
            {
                pointVector.push_back(pConn->getStartPort()->boundingRect().center());
                pointVector.push_back(pConn->getEndPort()->boundingRect().center());
                geometryList.clear();
                geometryList.append("diagonal");
            }
            pConn->setPointsAndGeometries(pointVector, geometryList);
            pConn->setDashed(isDashed);
            pConn->refreshConnectorAppearance();
            pConn->setColor(color);

            if(undoSettings == Undo)
            {
                pContainer->getUndoStackPtr()->registerAddedConnector(pConn);
            }
            if (pConn->isConnected())
            {
                success = true;
            }
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    gpMessageHandler->collectHopsanCoreMessages();;

    if (!success)
    {
        const QString str("Failed to load connector between: "+startComponentName+"->"+startPortName+" and "+endComponentName+"->"+endPortName+" in system: "+pContainer->getName());
        gpMessageHandler->addErrorMessage(str, "FailedLoadConnector");
    }

    return success;
}



//! @brief Loads a component Parameter Value (no metadata is loaded/expected, for ordinary components)
//! @todo Make undo settings work or remove it
void loadParameterValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT undoSettings)
{
    Q_UNUSED(undoSettings)
    QString parameterName;
    QString parameterValue;
    QString parameterType;

    parameterName = rDomElement.attribute(HMF_NAMETAG);
    parameterName.replace("::","#"); //!< @todo this can be removed in the future (after 0.7), used to upconvert old hmf file formats
    parameterValue = rDomElement.attribute(HMF_VALUETAG);
    parameterType = rDomElement.attribute(HMF_TYPE);
    parameterType = rDomElement.attribute(HMF_TYPENAME, parameterType); //!< @deprecated load old typename

    //! @todo Remove this check in the future when models should have been updated
    QStringList existingNames = pObject->getParameterNames();
    // This code makes sure we can load old parameters from before they became ports
    bool tryingToAddColonColonValue = false;
    if(!existingNames.contains(parameterName))
    {
        if (!parameterName.contains("#"))
        {
            parameterName = parameterName+"#Value";
            tryingToAddColonColonValue = true;
        }
    }

    //Use the setParameter method that maps to System parameter
    if(!parameterName.startsWith("noname_subport:") && !existingNames.contains(parameterName))
    {
        if (parameterName.contains("#"))
        {
            gpMessageHandler->addWarningMessage("Startvalue mismatch:  "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+".  Startvalue ignored.", "startvaluemismatch");
        }
        else
        {
            gpMessageHandler->addWarningMessage("Parameter mismatch: "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+".  Parameter ignored.", "parametermismatch");
        }
        return;
    }

    //! @todo this is also a compatibility hack, to prevent startvalues in dynamic parameter ports from overwriting previously renamed but loaded parameter values, when the parameter has the same name as its port. This prevents data from overwriting if loaded last
    if (!tryingToAddColonColonValue && parameterName.contains("::Value"))
    {
        QStringList theotherparameternames;
        QDomElement parameterDOM = rDomElement.parentNode().firstChildElement(HMF_PARAMETERTAG);
        while (!parameterDOM.isNull())
        {
            theotherparameternames.append(parameterDOM.attribute(HMF_NAMETAG));
            parameterDOM = parameterDOM.nextSiblingElement(HMF_PARAMETERTAG);
        }

        QString portparname = parameterName.split("::")[0];
        if (theotherparameternames.contains(portparname))
        {
            gpMessageHandler->addWarningMessage("Prevented overwriting:  "+portparname+"  with:  "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+"   (This is a good thing and probably nothing to worry about)");
        }
        else
        {
            pObject->setParameterValue(parameterName, parameterValue, true);
        }
        return;
    }

    if(!(pObject->getTypeName() == MODELICATYPENAME &&
       (parameterName == "ports" || parameterName == "parameters" || parameterName == "defaults")))
    {
        pObject->setParameterValue(parameterName, parameterValue, true);
    }
}


//! @deprecated This StartValue load code is only kept for upconverting old files, we should keep it here until we have some other way of upconverting old formats
void loadStartValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT /*undoSettings*/)
{
    QString portName = rDomElement.attribute("portname");
    QString variable = rDomElement.attribute("variable");
    QString startValue = rDomElement.attribute("value");

//    bool isDbl;
//    //Assumes that if it is convertible to a double it is a plain value otherwise it is assumed to be mapped to a System parameter
//    double value = rData.startValue.toDouble(&isDbl);
//    if(isDbl)
//    {
//        pObject->setStartValue(rData.portName, rData.variable, value);
//    }
//    else
    {
        //Use the setStartValue method that maps to System parameter
        pObject->setStartValue(portName, variable, startValue);
    }
}


//! @brief Loads a ModelObject from the supplied load data
//! @param[in] rData The ModelObjectLoadData to load from
//! @param[in] pLibrary a pointer to the library widget which holds appearance data
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Whether or not to register undo for the operation
ModelObject* loadModelObject(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings)
{
    //Read core specific data
    QString type = rDomElement.attribute(HMF_TYPENAME);
    QString subtype = rDomElement.attribute(HMF_SUBTYPENAME);
    QString name = rDomElement.attribute(HMF_NAMETAG);
    bool locked = parseAttributeBool(rDomElement, HMF_LOCKEDTAG, false);
    bool disabled = parseAttributeBool(rDomElement, HMF_DISABLEDTAG, false);


    //Read gui specific data
    double posX, posY, target_rotation;
    bool isFlipped;

    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    parsePoseTag(guiData.firstChildElement(HMF_POSETAG), posX, posY, target_rotation, isFlipped);
    target_rotation = normDeg360(target_rotation); //Make sure target rotation between 0 and 359.999

    int nameTextPos = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("position").toInt();
    bool nameTextVisible = parseAttributeBool(guiData.firstChildElement(HMF_NAMETEXTTAG), "visible", false);

    const SharedModelObjectAppearanceT pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(type, subtype);
    if (pAppearanceData && gpLibraryHandler->getEntry(type).state != Disabled)
    {
        ModelObjectAppearance appearanceData = *pAppearanceData; //Make a copy

        QDomElement animationElement = guiData.firstChildElement(HMF_ANIMATION);
        if(!animationElement.isNull() && appearanceData.getTypeName() == type)
        {
            appearanceData.getAnimationDataPtr()->readFromDomElement(animationElement, appearanceData.getBasePath());
        }

        appearanceData.setDisplayName(name);

        NameVisibilityEnumT nameStatus;
        if(pContainer->areSubComponentNamesShown())
        {
            nameStatus = NameVisible;
        }
        else
        {
            nameStatus = NameNotVisible;
        }

        ModelObject* pObj = pContainer->addModelObject(&appearanceData, QPointF(posX, posY), 0, Deselected, nameStatus, undoSettings);
        pObj->setNameTextPos(nameTextPos);
        pObj->setNameTextAlwaysVisible(nameTextVisible);
        pObj->setSubTypeName(subtype); //!< @todo is this really needed

        //First set flip (before rotate, Important!)
        //! @todo For now If flipped than we need to rotate in wrong direction also, saving saves flipped rotation angle i think but changing save and load would cause old models to load incorrectly
        if (isFlipped)
        {
            pObj->flipHorizontal(undoSettings);
            pObj->rotate(-target_rotation, undoSettings); //This assumes object created with initial rotation 0 in addGuiModelObject above
        }
        else
        {
            pObj->rotate(target_rotation, undoSettings); //This assumes object created with initial rotation 0 in addGuiModelObject above
        }

        //Read system specific core and gui data
        if (rDomElement.tagName() == HMF_SYSTEMTAG)
        {
            //Check if we should load an embedded or external system
            QString externalFilePath = appearanceData.getBasePath()+"/"+appearanceData.getHmfFile();
            if(!QFile::exists(externalFilePath) || appearanceData.getSubTypeName().isEmpty())
            {
                externalFilePath = rDomElement.attribute(HMF_EXTERNALPATHTAG);
            }
            if (externalFilePath.isEmpty())
            {
                //Load embedded system
                pObj->getAppearanceData()->setBasePath(pContainer->getAppearanceData()->getBasePath()); // Set the basepath for relative icon paths
                pObj->loadFromDomElement(rDomElement);
            }
            else
            {
                //Now read the external file to change appearance and populate the system
                QFileInfo extFileInfo(externalFilePath);
                if(!extFileInfo.exists())
                {
                    if (!extFileInfo.isAbsolute() && !pContainer->getModelPath().isEmpty())
                    {
                        externalFilePath = pContainer->getModelPath() + "/" + externalFilePath;
                        // Update extFileInfo with full path
                        extFileInfo.setFile(externalFilePath);
                    }
                    else
                    {
                        externalFilePath = gpDesktopHandler->getExecPath()+"/"+externalFilePath;
                        extFileInfo.setFile(externalFilePath);
                    }
                }

                QFile externalModelFile(externalFilePath);
                if (externalModelFile.exists())
                {
                    QDomDocument domDocument;
                    QDomElement externalRoot = loadXMLDomDocument(externalModelFile, domDocument, HMF_ROOTTAG);
                    QDomElement externalSystemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
                    //! @todo set the modefile info, maybe we should have built in helpfunction for loading directly from file in System
                    pObj->setModelFileInfo(externalModelFile, rDomElement.attribute(HMF_EXTERNALPATHTAG));
                    pObj->getAppearanceData()->setBasePath(extFileInfo.absolutePath()); // Set the basepath for relative icon paths
                    pObj->loadFromDomElement(externalSystemRoot);
                    //! @todo this code is duplicated with the one in system->loadfromdomelement (external code) that code will never run, as this will take care of it. When we have embedded subsystems will will need to fix this

                    //Overwrite any loaded external name with the one that was stored in the main file from which we are loading
                    if (!name.isEmpty())
                    {
                        pObj->setName(name);
                    }

                    // Now load all overwritten parameters
                    QDomElement xmlParameters = rDomElement.firstChildElement(HMF_PARAMETERS);
                    QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
                    while (!xmlParameter.isNull())
                    {
                        ContainerObject* pCont = dynamic_cast<ContainerObject*>(pObj);
                        loadSystemParameter(xmlParameter, false, "1000", pCont);
                        xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("The file: %1 does not exist!").arg(externalFilePath));
                    return nullptr;
                }
            }
        }
        else
        {
            //! @todo maybe this parameter load code and the one for external systems above could be the same
            //Load parameter values
            QDomElement xmlParameters = rDomElement.firstChildElement(HMF_PARAMETERS);
            QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
            while (!xmlParameter.isNull())
            {
                loadParameterValue(xmlParameter, pObj, NoUndo);
                xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
            }

            // Load any custom signal quantities
            QDomElement portTag = rDomElement.firstChildElement(HMF_PORTSTAG).firstChildElement(HMF_PORTTAG);
            while (!portTag.isNull())
            {
                QString q = portTag.attribute("signalquantity");
                if (!q.isEmpty())
                {
                    pObj->setModifyableSignalQuantity(portTag.attribute("name")+"#Value", q);
                }
                portTag = portTag.nextSiblingElement(HMF_PORTTAG);
            }


            // Load component specific override port data, and dynamic parameter port positions
            QDomElement cafMoStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG).firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT);
            if (!cafMoStuff.isNull())
            {
                //We read all model object appearance data in the hmf to get the ports
                //!  @todo This work right now maybe not in the future if more data will be there for ordinary components
                pObj->getAppearanceData()->readFromDomElement(cafMoStuff);
                pObj->refreshDisplayName(); //Need to refresh display name if read appearance data contained an incorrect name

                // Now refresh only those ports that were new
                QDomElement dom_port = cafMoStuff.firstChildElement("ports").firstChildElement("port");
                while (!dom_port.isNull())
                {
                    // Create or refresh modified ports
                    QString portName = dom_port.attribute("name");
                    pObj->createRefreshExternalPort(portName);

                    Port *pPort = pObj->getPort(portName);
                    if (pPort)
                    {
                        pPort->setModified(true); //Tag as modified since we loaded override data
                    }

                    dom_port = dom_port.nextSiblingElement("port");
                }
            }

            // Load any custom set parameter scales
            QDomElement paramscale = rDomElement.firstChildElement(HMF_HOPSANGUITAG).firstChildElement(HMF_PARAMETERSCALES).firstChildElement(HMF_PARAMETERSCALE);
            while (!paramscale.isNull())
            {
                QString quantity = paramscale.attribute(HMF_PARAMETERSCALEQUANTITY);
                QString unit = paramscale.attribute(HMF_PARAMETERSCALEUNIT);
                QString scale = paramscale.attribute(HMF_PARAMETERSCALESCALE);

                UnitConverter confus, us;
                gpConfig->getUnitScale(quantity, unit, confus);

                if (!confus.isEmpty() && (confus.mScale != scale))
                {
                    gpMessageHandler->addWarningMessage("Missmatch in custom unit scale "+quantity+":"+unit+" updating to new scale (likely model from older version of Hopsan)");
                    us = confus;
                }
                else
                {
                    us = UnitConverter(quantity,
                                       unit,
                                       scale,
                                       paramscale.attribute(HMF_PARAMETERSCALEOFFSET));
                }

                pObj->registerCustomParameterUnitScale(paramscale.attribute(HMF_PARAMETERSCALEPARAMNAME), us);
                //! @todo The actual custom value is ignored here, since only scale can be registered, custom values are not a part of parameters yet so it is difficult to support loading custom values, (rescaling will happen automatically from SI unit value loaded by core)
                paramscale = paramscale.nextSiblingElement(HMF_PARAMETERSCALE);
            }

            // Load any custom variable plot settings
            QDomElement plotsetting = rDomElement.firstChildElement(HMF_HOPSANGUITAG).firstChildElement(HMF_VARIABLEPLOTSETTINGS).firstChildElement(HMF_VARIABLEPLOTSETTING);
            while (!plotsetting.isNull())
            {
                QString name = plotsetting.attribute("name");
                if (!name.isEmpty())
                {
                    if(plotsetting.hasAttribute("invert"))
                    {
                        pObj->setInvertPlotVariable(name, parseAttributeBool(plotsetting, "invert", false));
                    }
                    if(plotsetting.hasAttribute("label"))
                    {
                        pObj->setVariablePlotLabel(name, plotsetting.attribute("label"));
                    }
                }
                plotsetting = plotsetting.nextSiblingElement(HMF_VARIABLEPLOTSETTING);
            }
        }

        //Set disabled state
        pObj->setDisabled(disabled);

        //Set lock state (must be done last, or other settings may not be applied)
        pObj->setIsLocked(locked);

        return pObj;
    }
    else
    {
        gpMessageHandler->addErrorMessage(QString("In loadModelObject Some error happend, pAppearanceData == 0, for type: %1 subtype: %2").arg(type).arg(subtype));
        return 0;
    }
}




//! @brief Loads a containerport object from a xml dom element
ModelObject* loadContainerPortObject(QDomElement &rDomElement, ContainerObject* pContainer, UndoStatusEnumT undoSettings)
{
    //! @todo this does not feel right should try to avoid it maybe
    rDomElement.setAttribute(HMF_TYPENAME, HOPSANGUICONTAINERPORTTYPENAME); //Set the typename for the gui, or overwrite if anything was actually given in the HMF file (should not be)
    return loadModelObject(rDomElement, pContainer, undoSettings); //We use the loadGUIModelObject function as it does what is needed
}

//! @brief Loads a SystemParameter from the supplied load data
//! @param[in] rDomElement The SystemParameter DOM element to load from
//! @param[in] doAdd Should loading add the system parameter
//! @param[in] hmfVersion The HopsanModelFile version used during loading
//! @param[in] pContainer The Container Object to load into
void loadSystemParameter(QDomElement &rDomElement, bool doAdd, const QString hmfVersion, ContainerObject* pContainer)
{
    QString name = rDomElement.attribute(HMF_NAMETAG);
    QString value = rDomElement.attribute(HMF_VALUETAG);
    QString type = rDomElement.attribute(HMF_TYPE);
    type = rDomElement.attribute(HMF_TYPENAME, type); //!< @deprecated load old typename
    QString quantityORunit = rDomElement.attribute(HMF_QUANTITY, rDomElement.attribute(HMF_UNIT));
    QString description = rDomElement.attribute(HMF_DESCRIPTIONTAG);

    if( (hmfVersion <= "0.3") && type.isEmpty())     //Version check, types did not exist in 0.3 and bellow (everything was double)
    {
        type = "double";
    }

    // Core will take care of deciding about quantity or unit, leave unit empty
    CoreParameterData paramData(name, value, type, quantityORunit, "", description);
    if (doAdd)
    {
        pContainer->setOrAddParameter(paramData, true);
    }
    else
    {
        pContainer->setParameter(paramData, true);
    }
}


//! @todo We should remove Plot from the name as this is supposed to be usable for more then plotting only
void loadPlotAlias(QDomElement &rDomElement, ContainerObject* pContainer)
{
    QString aliasname, fullName;

    //! @todo not hardcoded attrnames
    //! @todo what about type
    //! @todo actually Core should load this data
    QString type = rDomElement.attribute("type", "UnknownAliasType");
    aliasname = rDomElement.attribute("name", "UnknownAliasName");
    fullName = rDomElement.firstChildElement("fullname").text();

    // try the old format
    //! @todo don't know if this works, but likely only atlas cares
    if (type=="UnknownAliasType")
    {
        aliasname = rDomElement.attribute("alias");
        QString componentName = rDomElement.attribute("component");
        QString portName = rDomElement.attribute("port");
        QString dataName = rDomElement.attribute("data");

        //! @todo we need a help function to build and to split a full variable name
        fullName = componentName+"#"+portName+"#"+dataName;
    }

    //! @todo maybe should only be in core
    pContainer->setVariableAlias(fullName,aliasname);
    //! @todo instead of bool return the unique changed alias should be returned
    //! @todo what about parameter alias or other types
}


//! @todo this function should not be needed, figure out the stupid stuff below then code this function away
TextBoxWidget *loadTextBoxWidget(QDomElement &rDomElement, ContainerObject *pContainer, UndoStatusEnumT undoSettings)
{
    TextBoxWidget *pWidget = pContainer->addTextBoxWidget(QPointF(1,1), undoSettings);
    pWidget->loadFromDomElement(rDomElement);

    pWidget->setSelected(true);     //!< @todo Stupid!
    pWidget->setSelected(false);    //For some reason this is needed...

    return pWidget;
}
