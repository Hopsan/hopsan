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
//! @file   GUISystem.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI System class, representing system components
//!
//$Id$

#include "GUISystem.h"
#include "MainWindow.h"
#include "GraphicsView.h"
#include "CoreAccess.h"
#include "loadFunctions.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "version_gui.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Dialogs/ContainerPropertiesDialog.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIWidgets.h"
#include "Widgets/PyDockWidget.h"
#include "Configuration.h"
#include "GUIContainerObject.h"

SystemContainer::SystemContainer(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : ContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpParentProjectTab = pParentContainer->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
SystemContainer::SystemContainer(ProjectTab *parentProjectTab, QGraphicsItem *pParent)
    : ContainerObject(QPointF(0,0), 0, 0, Deselected, UserGraphics, 0, pParent)
{
    this->mModelObjectAppearance = *(gpMainWindow->mpLibrary->getAppearanceData(HOPSANGUISYSTEMTYPENAME)); //This will crash if Subsystem not already loaded
    this->mpParentProjectTab = parentProjectTab;
    this->commonConstructorCode();
    this->mpUndoStack->newPost();
    this->mSaveUndoStack = false;       //Do not save undo stack by default
}

void SystemContainer::deleteInHopsanCore()
{
    this->setUndoEnabled(false, true); //The last true means DONT ASK
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,GUISystem destructor";
    //First remove all contents
    this->clearContents();

    if (mpParentContainerObject != 0)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
    }
    else
    {
        mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @brief This code is common among the two constructors, we use one function to avoid code duplication
void SystemContainer::commonConstructorCode()
{
        //Set the hmf save tag name
    mHmfTagName = HMF_SYSTEMTAG;

        //Set default values
    mLoadType = "EMBEDED";
    mNumberOfLogSamples = 2048;

        //Create the object in core, and update name
    if (this->mpParentContainerObject == 0)
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
        qDebug() << "creating subsystem and setting name in " << mpParentContainerObject->getCoreSystemAccessPtr()->getSystemName();
        mName = mpParentContainerObject->getCoreSystemAccessPtr()->createSubSystem(this->getName());
        refreshDisplayName();
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentContainerObject->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentContainerObject->getCoreSystemAccessPtr());
    }

    refreshDisplayName(); //Make sure name window is correct size for center positioning
}


//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//!
void SystemContainer::setName(QString newName)
{
    if (mpParentContainerObject == 0)
    {
        mName = mpCoreSystemAccess->setSystemName(newName);
    }
    else
    {
        mpParentContainerObject->renameModelObject(this->getName(), newName);
    }
    refreshDisplayName();
}


//! Returns a string with the sub system type.
QString SystemContainer::getTypeName()
{
    //! @todo is this OK should really ask the subsystem but result should be subsystem i think
    return HOPSANGUISYSTEMTYPENAME;
}

//! @brief Get the system cqs type
//! @returns A string containing the CQS type
QString SystemContainer::getTypeCQS()
{
    return mpCoreSystemAccess->getSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QStringList containing the parameter names
QStringList SystemContainer::getParameterNames()
{
    return mpCoreSystemAccess->getSystemParameterNames();
}

//! @brief Get a vector contain data from all parameters
//! @param [out] rParameterDataVec A vector that will contain parameter data
void SystemContainer::getParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    mpCoreSystemAccess->getSystemParameters(rParameterDataVec);
}

//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString SystemContainer::getParameterValue(const QString paramName)
{
    return mpCoreSystemAccess->getSystemParameterValue(paramName);
}

//! @brief Get parameter data for a specific parameter
//! @param [out] rData The parameter data
void SystemContainer::getParameter(const QString paramName, CoreParameterData &rData)
{
    return mpCoreSystemAccess->getSystemParameter(paramName, rData);
}

//! @brief Get a pointer the the CoreSystemAccess object that this system is representing
CoreSystemAccess* SystemContainer::getCoreSystemAccessPtr()
{
    return mpCoreSystemAccess;
}

//! @brief Overloaded version that returns self if root system
ContainerObject *SystemContainer::getParentContainerObject()
{
    if (mpParentContainerObject==0)
    {
        return this;
    }
    else
    {
        return mpParentContainerObject;
    }
}


int SystemContainer::type() const
{
    return Type;
}


//! @brief Opens the GUISystem properties dialog
void SystemContainer::openPropertiesDialog()
{
    //! @todo shouldnt this be in the containerproperties class, right now groups are not working thats is why it is here, the containerproperties dialog only works with systems for now
    ContainerPropertiesDialog dialog(this, gpMainWindow);
    dialog.setAttribute(Qt::WA_DeleteOnClose, false);
    dialog.exec();
}


//! @brief Saves the System specific coredata to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement);

    if (mLoadType == "EXTERNAL" && contents == FullModel)
    {
        //This information should ONLY be used to indicate that a system is external, it SHOULD NOT be included in the actual external system
        //If it would be, the load function will fail
        rDomElement.setAttribute( HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentContainerObject->getModelFileInfo().absolutePath()) );
    }

    if (mLoadType != "EXTERNAL" && contents == FullModel)
    {
        appendSimulationTimeTag(rDomElement, mpParentProjectTab->getStartTime().toDouble(), this->getTimeStep(), mpParentProjectTab->getStopTime().toDouble(), this->doesInheritTimeStep());

//        //AllDataGenerations::AliasMapT::iterator ita;
//        QDomElement xmlAliases = appendDomElement(rDomElement, HMF_ALIASES);
//        AllDataGenerations::AliasMapT aliasMap = getPlotDataPtr()->getPlotAliasMap();
//        for(ita=aliasMap.begin(); ita!=aliasMap.end(); ++ita)
//        {
//            QDomElement aliasElement = appendDomElement(xmlAliases, HMF_ALIAS);
//            aliasElement.setAttribute("alias", ita.key());
//            aliasElement.setAttribute("component", ita.value().componentName);
//            aliasElement.setAttribute("port",ita.value().portName);
//            aliasElement.setAttribute("data",ita.value().dataName);
//        }
    }

    //Save the parameter values for the system
    // In case of external system save those that have been changed
    //! @todo right now we save all of them, but I think this is good even if they have not changed
    QVector<CoreParameterData> paramDataVector;
    this->getParameters(paramDataVector);
    QDomElement xmlParameters = appendDomElement(rDomElement, HMF_PARAMETERS);
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        QDomElement xmlParameter = appendDomElement(xmlParameters, HMF_PARAMETERTAG);
        xmlParameter.setAttribute(HMF_NAMETAG, paramDataVector[i].mName);
        xmlParameter.setAttribute(HMF_VALUETAG, paramDataVector[i].mValue);
        xmlParameter.setAttribute(HMF_TYPE, paramDataVector[i].mType);
        xmlParameter.setAttribute(HMF_UNIT, paramDataVector[i].mUnit);
    }

    // Save the alias names in this system
    QDomElement xmlAliases = appendDomElement(rDomElement, HMF_ALIASES);
    QStringList aliases = getAliasNames();
    //! @todo need one function that gets both alias anf full maybe
    for (int i=0; i<aliases.size(); ++i)
    {
        QDomElement alias = appendDomElement(xmlAliases, HMF_ALIAS);
        alias.setAttribute(HMF_TYPE, "variable"); //!< @todo not maunal type
        alias.setAttribute(HMF_NAMETAG, aliases[i]);
        QString fullName = getFullNameFromAlias(aliases[i]);
        appendDomTextNode(alias, "fullname",fullName );
    }
}


