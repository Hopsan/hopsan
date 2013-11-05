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
//! @file   loadFunctions.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains functions used when models are loaded from hmf
//!
//$Id$

#include "loadFunctions.h"

#include "global.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "LibraryHandler.h"
#include "UndoStack.h"
#include "Widgets/HcomWidget.h"
#include "Utilities/GUIUtilities.h"

#include <QMap>

//! @brief Loads a Connector from the supplied load data
//! @param[in] rDomElement The DOM element to load from
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
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
    qreal x,y;
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

    Connector* pConn = pContainer->createConnector(startPort, endPort, NoUndo);
    if (pConn)
    {
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

    gpTerminalWidget->checkMessages();

    if (!success)
    {
        const QString str("Failed to load connector between: "+startComponentName+"->"+startPortName+" and "+endComponentName+"->"+endPortName+" in system: "+pContainer->getName());
        gpTerminalWidget->mpConsole->printErrorMessage(str, "FailedLoadConnector");
    }

    return success;
}



//! @brief xml version
//! @todo Make undo settings work or remove it
//! @todo Make loadParameterValue and loadSystemParameter same function
void loadParameterValue(QDomElement &rDomElement, ModelObject* pObject, UndoStatusEnumT /*undoSettings*/)
{
    QString parameterName;
    QString parameterValue;
    QString parameterType;

    parameterName = rDomElement.attribute(HMF_NAMETAG);
    parameterName.replace("::","#"); //!< @todo this can be removed in the future (after 0.7), used to upconvert old hmf file formats
    parameterValue = rDomElement.attribute(HMF_VALUETAG);
    parameterType = rDomElement.attribute(HMF_TYPE);
    parameterType = rDomElement.attribute(HMF_TYPENAME, parameterType); //!< @deprecated load old typename

    //! @todo Remove this check in teh future when models should have been updated
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

    //Use the setParameter method that mapps to System parameter
    if(!parameterName.startsWith("noname_subport:") && !existingNames.contains(parameterName))
    {
        if (parameterName.contains("#"))
        {
            gpTerminalWidget->mpConsole->printWarningMessage("Startvalue mismatch:  "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+".  Startvalue ignored.", "startvaluemismatch");
        }
        else
        {
            gpTerminalWidget->mpConsole->printWarningMessage("Parameter mismatch: "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+".  Parameter ignored.", "parametermismatch");
        }
        return;
    }

    //! @todo this is also a compatibility hack, to prevent startvalues in dynamic parameter ports from overwriting previously renamed but loaded paremter values, when the parmeter has the same name as its port. This prevents data from overwriting if loaded last
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
            gpTerminalWidget->mpConsole->printWarningMessage("Prevented overwriting:  "+portparname+"  with:  "+parameterName+" = "+parameterValue+"  in Component:  "+pObject->getName()+"   (This is a good thing and probably nothing to worry about)");
        }
        else
        {
            pObject->setParameterValue(parameterName, parameterValue, true);
        }
        return;
    }

    pObject->setParameterValue(parameterName, parameterValue, true);
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
        //Use the setStartValue method that mapps to System parameter
        pObject->setStartValue(portName, variable, startValue);
    }
}