void SystemContainer::saveOptSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLopt = appendDomElement(rDomElement, "optimization");
    QDomElement XMLsetting = appendDomElement(XMLopt, "settings");
    appendDomIntegerNode(XMLsetting, "niter", mOptSettings.mNiter);
    appendDomIntegerNode(XMLsetting, "nsearchp", mOptSettings.mNsearchp);
    appendDomValueNode(XMLsetting, "refcoeff", mOptSettings.mRefcoeff);
    appendDomValueNode(XMLsetting, "randfac", mOptSettings.mRandfac);
    appendDomValueNode(XMLsetting, "forgfac", mOptSettings.mForgfac);
    appendDomValueNode(XMLsetting, "functol", mOptSettings.mFunctol);
    appendDomValueNode(XMLsetting, "partol", mOptSettings.mPartol);
    appendDomBooleanNode(XMLsetting, "plot", mOptSettings.mPlot);
    appendDomBooleanNode(XMLsetting, "savecsv", mOptSettings.mSavecsv);

    //Parameters
    appendDomBooleanNode(XMLsetting, "logpar", mOptSettings.mlogPar);
    QDomElement XMLparameters = appendDomElement(XMLopt, "parameters");
    for(int i = 0; i < mOptSettings.mParamters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, "parameter");
        appendDomTextNode(XMLparameter, "componentname", mOptSettings.mParamters.at(i).mComponentName);
        appendDomTextNode(XMLparameter, "parametername", mOptSettings.mParamters.at(i).mParameterName);
        appendDomValueNode2(XMLparameter, "minmax", mOptSettings.mParamters.at(i).mMin, mOptSettings.mParamters.at(i).mMax);
    }

    //Objective Functions
    QDomElement XMLobjectives = appendDomElement(XMLopt, "objectives");
    for(int i = 0; i < mOptSettings.mObjectives.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, "objective");
        appendDomTextNode(XMLobjective, "functionname", mOptSettings.mObjectives.at(i).mFunctionName);
        appendDomValueNode(XMLobjective, "weight", mOptSettings.mObjectives.at(i).mWeight);
        appendDomValueNode(XMLobjective, "norm", mOptSettings.mObjectives.at(i).mNorm);
        appendDomValueNode(XMLobjective, "exp", mOptSettings.mObjectives.at(i).mExp);

        QDomElement XMLObjectiveVariables = appendDomElement(XMLobjective, "variables");
        if(!(mOptSettings.mObjectives.at(i).mVariableInfo.isEmpty()))
        {
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mVariableInfo.size(); ++j)
            {
                QDomElement XMLObjectiveVariable = appendDomElement(XMLObjectiveVariables, "variable");
                appendDomTextNode(XMLObjectiveVariable, "componentname", mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(0));
                appendDomTextNode(XMLObjectiveVariable, "portname", mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(1));
                appendDomTextNode(XMLObjectiveVariable, "variablename", mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(2));
            }
        }


        if(!(mOptSettings.mObjectives.at(i).mData.isEmpty()))
        {
            QDomElement XMLdata = appendDomElement(XMLobjective, "data");
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mData.size(); ++j)
            {
                appendDomTextNode(XMLdata, "parameter", mOptSettings.mObjectives.at(i).mData.at(j));
            }
        }
    }
}


void SystemContainer::loadOptSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    if(!rDomElement.firstChildElement("settings").isNull())
    {
        if(!rDomElement.firstChildElement("settings").firstChildElement("niter").isNull())
            mOptSettings.mNiter = parseDomIntegerNode(rDomElement.firstChildElement("settings").firstChildElement("niter"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("nsearchp").isNull())
            mOptSettings.mNsearchp = parseDomIntegerNode(rDomElement.firstChildElement("settings").firstChildElement("nsearchp"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("refcoeff").isNull())
            mOptSettings.mRefcoeff = parseDomValueNode(rDomElement.firstChildElement("settings").firstChildElement("refcoeff"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("randfac").isNull())
            mOptSettings.mRandfac = parseDomValueNode(rDomElement.firstChildElement("settings").firstChildElement("randfac"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("forgfac").isNull())
            mOptSettings.mForgfac = parseDomValueNode(rDomElement.firstChildElement("settings").firstChildElement("forgfac"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("functol").isNull())
            mOptSettings.mFunctol = parseDomValueNode(rDomElement.firstChildElement("settings").firstChildElement("functol"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("partol").isNull())
            mOptSettings.mPartol = parseDomValueNode(rDomElement.firstChildElement("settings").firstChildElement("partol"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("plot").isNull())
            mOptSettings.mPlot = parseDomBooleanNode(rDomElement.firstChildElement("settings").firstChildElement("plot"));
        if(!rDomElement.firstChildElement("settings").firstChildElement("savecsv").isNull())
            mOptSettings.mSavecsv = parseDomBooleanNode(rDomElement.firstChildElement("settings").firstChildElement("savecsv"));

        if(!rDomElement.firstChildElement("settings").firstChildElement("logpar").isNull())
            mOptSettings.mlogPar = parseDomBooleanNode(rDomElement.firstChildElement("settings").firstChildElement("logpar"));
    }
    if(!rDomElement.firstChildElement("parameters").isNull())
    {
        QDomElement XMLpar = rDomElement.firstChildElement("parameters").firstChildElement("parameter");
        while (!XMLpar.isNull())
        {
            OptParameter parameter;
            parameter.mComponentName = XMLpar.firstChildElement("componentname").text();
            parameter.mParameterName = XMLpar.firstChildElement("parametername").text();
            parseDomValueNode2(XMLpar.firstChildElement("minmax"), parameter.mMin, parameter.mMax);
            mOptSettings.mParamters.append(parameter);

            XMLpar = XMLpar.nextSiblingElement("parameter");
        }

        if(!rDomElement.firstChildElement("parameters").firstChildElement("savecsv").isNull())
            mOptSettings.mSavecsv = parseDomBooleanNode(rDomElement.firstChildElement("settings").firstChildElement("savecsv"));
    }
    if(!rDomElement.firstChildElement("objectives").isNull())
    {
        QDomElement XMLobj = rDomElement.firstChildElement("objectives").firstChildElement("objective");
        while (!XMLobj.isNull())
        {
            Objectives objectives;

            objectives.mFunctionName = XMLobj.firstChildElement("functionname").text();
            objectives.mWeight = XMLobj.firstChildElement("weight").text().toDouble();
            objectives.mNorm = XMLobj.firstChildElement("norm").text().toDouble();
            objectives.mExp = XMLobj.firstChildElement("exp").text().toDouble();

            if(!XMLobj.firstChildElement("variables").isNull())
            {
                QDomElement XMLVars = XMLobj.firstChildElement("variables").firstChildElement("variable");
                while (!XMLVars.isNull())
                {
                    QStringList variableInfo;

                    variableInfo.append(XMLVars.firstChildElement("componentname").text());
                    variableInfo.append(XMLVars.firstChildElement("portname").text());
                    variableInfo.append(XMLVars.firstChildElement("variablename").text());

                    objectives.mVariableInfo.append(variableInfo);

                    XMLVars = XMLVars.nextSiblingElement("variable");
                }
            }

            if(!XMLobj.firstChildElement("data").isNull())
            {
                QDomElement XMLpar = XMLobj.firstChildElement("data").firstChildElement("parameter");
                while (!XMLpar.isNull())
                {
                    objectives.mData.append(XMLpar.text());

                    XMLpar = XMLpar.nextSiblingElement("parameter");
                }
            }

            XMLobj = XMLobj.nextSiblingElement("objective");

            mOptSettings.mObjectives.append(objectives);
        }
    }
}


OptimizationSettings SystemContainer::getOptimizationSettings()
{
    return mOptSettings;
}


void SystemContainer::setOptimizationSettings(OptimizationSettings optSettings)
{
    mOptSettings = optSettings;
}


//! @brief Saves the System specific GUI data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
QDomElement SystemContainer::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    QDomElement guiStuff = ModelObject::saveGuiDataToDomElement(rDomElement);

    //Should we try to append appearancedata stuff, we dont want this in external systems as they contain their own appearance
    if (mLoadType!="EXTERNAL")
    {
        //! @todo what happens if a subsystem (embeded) is asved, then we dont want to set the current graphics view
        if (this->mpParentProjectTab->getGraphicsView() != 0)
        {
            qreal x,y,zoom;
            this->mpParentProjectTab->getGraphicsView()->getViewPort(x,y,zoom);
            appendViewPortTag(guiStuff, x, y, zoom);
        }
        QDomElement portsHiddenElement = appendDomElement(guiStuff, HMF_PORTSTAG);
        portsHiddenElement.setAttribute("hidden", mSubComponentPortsHidden);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, HMF_NAMESTAG);
        namesHiddenElement.setAttribute("hidden", mSubComponentNamesHidden);

        QString gfxType = "iso";
        if(mGfxType == UserGraphics)
            gfxType = "user";
        QDomElement gfxTypeElement = appendDomElement(guiStuff, HMF_GFXTAG);
        gfxTypeElement.setAttribute("type", gfxType);

        QDomElement scriptFileElement = appendDomElement(guiStuff, HMF_SCRIPTFILETAG);
        scriptFileElement.setAttribute("path", mScriptFilePath);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mModelObjectAppearance.saveToDomElement(xmlApp);

        //Save favorite variables
        QDomElement xmlFavVars = appendDomElement(guiStuff, HMF_FAVORITEVARIABLES);
        QList<VariableDescription> favVars = this->getLogDataHandler()->getFavoriteVariableList();
        QList<VariableDescription>::iterator itf;
        for(itf = favVars.begin(); itf != favVars.end(); ++itf)
        {
            QDomElement favoriteElement = appendDomElement(xmlFavVars, HMF_FAVORITEVARIABLETAG);
            favoriteElement.setAttribute("componentname", (*itf).mComponentName);
            favoriteElement.setAttribute("portname", (*itf).mPortName);
            favoriteElement.setAttribute("dataname", (*itf).mDataName);
            favoriteElement.setAttribute("dataunit", (*itf).mDataUnit);
        }
    }

    saveOptSettingsToDomElement(guiStuff);

    //Save undo stack if setting is activated
    if(mSaveUndoStack)
    {
        guiStuff.appendChild(mpUndoStack->toXml());
    }

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    if(this == mpParentProjectTab->getTopLevelSystem() && contents==FullModel)
    {
        //Append model info
        QString author, email, affiliation, description;
        getModelInfo(author, email, affiliation, description);
        QDomElement infoElement = appendDomElement(rDomElement, HMF_INFOTAG);
        appendDomTextNode(infoElement, HMF_AUTHORTAG, author);
        appendDomTextNode(infoElement, HMF_EMAILTAG, email);
        appendDomTextNode(infoElement, HMF_AFFILIATIONTAG, affiliation);
        appendDomTextNode(infoElement, HMF_DESCRIPTIONTAG, description);
    }

    //qDebug() << "Saving to dom node in: " << this->mModelObjectAppearance.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, mHmfTagName);

    //! @todo maybe use enums instead of strings
    //! @todo should not need to set this here
    if (mpParentContainerObject==0)
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
        xmlSubsystem.setAttribute(HMF_LOGSAMPLES, mNumberOfLogSamples);
    }

    if(contents==FullModel)
    {
        // Save gui object stuff
        this->saveGuiDataToDomElement(xmlSubsystem);
    }

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save subcomponents and subsystems
        QDomElement xmlObjects = appendDomElement(xmlSubsystem, HMF_OBJECTS);
        ModelObjectMapT::iterator it;
        for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlObjects, contents);
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
            QDomElement xmlConnections = appendDomElement(xmlSubsystem, HMF_CONNECTIONS);
            for(int i=0; i<mSubConnectorList.size(); ++i)
            {
                mSubConnectorList[i]->saveToDomElement(xmlConnections);
            }
        }
    }
}