//! @brief Loads a ModelObject from the supplied load data
//! @param[in] rData The ModelObjectLoadData to load from
//! @param[in] pLibrary a pointer to the library widget which holds appearance data
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
ModelObject* loadModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, ContainerObject* pContainer, UndoStatusEnumT undoSettings)
{
    //Read core specific data
    QString type = rDomElement.attribute(HMF_TYPENAME);
    QString subtype = rDomElement.attribute(HMF_SUBTYPENAME);
    QString name = rDomElement.attribute(HMF_NAMETAG);

    //Read gui specific data
    qreal posX, posY, target_rotation;
    bool isFlipped;

    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    parsePoseTag(guiData.firstChildElement(HMF_POSETAG), posX, posY, target_rotation, isFlipped);
    target_rotation = normDeg360(target_rotation); //Make sure target rotation between 0 and 359.999

    int nameTextPos = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("position").toInt();
    int nameTextVisible = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("visible").toInt(); //should be bool, +0.5 to roound to int on truncation

    ModelObjectAppearance *pAppearanceData = gpLibraryHandler->getEntry(type, subtype).pAppearance;
    if (pAppearanceData != 0)
    {
        QDomElement animationElement = guiData.firstChildElement("animation");
        if(!animationElement.isNull() && pAppearanceData->getTypeName() == type)
        {
            pAppearanceData->getAnimationDataPtr()->readFromDomElement(animationElement, pAppearanceData->getBasePath(), true);
        }

        ModelObjectAppearance appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setDisplayName(name);

        NameVisibilityEnumT nameStatus;
        if(nameTextVisible)
        {
            nameStatus = NameVisible;
        }
        else
        {
            nameStatus = NameNotVisible;
        }

        ModelObject* pObj = pContainer->addModelObject(&appearanceData, QPointF(posX, posY), 0, Deselected, nameStatus, undoSettings);
        pObj->setNameTextPos(nameTextPos);
        pObj->setSubTypeName(subtype); //!< @todo is this really needed

        //First set flip (before rotate, Important!)
        //! @todo For now If flipped than we need to rotate in wrong direction also, saving saves flipped rotation angle i think but changing save and load would couse old models to load incorrectly
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
            //Check if we should load an embeded or external system
            QString externalfilepath = rDomElement.attribute(HMF_EXTERNALPATHTAG);
            if (externalfilepath.isEmpty())
            {
                //Load embeded system
                pObj->getAppearanceData()->setBasePath(pContainer->getAppearanceData()->getBasePath()); // Set the basepath for relative icon paths
                pObj->loadFromDomElement(rDomElement);
            }
            else
            {
                //Now read the external file to change appearance and populate the system
                //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
                QString path = pContainer->getModelFileInfo().absolutePath() + "/" + externalfilepath;
                QFile file(path);
                if (!(file.exists()))
                {
                    qDebug() << "file: " << path << " does not exist";
                }
                //! @todo need error handling if file does not exist
                QDomDocument domDocument;
                QDomElement externalRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
                QDomElement externalSystemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
                //! @todo set the modefile info, maybe we should have built in helpfunction for loading directly from file in System
                pObj->setModelFileInfo(file);
                pObj->loadFromDomElement(externalSystemRoot);
                //! @todo this code is duplicated with the one in system->loadfromdomelement (external code) that code will never run, as this will take care of it. When we have embeded subsystems will will need to fix this

                //Overwrite any loaded external name with the one that was stored in the main file from which we are loading
                if (!name.isEmpty())
                {
                    pObj->setName(name);
                }

                // Now load all overwriten parameters
                QDomElement xmlParameters = rDomElement.firstChildElement(HMF_PARAMETERS);
                QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
                while (!xmlParameter.isNull())
                {
                    ContainerObject* pCont = dynamic_cast<ContainerObject*>(pObj);
                    loadSystemParameter(xmlParameter, "1000", pCont);
                    xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
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


            // Load component specific override port data, and dynamic parameter port positions
            QDomElement cafMoStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG).firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT);
            if (!cafMoStuff.isNull())
            {
                //We read all model object appearance data in the hmf to get the ports
                //!  @todo This work right now maybe not in the future if more data will be there for ordinary components
                pObj->getAppearanceData()->readFromDomElement(cafMoStuff);
                pObj->refreshDisplayName(); //Need to refresh display name if read appearance data contained an incorrect name

                // Now refresh only thos ports that were new
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

            // Load any custom set plot scales
            QDomElement plotscale = rDomElement.firstChildElement(HMF_HOPSANGUITAG).firstChildElement(HMF_PLOTSCALES).firstChildElement(HMF_PLOTSCALE);
            while (!plotscale.isNull())
            {
                pObj->registerCustomPlotUnitOrScale(plotscale.attribute(HMF_PLOTSCALEPORTDATANAME), plotscale.attribute(HMF_PLOTSCALEDESCRIPTION), plotscale.attribute(HMF_PLOTSCALEVALUE));
                plotscale = plotscale.nextSiblingElement(HMF_PLOTSCALE);
            }
        }

        return pObj;
    }
    else
    {
        qDebug() << "loadGUIObj Some error happend pAppearanceData == 0";
        //! @todo Some error message
        return 0;
    }
}




//! @brief Loads a containerport object from a xml dom element
ModelObject* loadContainerPortObject(QDomElement &rDomElement, LibraryWidget* pLibrary, ContainerObject* pContainer, UndoStatusEnumT undoSettings)
{
    //! @todo this does not feel right should try to avoid it maybe
    rDomElement.setAttribute(HMF_TYPENAME, HOPSANGUICONTAINERPORTTYPENAME); //Set the typename for the gui, or overwrite if anything was actaully given in the HMF file (should not be)
    return loadModelObject(rDomElement, pLibrary, pContainer, undoSettings); //We use the loadGUIModelObject function as it does what is needed
}