//! @brief Loads a System from an XML DOM Element
//! @param[in] rDomElement The element to load from
void SystemContainer::loadFromDomElement(QDomElement &rDomElement)
{
    // Loop back up to root level to get version numbers
    double hmfVersion = rDomElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_VERSIONTAG).toDouble();
    QString coreHmfVersion = rDomElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_HOPSANCOREVERSIONTAG);

    if(hmfVersion <= 0.2 && hmfVersion != 0.0)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("Model file is saved with Hopsan version 0.2 or older. Full compatibility is not guaranteed.");
    }
    else if(hmfVersion != QString(HMF_VERSIONNUM).toDouble() && hmfVersion != 0.0)
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("Model file is saved with an older version of Hopsan, but versions are compatible.");
    }

    //Load model info
    QDomElement infoElement = rDomElement.parentNode().firstChildElement(HMF_INFOTAG);
    if(!infoElement.isNull())
    {
        QString author, email, affiliation, description;
        QDomElement authorElement = infoElement.firstChildElement(HMF_AUTHORTAG);
        if(!authorElement.isNull())
        {
            author = authorElement.text();
        }
        QDomElement emailElement = infoElement.firstChildElement(HMF_EMAILTAG);
        if(!emailElement.isNull())
        {
            email = emailElement.text();
        }
        QDomElement affiliationElement = infoElement.firstChildElement(HMF_AFFILIATIONTAG);
        if(!affiliationElement.isNull())
        {
            affiliation = affiliationElement.text();
        }
        QDomElement descriptionElement = infoElement.firstChildElement(HMF_DESCRIPTIONTAG);
        if(!descriptionElement.isNull())
        {
            description = descriptionElement.text();
        }

        this->setModelInfo(author, email, affiliation, description);
    }

    //Check if the subsystem is external or internal, and load appropriately
    QString external_path = rDomElement.attribute(HMF_EXTERNALPATHTAG);
    if (external_path.isEmpty())
    {
        //Load embedded subsystem
        //0. Load core and gui stuff
        //! @todo might need some error checking here incase some fields are missing
        //Now load the core specific data, might need inherited function for this
        this->setName(rDomElement.attribute(HMF_NAMETAG));

        //Load the GUI stuff like appearance data and viewport
        QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
        this->mModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT));
        this->refreshDisplayName(); // This must be done becouse in some occations the loadAppearanceDataline above will overwrite the correct name
        this->mSubComponentNamesHidden = guiStuff.firstChildElement(HMF_NAMESTAG).attribute("hidden").toInt();
        this->mSubComponentPortsHidden = guiStuff.firstChildElement(HMF_PORTSTAG).attribute("hidden").toInt();
        QString gfxType = guiStuff.firstChildElement(HMF_GFXTAG).attribute("type");
        if(gfxType == "user") { mGfxType = UserGraphics; }
        else if(gfxType == "iso") { mGfxType = ISOGraphics; }
        gpMainWindow->mpToggleNamesAction->setChecked(!mSubComponentNamesHidden);
        gpMainWindow->mpTogglePortsAction->setChecked(!mSubComponentPortsHidden);
        double x = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("x").toDouble();
        double y = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("y").toDouble();
        double zoom = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("zoom").toDouble();
        setScriptFile(guiStuff.firstChildElement(HMF_SCRIPTFILETAG).attribute("path"));

        bool dontClearUndo = false;
        if(!guiStuff.firstChildElement(HMF_UNDO).isNull())
        {
            QDomElement undoElement = guiStuff.firstChildElement(HMF_UNDO);
            mpUndoStack->fromXml(undoElement);
            dontClearUndo = true;
            mSaveUndoStack = true;      //Set save undo stack setting to true if loading a hmf file with undo stack saved
        }

        mpParentProjectTab->getGraphicsView()->setZoomFactor(zoom);

        qDebug() << "Center on " << x << ", " << y;
        mpParentProjectTab->getGraphicsView()->centerOn(x, y);
        //! @todo load viewport and pose and stuff

        //Load simulation time
        QString startT,stepT,stopT;
        bool inheritTs;
        parseSimulationTimeTag(rDomElement.firstChildElement(HMF_SIMULATIONTIMETAG), startT, stepT, stopT, inheritTs);
        this->setTimeStep(stepT.toDouble());
        mpCoreSystemAccess->setInheritTimeStep(inheritTs);

        //Load number of log samples
        if(rDomElement.hasAttribute(HMF_LOGSAMPLES))
        {
            mNumberOfLogSamples = rDomElement.attribute(HMF_LOGSAMPLES).toInt();
        }

        //Only set start stop time for the top level system
        if (mpParentContainerObject == 0)
        {
            mpParentProjectTab->setTopLevelSimulationTime(startT,stepT,stopT);
            mpParentProjectTab->setToolBarSimulationTimeParametersFromTab();
        }

        //1. Load global parameters
        QDomElement xmlParameters = rDomElement.firstChildElement(HMF_PARAMETERS);
        QDomElement xmlSubObject = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, hmfVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_PARAMETERTAG);
        }

        //2. Load all sub-components
        QDomElement xmlSubObjects = rDomElement.firstChildElement(HMF_OBJECTS);
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_COMPONENTTAG);
        while (!xmlSubObject.isNull())
        {
            verifyHmfSubComponentCompatibility(xmlSubObject, hmfVersion, coreHmfVersion);
            ModelObject* pObj = loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NoUndo);
            if(pObj == NULL)
            {
                gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage(QString("Model contains component from a library that has not been loaded. TypeName: ") +
                                                                    xmlSubObject.attribute(HMF_TYPENAME) + QString(", Name: ") + xmlSubObject.attribute(HMF_NAMETAG));

                // Insert missing component dummy instead
                xmlSubObject.setAttribute(HMF_TYPENAME, "MissingComponent");
                pObj = loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NoUndo);
            }
            else
            {



                //! @deprecated This StartValue load code is only kept for upconverting old files, we should keep it here until we have some other way of upconverting old formats
                //Load start values //Is not needed, start values are saved as ordinary parameters! This code snippet can probably be removed.
                QDomElement xmlStartValues = xmlSubObject.firstChildElement(HMF_STARTVALUES);
                QDomElement xmlStartValue = xmlStartValues.firstChildElement(HMF_STARTVALUE);
                while (!xmlStartValue.isNull())
                {
                    loadStartValue(xmlStartValue, pObj, NoUndo);
                    xmlStartValue = xmlStartValue.nextSiblingElement(HMF_STARTVALUE);
                }
            }

            if(pObj && pObj->getTypeName().startsWith("CppComponent"))
            {
                recompileCppComponents(pObj);
            }

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }

        //3. Load all text box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_TEXTBOXWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextBoxWidget(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTBOXWIDGETTAG);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //6. Load all systemports
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadContainerPortObject(xmlSubObject, gpMainWindow->mpLibrary, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMPORTTAG);
        }

        //7. Load all connectors
        QDomElement xmlConnections = rDomElement.firstChildElement(HMF_CONNECTIONS);
        xmlSubObject = xmlConnections.firstChildElement(HMF_CONNECTORTAG);
        QList<QDomElement> failedConnections;
        while (!xmlSubObject.isNull())
        {
            if(!loadConnector(xmlSubObject, this, NoUndo))
            {
                failedConnections.append(xmlSubObject);
            }
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_CONNECTORTAG);
        }
        //If some connectors failed to load, it could mean that they were loaded in wrong order.
        //Try again until they work, or abort if number of attempts are greater than maximum possible for success.
        int stop=failedConnections.size()*(failedConnections.size()+1)/2;
        int i=0;
        while(!failedConnections.isEmpty())
        {
            if(!loadConnector(failedConnections.first(), this, NoUndo))
            {
                failedConnections.append(failedConnections.first());
            }
            failedConnections.removeFirst();
            ++i;
            if(i>stop) break;
        }


        //8. Load favorite variables
        QDomElement xmlFavVariables = guiStuff.firstChildElement(HMF_FAVORITEVARIABLES);
        QDomElement xmlFavVariable = xmlFavVariables.firstChildElement(HMF_FAVORITEVARIABLETAG);
        while (!xmlFavVariable.isNull())
        {
            loadFavoriteVariable(xmlFavVariable, this);
            xmlFavVariable = xmlFavVariable.nextSiblingElement(HMF_FAVORITEVARIABLETAG);
        }

        //8.1 Load favorite variables, from old place among core data (which is wrong)
        //! @deprecated Remove this block of code later on in the future
        xmlSubObject = xmlParameters.firstChildElement(HMF_FAVORITEVARIABLETAG);
        while (!xmlSubObject.isNull())
        {
            loadFavoriteVariable(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_FAVORITEVARIABLETAG);
        }

        //9. Load plot variable aliases
        QDomElement xmlAliases = rDomElement.firstChildElement(HMF_ALIASES);
        QDomElement xmlAlias = xmlAliases.firstChildElement(HMF_ALIAS);
        while (!xmlAlias.isNull())
        {
            loadPlotAlias(xmlAlias, this);
            xmlAlias = xmlAlias.nextSiblingElement(HMF_ALIAS);
        }

        //9.1 Load plot variable aliases
        //! @deprecated Remove in teh future when hmf format stabilized and everyone has upgraded
        xmlSubObject = xmlParameters.firstChildElement("alias");
        while (!xmlSubObject.isNull())
        {
            loadPlotAlias(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement("alias");
        }

        //10. Load optimization settings
        xmlSubObject = guiStuff.firstChildElement("optimization");
        loadOptSettingsFromDomElement(xmlSubObject);

        //Refresh the appearance of the subsystemem and create the GUIPorts based on the loaded portappearance information
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
        //! @todo maybe can do this for subsystems to (even if we dont see them right now)
        if (this->mpParentContainerObject == 0)
        {
            //mpParentProjectTab->getGraphicsView()->centerView();
            mpParentProjectTab->getGraphicsView()->updateViewPort();
        }
        this->mpParentProjectTab->setSaved(true);

        gpMainWindow->mpPyDockWidget->runPyScript(mScriptFilePath);

        emit systemParametersChanged(); // Make sure we refresh the syspar widget
        emit checkMessages();
    }
    else
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printWarningMessage("A system you tried to load is taged as an external system, but the ContainerSystem load function only loads embeded systems");
    }
}


void SystemContainer::createLabviewSourceFiles()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(gpMainWindow, tr("Export to LabVIEW/SIT"),
                                  "This will create source code for a LabVIEW/SIT DLL-file from current model. You will need the HopsanCore source code and Visual Studio 2003 to compile it.\nContinue?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    //QFileInfo fileInfo;
    //QFile file;
    filePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Export Project to HopsanRT Wrapper Code"),
                                            fileDialogSaveDir.currentPath(),
                                            tr("Text file (*.txt)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToLabViewSIT(filePath, this);
    delete(pCoreAccess);
}

void SystemContainer::exportToFMU()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
                                                    gConfig.getFmuExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QDir saveDir;
    saveDir.setPath(savePath);
    gConfig.setFmuExportDir(saveDir.absolutePath());
    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    if(!saveDir.entryList().isEmpty())
    {
        qDebug() << saveDir.entryList();
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
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

    exportToFMU(savePath);
}


void SystemContainer::exportToFMU(QString savePath)
{
    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    //Save model to hmf in export directory
    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, "0");
    saveToDomElement(hmfRoot);
    QFile xmlhmf(savePath + "/" + mModelFileInfo.fileName());
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to open "+savePath+"/"+mModelFileInfo.fileName()+" for writing.");
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, XMLINDENTATION);

    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToFmu(savePath, this);
    delete(pCoreAccess);

}

//void SystemContainer::exportToFMU()
//{
//    //Open file dialog and initialize the file stream
//    QDir fileDialogSaveDir;
//    QString savePath;
//    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
//                                                    gConfig.getFmuExportDir(),
//                                                    QFileDialog::ShowDirsOnly
//                                                    | QFileDialog::DontResolveSymlinks);
//    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

//    QDir saveDir;
//    saveDir.setPath(savePath);
//    gConfig.setFmuExportDir(saveDir.absolutePath());
//    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
//    if(!saveDir.entryList().isEmpty())
//    {
//        qDebug() << saveDir.entryList();
//        QMessageBox msgBox;
//        msgBox.setWindowIcon(gpMainWindow->windowIcon());
//        msgBox.setText(QString("Folder is not empty!"));
//        msgBox.setInformativeText("Are you sure you want to export files here?");
//        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        msgBox.setDefaultButton(QMessageBox::No);