//! @brief Loads a SystemParameter from the supplied load data
//! @param[in] rDomElement The SystemParameter DOM element to load from
//! @param[in] pContainer The Container Object to load into
void loadSystemParameter(QDomElement &rDomElement, const QString hmfVersion, ContainerObject* pContainer)
{
    QString name = rDomElement.attribute(HMF_NAMETAG);
    QString value = rDomElement.attribute(HMF_VALUETAG);
    QString type = rDomElement.attribute(HMF_TYPE);
    type = rDomElement.attribute(HMF_TYPENAME, type); //!< @deprecated load old typename

    if( (hmfVersion <= "0.3") && type.isEmpty())     //Version check, types did not exist in 0.3 and bellow (everything was double)
    {
        type = "double";
    }

    CoreParameterData paramData(name,value, type);
    pContainer->setOrAddParameter(paramData, true);
}

//! @brief Loads a FavouriteParameter from the supplied load data
//! @param[in] rDomElement The FavoriteVariableLoadData DOM element to load from
//! @param[in] pContainer The Container Object to load into (Must be a system)
void loadFavoriteVariable(QDomElement &rDomElement, ContainerObject* pContainer)
{
    QString componentName = rDomElement.attribute("componentname");
    QString portName = rDomElement.attribute("portname");
    QString dataName = rDomElement.attribute("dataname");
    QString dataUnit = rDomElement.attribute("dataunit");

    dynamic_cast<SystemContainer *>(pContainer)->getLogDataHandler()->setFavoriteVariable(componentName, portName, dataName, dataUnit);
}

//! @todo We should remove Plot from the name as this is suposed to be useable for more then plotting only
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
    //! @todo dont know if this works, but likely only atlas cares
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
    QString comp,port,var;
    splitConcatName(fullName, comp,port,var);
    pContainer->setVariableAlias(comp,port,var,aliasname);
    //! @todo instead of bool return the uniqe changed alias should be returned
    //! @todo what about parameter alias or other types
}


TextBoxWidget *loadTextBoxWidget(QDomElement &rDomElement, ContainerObject *pContainer, UndoStatusEnumT undoSettings)
{
    QString text;
    QFont font;
    QColor color;
    QString linestyle;
    bool lineVisible;
    QPointF point;
    qreal width, height, linewidth;

    //Read gui specific stuff
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);

    QDomElement textobjectTag = guiData.firstChildElement("textobject");
    text = textobjectTag.attribute("text");
    font.fromString(textobjectTag.attribute("font"));
    color.setNamedColor(textobjectTag.attribute("fontcolor"));

    QDomElement poseTag = guiData.firstChildElement(HMF_POSETAG);
    QPointF tempPoint;
    tempPoint.setX(poseTag.attribute("x").toDouble());
    tempPoint.setY(poseTag.attribute("y").toDouble());
    point = tempPoint.toPoint();

    QDomElement sizeTag = guiData.firstChildElement("size");
    width = sizeTag.attribute("width").toDouble();
    height = sizeTag.attribute("height").toDouble();

    QDomElement lineTag = guiData.firstChildElement("line");
    lineVisible = lineTag.attribute("visible").toInt();
    linewidth = lineTag.attribute("width").toDouble();
    linestyle = lineTag.attribute(HMF_STYLETAG);

    TextBoxWidget *pWidget = pContainer->addTextBoxWidget(point, NoUndo);
    pWidget->setText(text);
    pWidget->setFont(font);
    pWidget->setColor(color);
    pWidget->setSize(width, height);
    pWidget->setLineWidth(linewidth);
    pWidget->setBoxVisible(lineVisible);
    if(linestyle == "solidline")
        pWidget->setLineStyle(Qt::SolidLine);
    if(linestyle == "dashline")
        pWidget->setLineStyle(Qt::DashLine);
    if(linestyle == "dotline")
        pWidget->setLineStyle(Qt::DotLine);
    if(linestyle == "dashdotline")
        pWidget->setLineStyle(Qt::DashDotLine);
    pWidget->setSelected(true);     //!< @todo Stupid!
    pWidget->setSelected(false);    //For some reason this is needed...
    if(undoSettings == Undo)
    {
        pContainer->getUndoStackPtr()->registerAddedWidget(pWidget);
    }

    return pWidget;
}