//        int answer = msgBox.exec();
//        if(answer == QMessageBox::No)
//        {
//            return;
//        }
//    }
//    saveDir.setFilter(QDir::NoFilter);


//    //Tells if user selected the gcc compiler or not (= visual studio)
//    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();


//    //Write the FMU ID
//    int random = rand() % 1000;
//    QString randomString = QString().setNum(random);
//    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?


//    //Collect information about input ports
//    QStringList inputVariables;
//    QStringList inputComponents;
//    QStringList inputPorts;
//    QList<int> inputDatatypes;

//    ModelObjectMapT::iterator it;
//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalInputInterface")
//        {
//            inputVariables.append(it.value()->getName().remove(' '));
//            inputComponents.append(it.value()->getName());
//            inputPorts.append("out");
//            inputDatatypes.append(0);
//        }
//    }


//    //Collect information about output ports
//    QStringList outputVariables;
//    QStringList outputComponents;
//    QStringList outputPorts;
//    QList<int> outputDatatypes;

//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalOutputInterface")
//        {
//            outputVariables.append(it.value()->getName().remove(' '));
//            outputComponents.append(it.value()->getName());
//            outputPorts.append("in");
//            outputDatatypes.append(0);
//        }
//    }


//    //Create file objects for all files that shall be created
//    QFile modelSourceFile;
//    QString modelName = getModelFileInfo().fileName();
//    modelName.chop(4);
//    QString realModelName = modelName;          //Actual model name (used for hmf file)
//    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
//    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
//    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open " + modelName + ".c for writing.");
//        return;
//    }

//    QFile modelDescriptionFile;
//    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
//    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open modelDescription.xml for writing.");
//        return;
//    }

//    QFile fmuHeaderFile;
//    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
//    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open HopsanFMU.h for writing.");
//        return;
//    }

//    QFile fmuSourceFile;
//    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
//    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open HopsanFMU.cpp for writing.");
//        return;
//    }

//#ifdef WIN32
//    QFile clBatchFile;
//    clBatchFile.setFileName(savePath + "/compile.bat");
//    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Failed to open compile.bat for writing.");
//        return;
//    }
//#endif

//    //progressBar.setLabelText("Writing modelDescription.xml");
//    //progressBar.setValue(1);

//    QTextStream modelDescriptionStream(&modelDescriptionFile);
//    modelDescriptionStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";       //!< @todo Encoding, should it be UTF-8?
//    modelDescriptionStream << "<fmiModelDescription\n";
//    modelDescriptionStream << "  fmiVersion=\"1.0\"\n";
//    modelDescriptionStream << "  modelName=\"" << modelName << "\"\n";               //!< @todo What's the difference between name and identifier?
//    modelDescriptionStream << "  modelIdentifier=\"" << modelName << "\"\n";
//    modelDescriptionStream << "  guid=\"" << ID << "\"\n";
//    modelDescriptionStream << "  numberOfContinuousStates=\"" << inputVariables.size() + outputVariables.size() << "\"\n";
//    modelDescriptionStream << "  numberOfEventIndicators=\"0\">\n";
//    modelDescriptionStream << "<ModelVariables>\n";
//    int i, j;
//    for(i=0; i<inputVariables.size(); ++i)
//    {
//        QString refString = QString().setNum(i);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << inputVariables.at(i) << "\" valueReference=\""+refString+"\" description=\"input variable\" causality=\"input\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    for(j=0; j<outputVariables.size(); ++j)
//    {
//        QString refString = QString().setNum(i+j);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << outputVariables.at(j) << "\" valueReference=\""+refString+"\" description=\"output variable\" causality=\"output\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    modelDescriptionStream << "</ModelVariables>\n";
//    modelDescriptionStream << "</fmiModelDescription>\n";
//    modelDescriptionFile.close();


//    //progressBar.setLabelText("Writing " + modelName + ".c");
//    //progressBar.setValue(2);


//    QTextStream modelSourceStream(&modelSourceFile);
//    modelSourceStream << "// Define class name and unique id\n";
//    modelSourceStream << "    #define MODEL_IDENTIFIER " << modelName << "\n";
//    modelSourceStream << "    #define MODEL_GUID \"" << ID << "\"\n\n";
//    modelSourceStream << "    // Define model size\n";
//    modelSourceStream << "    #define NUMBER_OF_REALS " << inputVariables.size() + outputVariables.size() << "\n";
//    modelSourceStream << "    #define NUMBER_OF_INTEGERS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_BOOLEANS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STRINGS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STATES "<< inputVariables.size() + outputVariables.size() << "\n";        //!< @todo Does number of variables equal number of states?
//    modelSourceStream << "    #define NUMBER_OF_EVENT_INDICATORS 0\n\n";
//    modelSourceStream << "    // Include fmu header files, typedefs and macros\n";
//    modelSourceStream << "    #include \"fmuTemplate.h\"\n";
//    modelSourceStream << "    #include \"HopsanFMU.h\"\n\n";
//    modelSourceStream << "    // Define all model variables and their value references\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "    #define " << inputVariables.at(i) << "_ " << i << "\n\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "    #define " << outputVariables.at(j) << "_ " << j+i << "\n\n";
//    modelSourceStream << "    // Define state vector as vector of value references\n";
//    modelSourceStream << "    #define STATES { ";
//    i=0;
//    j=0;
//    if(!inputVariables.isEmpty())
//    {
//        modelSourceStream << inputVariables.at(0) << "_";
//        ++i;
//    }
//    else if(!outputVariables.isEmpty())
//    {
//        modelSourceStream << outputVariables.at(0) << "_";
//        ++j;
//    }
//    for(; i<inputVariables.size(); ++i)
//        modelSourceStream << ", " << inputVariables.at(i) << "_";
//    for(; j<outputVariables.size(); ++j)
//        modelSourceStream << ", " << outputVariables.at(j) << "_";
//    modelSourceStream << " }\n\n";
//    modelSourceStream << "    //Set start values\n";
//    modelSourceStream << "    void setStartValues(ModelInstance *comp) \n";
//    modelSourceStream << "    {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "        r(" << inputVariables.at(i) << "_) = 0;\n";        //!< Fix start value handling
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "        r(" << outputVariables.at(j) << "_) = 0;\n";        //!< Fix start value handling
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Initialize\n";
//    modelSourceStream << "    void initialize(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        initializeHopsanWrapper(\""+realModelName+".hmf\");\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Return variable of real type\n";
//    modelSourceStream << "    fmiReal getReal(ModelInstance* comp, fmiValueReference vr)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: return getVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ");\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: return getVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ");\n";
//    modelSourceStream << "            default: return 1;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    void setReal(ModelInstance* comp, fmiValueReference vr, fmiReal value)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: setVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ", value);\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: setVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ", value);\n";
//    modelSourceStream << "            default: return;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Update at time event\n";
//    modelSourceStream << "    void eventUpdate(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        simulateOneStep();\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";      //!< @todo Hardcoded timestep
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    // Include code that implements the FMI based on the above definitions\n";
//    modelSourceStream << "    #include \"fmuTemplate.c\"\n";
//    modelSourceFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.h");
//    //progressBar.setValue(4);


//    QTextStream fmuHeaderStream(&fmuHeaderFile);
//    QTextLineStream fmuHeaderLines(fmuHeaderStream);
//    fmuHeaderLines << "#ifndef HOPSANFMU_H";
//    fmuHeaderLines << "#define HOPSANFMU_H";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    //fmuHeaderLines << "    #define DLLEXPORT __declspec(dllexport)";
//    fmuHeaderLines << "    extern \"C\" {";
//    fmuHeaderLines << "#else";
//    fmuHeaderLines << "    #define DLLEXPORT";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void initializeHopsanWrapper(char* filename);";
//    fmuHeaderLines << "DLLEXPORT void simulateOneStep();";
//    fmuHeaderLines << "DLLEXPORT double getVariable(char* component, char* port, size_t idx);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    fmuHeaderLines << "}";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "#endif // HOPSANFMU_H";
//    fmuHeaderFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.cpp");
//    //progressBar.setValue(5);


//    QTextStream fmuSourceStream(&fmuSourceFile);
//    QTextLineStream fmuSrcLines(fmuSourceStream);

//    fmuSrcLines << "#include <iostream>";
//    fmuSrcLines << "#include <assert.h>";
//    fmuSrcLines << "#include \"HopsanCore.h\"";
//    fmuSrcLines << "#include \"HopsanFMU.h\"";
//    //fmuSrcLines << "#include \"include/ComponentEssentials.h\"";
//    //fmuSrcLines << "#include \"include/ComponentUtilities.h\"";
//    fmuSrcLines << "";
//    fmuSrcLines << "static double fmu_time=0;";
//    fmuSrcLines << "static hopsan::ComponentSystem *spCoreComponentSystem;";
//    fmuSrcLines << "static std::vector<std::string> sComponentNames;";
//    fmuSrcLines << "hopsan::HopsanEssentials gHopsanCore;";
//    fmuSrcLines << "";
//    fmuSrcLines << "void initializeHopsanWrapper(char* filename)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    double startT;      //Dummy variable";
//    fmuSrcLines << "    double stopT;       //Dummy variable";
//    fmuSrcLines << "    gHopsanCore.loadExternalComponentLib(\"../componentLibraries/defaultLibrary/components/libdefaultComponentLibrary.so\");";
//    fmuSrcLines << "    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);\n";
//    fmuSrcLines << "    assert(spCoreComponentSystem);";
//    fmuSrcLines << "    spCoreComponentSystem->setDesiredTimestep(0.001);";           //!< @todo Time step should not be hard coded
//    fmuSrcLines << "    spCoreComponentSystem->initialize(0,10);";
//    fmuSrcLines << "";
//    fmuSrcLines << "    fmu_time = 0;";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void simulateOneStep()";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    if(spCoreComponentSystem->checkModelBeforeSimulation())";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        double timestep = spCoreComponentSystem->getDesiredTimeStep();";
//    fmuSrcLines << "        spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);";
//    fmuSrcLines << "        fmu_time = fmu_time+timestep;\n";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "    else";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        std::cout << \"Simulation failed!\";";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "double getVariable(char* component, char* port, size_t idx)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->readNode(idx);";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void setVariable(char* component, char* port, size_t idx, double value)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    assert(spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port) != 0);";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->writeNode(idx, value);";
//    fmuSrcLines << "}";
//    fmuSourceFile.close();

//#ifdef WIN32
//    //progressBar.setLabelText("Writing to compile.bat");
//    //progressBar.setValue(6);



//    //Write the compilation script file
//    QTextStream clBatchStream(&clBatchFile);
////    if(gccCompiler)
////    {
//        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
//    clBatchStream << "g++ -DWRAPPERCOMPILATION -c -Wl,--rpath,'$ORIGIN/.' HopsanFMU.cpp -I./include\n";
//    clBatchStream << "g++ -shared -Wl,--rpath,'$ORIGIN/.' -o HopsanFMU.dll HopsanFMU.o -L./ -lHopsanCore";
////    }
////    else
////    {
////        //! @todo Check that Visual Studio is installed, and warn user if not
////        clBatchStream << "echo Compiling Visual Studio libraries...\n";
////        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
////        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
////        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
////    }
//    clBatchFile.close();
//#endif

//    //progressBar.setLabelText("Copying binary files");
//    //progressBar.setValue(7);


//    //Copy binaries to export directory
//#ifdef WIN32
//    QFile dllFile;
//    QFile libFile;
//    QFile expFile;
////    if(gccCompiler)
////    {
//        dllFile.setFileName(gDesktopHandler.getExecPath() + "HopsanCore.dll");
//        dllFile.copy(savePath + "/HopsanCore.dll");
////    }
////    else
////    {
////        //! @todo this seem a bit hardcoded
////        dllFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.dll");
////        dllFile.copy(savePath + "/HopsanCore.dll");
////        libFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.lib");
////        libFile.copy(savePath + "/HopsanCore.lib");
////        expFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.exp");
////        expFile.copy(savePath + "/HopsanCore.exp");
////    }
//#elif linux
//    QFile soFile;
//    soFile.setFileName(gDesktopHandler.getExecPath() + "libHopsanCore.so");
//    soFile.copy(savePath + "/libHopsanCore.so");
//#endif


//    //progressBar.setLabelText("Copying include files");
//    //progressBar.setValue(8);


//    //Copy include files to export directory
//    copyIncludeFilesToDir(savePath);


//    progressBar.setLabelText("Writing "+realModelName+".hmf");
//    progressBar.setValue(9);


//    //Save model to hmf in export directory
//    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
//    QDomDocument domDocument;
//    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, "0");
//    saveToDomElement(hmfRoot);
//    const int IndentSize = 4;
//    QFile xmlhmf(savePath + "/" + mModelFileInfo.fileName());
//    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to open "+savePath+"/"+mModelFileInfo.fileName()+" for writing.");
//        return;
//    }
//    QTextStream out(&xmlhmf);
//    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//    domDocument.save(out, IndentSize);

//    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
//    pCoreAccess->generateToFmu(savePath, this);
//    delete(pCoreAccess);

//}


void SystemContainer::exportToSimulink()
{
    QDialog *pExportDialog = new QDialog(gpMainWindow);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
    pMSVC2008RadioButton->setChecked(true);
    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
    pCompilerLayout->addWidget(pMSVC2008RadioButton);
    pCompilerLayout->addWidget(pMSVC2010RadioButton);
    pCompilerLayout->addStretch(1);
    pCompilerGroupBox->setLayout(pCompilerLayout);

    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
    p32bitRadioButton->setChecked(true);
    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
    pArchitectureLayout->addWidget(p32bitRadioButton);
    pArchitectureLayout->addWidget(p64bitRadioButton);
    pArchitectureLayout->addStretch(1);
    pArchitectureGroupBox->setLayout(pArchitectureLayout);

    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
    pExportDialogLayout->addWidget(pCompilerGroupBox);
    pExportDialogLayout->addWidget(pArchitectureGroupBox);
    pExportDialogLayout->addWidget(pExportDialogLabel2);
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


        //Open file dialog and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Simulink Source Files"),
                                                    gConfig.getSimulinkExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gConfig.setSimulinkExportDir(file.absolutePath());



    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
        //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
    saveToDomElement(hmfRoot);
    QFile xmlhmf(savePath + "/" + fileName);
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, XMLINDENTATION);
    xmlhmf.close();


    int compiler;
    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=0;
    }
    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=1;
    }
    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=2;
    }
    else if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=3;
    }


    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToSimulink(savePath, this, pDisablePortLabels->isChecked(), compiler);
    delete(pCoreAccess);


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
    delete(pMSVC2008RadioButton);
    delete(pMSVC2010RadioButton);
    delete(p32bitRadioButton);
    delete(p64bitRadioButton);
}


void SystemContainer::exportToSimulinkCoSim()
{
    QDialog *pExportDialog = new QDialog(gpMainWindow);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
    pMSVC2008RadioButton->setChecked(true);
    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
    pCompilerLayout->addWidget(pMSVC2008RadioButton);
    pCompilerLayout->addWidget(pMSVC2010RadioButton);
    pCompilerLayout->addStretch(1);
    pCompilerGroupBox->setLayout(pCompilerLayout);

    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
    p32bitRadioButton->setChecked(true);
    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
    pArchitectureLayout->addWidget(p32bitRadioButton);
    pArchitectureLayout->addWidget(p64bitRadioButton);
    pArchitectureLayout->addStretch(1);
    pArchitectureGroupBox->setLayout(pArchitectureLayout);

    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
    pExportDialogLayout->addWidget(pCompilerGroupBox);
    pExportDialogLayout->addWidget(pArchitectureGroupBox);
    pExportDialogLayout->addWidget(pExportDialogLabel2);
    pExportDialogLayout->addWidget(pDisablePortLabels);
    pExportDialogLayout->addWidget(pExportButtonBox);
    pExportDialog->setLayout(pExportDialogLayout);

    connect(pExportButtonBox, SIGNAL(accepted()), pExportDialog, SLOT(accept()));
    connect(pExportButtonBox, SIGNAL(rejected()), pExportDialog, SLOT(reject()));

    if(pExportDialog->exec() == QDialog::Rejected)
    {
        return;
    }

    QString fileName;
    if(!mModelFileInfo.fileName().isEmpty())
    {
        fileName = mModelFileInfo.fileName();
    }
    else
    {
        fileName = "untitled.hmf";
    }

        //Open file dialog and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Simulink Source Files"),
                                                    gConfig.getSimulinkExportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gConfig.setSimulinkExportDir(file.absolutePath());




    int compiler;
    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=0;
    }
    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=1;
    }
    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        compiler=2;
    }
    else if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        compiler=3;
    }


    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
    pCoreAccess->generateToSimulinkCoSim(savePath, this, pDisablePortLabels->isChecked());
    delete(pCoreAccess);


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
    delete(pMSVC2008RadioButton);
    delete(pMSVC2010RadioButton);
    delete(p32bitRadioButton);
    delete(p64bitRadioButton);
}




//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
void SystemContainer::setModelFileInfo(QFile &rFile)
{
    this->mModelFileInfo.setFile(rFile);
}


void SystemContainer::loadParameterFile()
{
    qDebug() << "loadParameterFile()";
    QString parameterFileName = QFileDialog::getOpenFileName(gpMainWindow, tr("Load Parameter File"),
                                                         gConfig.getLoadModelDir(),
                                                         tr("Hopsan Parameter Files (*.hpf *.xml)"));
    if(!parameterFileName.isEmpty())
    {
        mpCoreSystemAccess->loadParameterFile(parameterFileName);
        QFileInfo fileInfo = QFileInfo(parameterFileName);
        gConfig.setLoadModelDir(fileInfo.absolutePath());
    }
}



//! Function to set the time step of the current system
void SystemContainer::setTimeStep(const double timeStep)
{
    mpCoreSystemAccess->setDesiredTimeStep(timeStep);
    this->hasChanged();
}

//! Returns the time step value of the current project.
double SystemContainer::getTimeStep()
{
    return mpCoreSystemAccess->getDesiredTimeStep();
}

//! @brief Check if the system inherits timestep from its parent
bool SystemContainer::doesInheritTimeStep()
{
    return mpCoreSystemAccess->doesInheritTimeStep();
}


//! Returns the number of samples value of the current project.
//! @see setNumberOfLogSamples(double)
size_t SystemContainer::getNumberOfLogSamples()
{
    return mNumberOfLogSamples;
}


//! Sets the number of samples value for the current project
//! @see getNumberOfLogSamples()
void SystemContainer::setNumberOfLogSamples(size_t nSamples)
{
    mNumberOfLogSamples = nSamples;
}


OptimizationSettings::OptimizationSettings()
{
    //Defaulf values
    mNiter=100;
    mNsearchp=8;
    mRefcoeff=1.3;
    mRandfac=.3;
    mForgfac=0.0;
    mFunctol=.00001;
    mPartol=.0001;
    mPlot=true;
    mSavecsv=false;
    mlogPar = false;
}
