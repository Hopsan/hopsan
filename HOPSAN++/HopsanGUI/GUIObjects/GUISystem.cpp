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
#include "Widgets/MessageWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Dialogs/ContainerPropertiesDialog.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIWidgets.h"
#include "Widgets/PyDockWidget.h"
#include "Configuration.h"
#include "GUIContainerObject.h"

SystemContainer::SystemContainer(QPointF position, qreal rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
    : ContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpParentProjectTab = pParentContainer->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
SystemContainer::SystemContainer(ProjectTab *parentProjectTab, QGraphicsItem *pParent)
    : ContainerObject(QPointF(0,0), 0, 0, DESELECTED, USERGRAPHICS, 0, pParent)
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
void SystemContainer::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    ModelObject::saveCoreDataToDomElement(rDomElement);

    if (mLoadType == "EXTERNAL" )
    {
        //This information should ONLY be used to indicate that a system is external, it SHOULD NOT be included in the actual external system
        //If it would be, the load function will fail
        rDomElement.setAttribute( HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentContainerObject->getModelFileInfo().absolutePath()) );
    }

    if (mLoadType != "EXTERNAL" )
    {
        appendSimulationTimeTag(rDomElement, mpParentProjectTab->getStartTime().toDouble(), this->getTimeStep(), mpParentProjectTab->getStopTime().toDouble(), this->doesInheritTimeStep());

        PlotData::AliasMapT::iterator ita;
        QDomElement xmlAliases = appendDomElement(rDomElement, HMF_ALIASES);
        PlotData::AliasMapT aliasMap = getPlotDataPtr()->getPlotAliasMap();
        for(ita=aliasMap.begin(); ita!=aliasMap.end(); ++ita)
        {
            QDomElement aliasElement = appendDomElement(xmlAliases, HMF_ALIAS);
            aliasElement.setAttribute("alias", ita.key());
            aliasElement.setAttribute("component", ita.value().componentName);
            aliasElement.setAttribute("port",ita.value().portName);
            aliasElement.setAttribute("data",ita.value().dataName);
        }
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
        QList<VariableDescription> favVars = this->getPlotDataPtr()->getFavoriteVariableList();
        QList<VariableDescription>::iterator itf;
        for(itf = favVars.begin(); itf != favVars.end(); ++itf)
        {
            QDomElement favoriteElement = appendDomElement(xmlFavVars, HMF_FAVORITEVARIABLETAG);
            favoriteElement.setAttribute("componentname", (*itf).componentName);
            favoriteElement.setAttribute("portname", (*itf).portName);
            favoriteElement.setAttribute("dataname", (*itf).dataName);
            favoriteElement.setAttribute("dataunit", (*itf).dataUnit);
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
void SystemContainer::saveToDomElement(QDomElement &rDomElement)
{
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
    this->saveCoreDataToDomElement(xmlSubsystem);
    xmlSubsystem.setAttribute(HMF_LOGSAMPLES, mNumberOfLogSamples);

    // Save gui object stuff
    this->saveGuiDataToDomElement(xmlSubsystem);

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save subcomponents and subsystems
        QDomElement xmlObjects = appendDomElement(xmlSubsystem, HMF_OBJECTS);
        ModelObjectMapT::iterator it;
        for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlObjects);
        }

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

//! @brief Loads a System from an XML DOM Element
//! @param[in] rDomElement The element to load from
void SystemContainer::loadFromDomElement(QDomElement &rDomElement)
{
    double hmfVersion = rDomElement.parentNode().toElement().attribute("hmfversion").toDouble();

    if(hmfVersion <= 0.2 && hmfVersion != 0.0)
    {
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("Model file is saved with Hopsan version 0.2 or older. Full compatibility is not guaranteed.");
    }
    else if(hmfVersion != QString(HMF_VERSIONNUM).toDouble() && hmfVersion != 0.0)
    {
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("Model file is saved with an older version of Hopsan, but versions are compatible.");
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
            verifyHmfSubComponentCompatibility(xmlSubObject, hmfVersion);
            ModelObject* pObj = loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            if(pObj == NULL)
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage(QString("Model contains component from a library that has not been loaded. TypeName: ") +
                                                                    xmlSubObject.attribute(HMF_TYPENAME) + QString(", Name: ") + xmlSubObject.attribute(HMF_NAMETAG));

                // Insert missing component dummy instead
                xmlSubObject.setAttribute(HMF_TYPENAME, "MissingComponent");
                pObj = loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            }
            else
            {



                //! @deprecated This StartValue load code is only kept for upconverting old files, we should keep it here until we have some other way of upconverting old formats
                //Load start values //Is not needed, start values are saved as ordinary parameters! This code snippet can probably be removed.
                QDomElement xmlStartValues = xmlSubObject.firstChildElement(HMF_STARTVALUES);
                QDomElement xmlStartValue = xmlStartValues.firstChildElement(HMF_STARTVALUE);
                while (!xmlStartValue.isNull())
                {
                    loadStartValue(xmlStartValue, pObj, NOUNDO);
                    xmlStartValue = xmlStartValue.nextSiblingElement(HMF_STARTVALUE);
                }
            }

            if(pObj->getTypeName().startsWith("CppComponent"))
            {
                recompileCppComponents(pObj);
            }

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }

        //3. Load all text box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_TEXTBOXWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextBoxWidget(xmlSubObject, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTBOXWIDGETTAG);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //6. Load all systemports
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadContainerPortObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMPORTTAG);
        }

        //7. Load all connectors
        QDomElement xmlConnections = rDomElement.firstChildElement(HMF_CONNECTIONS);
        xmlSubObject = xmlConnections.firstChildElement(HMF_CONNECTORTAG);
        QList<QDomElement> failedConnections;
        while (!xmlSubObject.isNull())
        {
            if(!loadConnector(xmlSubObject, this, NOUNDO))
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
            if(!loadConnector(failedConnections.first(), this, NOUNDO))
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
        gpMainWindow->mpMessageWidget->printGUIWarningMessage("A system you tried to load is taged as an external system, but the ContainerSystem load function only loads embeded systems");
    }
}


void SystemContainer::saveToWrappedCode()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString filePath;
    QFileInfo fileInfo;
    QFile file;
    filePath = QFileDialog::getSaveFileName(gpMainWindow, tr("Export Project to HopsanRT Wrapper Code"),
                                            fileDialogSaveDir.currentPath(),
                                            tr("Text file (*.txt)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel
    fileInfo.setFile(filePath);
    file.setFileName(fileInfo.filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open file for writing: " + filePath);
        return;
    }
    QTextStream fileStream(&file);  //Create a QTextStream object to stream the content of file

    //Create lists for input and output interface components
    QStringList inputs;
    QStringList outputs;
    QStringList mechCinterfaces;
    QStringList mechQinterfaces;
    QStringList hydCinterfaces;
    QStringList hydQinterfaces;
    ModelObjectMapT::iterator it;
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SignalInputInterface")
        {
            inputs.append(it.value()->getName());
        }
        else if(it.value()->getTypeName() == "SignalOutputInterface")
        {
            outputs.append(it.value()->getName());
        }
        else if(it.value()->getTypeName() == "MechanicInterfaceC")
        {
            mechCinterfaces.append(it.value()->getName());
        }
        else if(it.value()->getTypeName() == "MechanicInterfaceQ")
        {
            mechQinterfaces.append(it.value()->getName());
        }
        else if(it.value()->getTypeName() == "HydraulicInterfaceC")
        {
            hydCinterfaces.append(it.value()->getName());
        }
        else if(it.value()->getTypeName() == "HydraulicInterfaceQ")
        {
            hydQinterfaces.append(it.value()->getName());
        }
    }



        //Write initial comment
    fileStream << "// Code from exported Hopsan model. This can be used in conjunction with HopsanCore by using HopsanWrapper. Subsystems probably don't work.\n\n";
    fileStream << "\n";
    fileStream << "#include \"hopsanrt-wrapper.h\"\n";
    fileStream << "#include \"SIT_API.h\"\n";
    fileStream << "#include \"model.h\"\n";
    fileStream << "#include <stddef.h>\n";
    fileStream << "#include <math.h>\n";
    fileStream << "#include \"codegen.c\"\n";
    fileStream << "\n";
    fileStream << "#define rtDBL	0\n";
    fileStream << "\n";
    fileStream << "extern Parameters rtParameter[2];\n";
    fileStream << "extern long READSIDE;\n";
    fileStream << "\n";
    fileStream << "#define readParam rtParameter[READSIDE]\n";
    fileStream << "\n";
    fileStream << "typedef struct \n";
    fileStream << "{\n";
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = inputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+";\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"C;\n";
        fileStream << "    double "+tempString+"Zc;\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"F;\n";
        fileStream << "    double "+tempString+"X;\n";
        fileStream << "    double "+tempString+"V;\n";
        fileStream << "    double "+tempString+"M;\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"C;\n";
        fileStream << "    double "+tempString+"Zc;\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"P;\n";
        fileStream << "    double "+tempString+"Q;\n";
    }
    fileStream << "} Inports;\n";
    fileStream << "\n";
    fileStream << "typedef struct\n";
    fileStream << "{\n";
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = outputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+";\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"F;\n";
        fileStream << "    double "+tempString+"X;\n";
        fileStream << "    double "+tempString+"V;\n";
        fileStream << "    double "+tempString+"M;\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"C;\n";
        fileStream << "    double "+tempString+"Zc;\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"P;\n";
        fileStream << "    double "+tempString+"Q;\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    double "+tempString+"C;\n";
        fileStream << "    double "+tempString+"Zc;\n";
    }
    fileStream << "} Outports;\n";
    fileStream << "\n";
    fileStream << "typedef struct\n";
    fileStream << "{\n";
    fileStream << "    double Time;\n";
    fileStream << "} Signals;\n";
    fileStream << "\n";
    fileStream << "Inports rtInport;\n";
    fileStream << "Outports rtOutport;\n";
    fileStream << "Signals rtSignal;\n";
    fileStream << "\n";
    fileStream << "long SetValueByDataType(void* ptr, int subindex, double value, int type)\n";
    fileStream << "{\n";
    fileStream << "    switch (type)\n";
    fileStream << "    {\n";
    fileStream << "        case rtDBL:\n";
    fileStream << "        ((double *)ptr)[subindex] = (double)value;\n";
    fileStream << "        return NI_OK;\n";
    fileStream << "    }\n";
    fileStream << "    return NI_ERROR;\n";
    fileStream << "}\n";
    fileStream << "\n";
    fileStream << "double GetValueByDataType(void* ptr, int subindex, int type)\n";
    fileStream << "{\n";
    fileStream << "    switch (type)\n";
    fileStream << "    {\n";
    fileStream << "        case rtDBL:\n";
    fileStream << "        return (double)(((double *)ptr)[subindex]);\n";
    fileStream << "    }\n";
    fileStream << "    return 0x7FFFFFFFFFFFFFFF; /* NAN */\n";
    fileStream << "}\n";
    fileStream << "\n";
    fileStream << "const long ParameterSize = 1;\n";
    fileStream << "const ParameterAttributes rtParamAttribs[] = \n";
    fileStream << "{\n";
    fileStream << "        { \"HopsanRT/sine/Amplitude\", offsetof(Parameters, HopsanRT_sine_Amp), rtDBL, 1, 1}\n";
    fileStream << "};\n";
    fileStream << "\n";
    fileStream << "const Parameters initParams = {0.0 /*time*/};\n";
    fileStream << "\n";
    fileStream << "const long SignalSize = 1;\n";
    fileStream << "const SignalAttributes rtSignalAttribs[] = \n";
    fileStream << "{\n";
    fileStream << "    { \"HopsanRT/Time\", 0, \"Time\", &rtSignal.Time, rtDBL, 1, 1}\n";
    fileStream << "};\n";
    fileStream << "\n";
    fileStream << "const long InportSize = "+QString().setNum(inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size())+";\n";
    fileStream << "const ExtIOAttributes rtInportAttribs[] = \n";
    fileStream << "{\n";
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = inputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"\", 1, 1},\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"C\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Zc\", 1, 1},\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"F\", 1, 1},\n";
        fileStream << "    { \""+tempString+"X\", 1, 1},\n";
        fileStream << "    { \""+tempString+"V\", 1, 1},\n";
        fileStream << "    { \""+tempString+"M\", 1, 1},\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"P\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Q\", 1, 1},\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"C\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Zc\", 1, 1},\n";
    }
    fileStream << "};\n";
    fileStream << "\n";
    fileStream << "const long OutportSize = "+QString().setNum(outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size()+2*hydQinterfaces.size())+";\n";
    fileStream << "const ExtIOAttributes rtOutportAttribs[] = \n";
    fileStream << "{\n";
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = outputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"\", 1, 1},\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"F\", 1, 1},\n";
        fileStream << "    { \""+tempString+"X\", 1, 1},\n";
        fileStream << "    { \""+tempString+"V\", 1, 1},\n";
        fileStream << "    { \""+tempString+"M\", 1, 1},\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"C\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Zc\", 1, 1},\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"P\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Q\", 1, 1},\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    { \""+tempString+"C\", 1, 1},\n";
        fileStream << "    { \""+tempString+"Zc\", 1, 1},\n";
    }
    fileStream << "};\n";
    fileStream << "\n";
    fileStream << "const char * const ModelName = \"HopsanRT\";\n";
    fileStream << "const char * const build = \"5.0.1 SIT Custom DLL\";\n";
    fileStream << "\n";
    fileStream << "const double baserate = .001;\n";
    fileStream << "\n";
    fileStream << "long USER_Initialize() \n";
    fileStream << "{\n";
    fileStream << "    createSystem(1e-3);\n\n";
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        fileStream << "    addComponent(\"" << it.value()->getName() << "\", \"" << it.value()->getTypeName() << "\");\n";
    fileStream << "    \n";
    for(int i = 0; i != mSubConnectorList.size(); ++i)
        fileStream <<    "    connect(\"" << mSubConnectorList[i]->getStartComponentName() << "\", \"" << mSubConnectorList[i]->getStartPortName() <<
                      "\", \"" << mSubConnectorList[i]->getEndComponentName() << "\", \"" << mSubConnectorList[i]->getEndPortName() << "\");\n";
    fileStream << "    \n";
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        for(int i=0; i<it.value()->getParameterNames().size(); ++i)
            fileStream << "    setParameter(\"" << it.value()->getName() << "\", \"" << it.value()->getParameterNames().at(i) <<  "\", " << it.value()->getParameterValue(it.value()->getParameterNames().at(i)) << ");\n";
    fileStream << "    \n";
    fileStream << "    initSystem();\n";
    fileStream << "    rtSignal.Time = 0;\n";
    fileStream << "\n";
    fileStream << "    return NI_OK;\n";
    fileStream << "}\n";
    fileStream << "\n";
    fileStream << "void USER_TakeOneStep(double *inData, double *outData, double timestamp)\n";
    fileStream << "{\n";
    fileStream << "    rtSignal.Time += 0.001;\n";
    fileStream << "    if (inData)\n";
    fileStream << "    {\n";
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = inputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        rtInport."+tempString+" = inData["+QString().setNum(i)+"];\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        rtInport."+tempString+"C = inData["+QString().setNum(2*i+inputs.size())+"];\n";
        fileStream << "        rtInport."+tempString+"Zc = inData["+QString().setNum(2*i+1+inputs.size())+"];\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        rtInport."+tempString+"F = inData["+QString().setNum(4*i+inputs.size()+2*mechCinterfaces.size())+"];\n";
        fileStream << "        rtInport."+tempString+"X = inData["+QString().setNum(4*i+1+inputs.size()+2*mechCinterfaces.size())+"];\n";
        fileStream << "        rtInport."+tempString+"V = inData["+QString().setNum(4*i+2+inputs.size()+2*mechCinterfaces.size())+"];\n";
        fileStream << "        rtInport."+tempString+"M = inData["+QString().setNum(4*i+3+inputs.size()+2*mechCinterfaces.size())+"];\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        rtInport."+tempString+"C = inData["+QString().setNum(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n";
        fileStream << "        rtInport."+tempString+"Zc = inData["+QString().setNum(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size())+"];\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        rtInport."+tempString+"P = inData["+QString().setNum(2*i+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n";
        fileStream << "        rtInport."+tempString+"Q = inData["+QString().setNum(2*i+1+inputs.size()+2*mechCinterfaces.size()+4*mechQinterfaces.size()+2*hydCinterfaces.size())+"];\n";
    }
    fileStream << "    }\n";
    fileStream << "    \n";
    for(int i=0; i<inputs.size(); ++i)
    {
        QString tempString = inputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    writeNodeData(\""+inputs.at(i)+"\", \"out\", 0, rtInport."+tempString+");\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n";
        fileStream << "    writeNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"F);\n";
        fileStream << "    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 2, rtInport."+tempString+"X);\n";
        fileStream << "    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"V);\n";
        fileStream << "    writeNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 5, rtInport."+tempString+"M);\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 3, rtInport."+tempString+"C);\n";
        fileStream << "    writeNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 4, rtInport."+tempString+"Zc);\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 1, rtInport."+tempString+"P);\n";
        fileStream << "    writeNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 0, rtInport."+tempString+"Q);\n";
    }
    fileStream << "    simulateOneTimestep(rtSignal.Time);\n";
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = outputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    rtOutport."+tempString+" = readNodeData(\""+outputs.at(i)+"\", \"in\", 0);\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    rtOutport."+tempString+"F = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 1);\n";
        fileStream << "    rtOutport."+tempString+"X = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 2);\n";
        fileStream << "    rtOutport."+tempString+"V = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 0);\n";
        fileStream << "    rtOutport."+tempString+"M = readNodeData(\""+mechCinterfaces.at(i)+"\", \"P1\", 5);\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    rtOutport."+tempString+"C = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 3);\n";
        fileStream << "    rtOutport."+tempString+"Zc = readNodeData(\""+mechQinterfaces.at(i)+"\", \"P1\", 4);\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    rtOutport."+tempString+"P = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 1);\n";
        fileStream << "    rtOutport."+tempString+"Q = readNodeData(\""+hydCinterfaces.at(i)+"\", \"P1\", 0);\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "    rtOutport."+tempString+"C = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 3);\n";
        fileStream << "    rtOutport."+tempString+"Zc = readNodeData(\""+hydQinterfaces.at(i)+"\", \"P1\", 4);\n";
    }
    fileStream << "    \n";
    fileStream << "    if (outData)\n";
    fileStream << "    {\n";
    for(int i=0; i<outputs.size(); ++i)
    {
        QString tempString = outputs.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        outData["+QString().setNum(i)+"] = rtOutport."+tempString+";\n";
    }
    for(int i=0; i<mechCinterfaces.size(); ++i)
    {
        QString tempString = mechCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        outData["+QString().setNum(4*i+outputs.size())+"] = rtOutport."+tempString+"F;\n";
        fileStream << "        outData["+QString().setNum(4*i+1+outputs.size())+"] = rtOutport."+tempString+"X;\n";
        fileStream << "        outData["+QString().setNum(4*i+2+outputs.size())+"] = rtOutport."+tempString+"V;\n";
        fileStream << "        outData["+QString().setNum(4*i+3+outputs.size())+"] = rtOutport."+tempString+"M;\n";
    }
    for(int i=0; i<mechQinterfaces.size(); ++i)
    {
        QString tempString = mechQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        outData["+QString().setNum(2*i+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"C;\n";
        fileStream << "        outData["+QString().setNum(2*i+1+outputs.size()+4*mechCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n";
    }
    for(int i=0; i<hydCinterfaces.size(); ++i)
    {
        QString tempString = hydCinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        outData["+QString().setNum(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"P;\n";
        fileStream << "        outData["+QString().setNum(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size())+"] = rtOutport."+tempString+"Q;\n";
    }
    for(int i=0; i<hydQinterfaces.size(); ++i)
    {
        QString tempString = hydQinterfaces.at(i);
        tempString.remove(" ");
        tempString.remove("-");
        fileStream << "        outData["+QString().setNum(2*i+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"C;\n";
        fileStream << "        outData["+QString().setNum(2*i+1+outputs.size()+4*mechCinterfaces.size()+2*mechQinterfaces.size()+2*hydCinterfaces.size())+"] = rtOutport."+tempString+"Zc;\n";
    }
    fileStream << "    }\n";
    fileStream << "}\n";
    fileStream << "\n";
    fileStream << "long USER_Finalize()\n";
    fileStream << "{\n";
    fileStream << "    return NI_OK;\n";
    fileStream << "}\n";

    file.close();
}


//void SystemContainer::createFMUSourceFiles()
//{
//    QDialog *pExportFmuDialog = new QDialog(gpMainWindow);
//    pExportFmuDialog->setWindowTitle("Export to Functional Mockup Interface");

//    QLabel *pExportFmuLabel = new QLabel(gpMainWindow->tr("This will create a Functional Mockup Unit of\ncurrent model. Please choose compiler:"), pExportFmuDialog);

//    mpExportFmuGccRadioButton = new QRadioButton(gpMainWindow->tr("GCC"), pExportFmuDialog);
//    mpExportFmuGccRadioButton->setChecked(true);
//    mpExportFmuMsvcRadioButton = new QRadioButton(gpMainWindow->tr("Microsoft Visual C"), pExportFmuDialog);

//    QPushButton *pOkButton = new QPushButton("Okay", pExportFmuDialog);
//    QPushButton *pCancelButton = new QPushButton("Cancel", pExportFmuDialog);

//    QGridLayout *pExportFmuLayout = new QGridLayout(pExportFmuDialog);
//    pExportFmuLayout->addWidget(pExportFmuLabel,            0, 0, 1, 2);
//    pExportFmuLayout->addWidget(mpExportFmuGccRadioButton,  1, 0, 1, 2);
//    pExportFmuLayout->addWidget(mpExportFmuMsvcRadioButton, 2, 0, 1, 2);
//    pExportFmuLayout->addWidget(pOkButton,                  3, 0, 1, 1);
//    pExportFmuLayout->addWidget(pCancelButton,              3, 1, 1, 1);

//    pExportFmuDialog->setLayout(pExportFmuLayout);

//    pExportFmuDialog->show();

//    connect(pOkButton,      SIGNAL(clicked()), pExportFmuDialog,    SLOT(close()));
//    connect(pOkButton,      SIGNAL(clicked()), this,                SLOT(createFMUSourceFilesFromDialog()));
//    connect(pCancelButton,  SIGNAL(clicked()), pExportFmuDialog,    SLOT(close()));
//}


void SystemContainer::createFMUSourceFiles()
{
    //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
                                                    fileDialogSaveDir.currentPath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QDir saveDir;
    saveDir.setPath(savePath);
    saveDir.setFilter(QDir::NoDotAndDotDot);
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
    saveDir.setFilter(QDir::NoFilter);


    QProgressDialog progressBar(tr("Initializing"), QString(), 0, 0, gpMainWindow);
    progressBar.show();
    progressBar.setMaximum(10);
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Creating FMU"));
    progressBar.setMaximum(20);
    progressBar.setValue(0);


    //Tells if user selected the gcc compiler or not (= visual studio)
    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();


    //Write the FMU ID
    int random = rand() % 1000;
    QString randomString = QString().setNum(random);
    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?


    //Collect information about input ports
    QStringList inputVariables;
    QStringList inputComponents;
    QStringList inputPorts;
    QList<int> inputDatatypes;

    ModelObjectMapT::iterator it;
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SignalInputInterface")
        {
            inputVariables.append(it.value()->getName().remove(' '));
            inputComponents.append(it.value()->getName());
            inputPorts.append("out");
            inputDatatypes.append(0);
        }
    }


    //Collect information about output ports
    QStringList outputVariables;
    QStringList outputComponents;
    QStringList outputPorts;
    QList<int> outputDatatypes;

    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SignalOutputInterface")
        {
            outputVariables.append(it.value()->getName().remove(' '));
            outputComponents.append(it.value()->getName());
            outputPorts.append("in");
            outputDatatypes.append(0);
        }
    }


    //Create file objects for all files that shall be created
    QFile modelSourceFile;
    QString modelName = getModelFileInfo().fileName();
    modelName.chop(4);
    QString realModelName = modelName;          //Actual model name (used for hmf file)
    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open " + modelName + ".c for writing.");
        return;
    }

    QFile modelDescriptionFile;
    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open modelDescription.xml for writing.");
        return;
    }

    QFile fmuHeaderFile;
    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open HopsanFMU.h for writing.");
        return;
    }

    QFile fmuSourceFile;
    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open HopsanFMU.cpp for writing.");
        return;
    }

#ifdef win32
    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }
#endif

    progressBar.setLabelText("Writing modelDescription.xml");
    progressBar.setValue(1);

    QTextStream modelDescriptionStream(&modelDescriptionFile);
    modelDescriptionStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";       //!< @todo Encoding, should it be UTF-8?
    modelDescriptionStream << "<fmiModelDescription\n";
    modelDescriptionStream << "  fmiVersion=\"1.0\"\n";
    modelDescriptionStream << "  modelName=\"" << modelName << "\"\n";               //!< @todo What's the difference between name and identifier?
    modelDescriptionStream << "  modelIdentifier=\"" << modelName << "\"\n";
    modelDescriptionStream << "  guid=\"" << ID << "\"\n";
    modelDescriptionStream << "  numberOfContinuousStates=\"" << inputVariables.size() + outputVariables.size() << "\"\n";
    modelDescriptionStream << "  numberOfEventIndicators=\"0\">\n";
    modelDescriptionStream << "<ModelVariables>\n";
    int i, j;
    for(i=0; i<inputVariables.size(); ++i)
    {
        QString refString = QString().setNum(i);
        modelDescriptionStream << "  <ScalarVariable name=\"" << inputVariables.at(i) << "\" valueReference=\""+refString+"\" description=\"input variable\" causality=\"input\">\n";
        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
        modelDescriptionStream << "  </ScalarVariable>\n";
    }
    for(j=0; j<outputVariables.size(); ++j)
    {
        QString refString = QString().setNum(i+j);
        modelDescriptionStream << "  <ScalarVariable name=\"" << outputVariables.at(j) << "\" valueReference=\""+refString+"\" description=\"output variable\" causality=\"output\">\n";
        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
        modelDescriptionStream << "  </ScalarVariable>\n";
    }
    modelDescriptionStream << "</ModelVariables>\n";
    modelDescriptionStream << "</fmiModelDescription>\n";
    modelDescriptionFile.close();


    progressBar.setLabelText("Writing " + modelName + ".c");
    progressBar.setValue(2);


    QTextStream modelSourceStream(&modelSourceFile);
    modelSourceStream << "// Define class name and unique id\n";
    modelSourceStream << "    #define MODEL_IDENTIFIER " << modelName << "\n";
    modelSourceStream << "    #define MODEL_GUID \"" << ID << "\"\n\n";
    modelSourceStream << "    // Define model size\n";
    modelSourceStream << "    #define NUMBER_OF_REALS " << inputVariables.size() + outputVariables.size() << "\n";
    modelSourceStream << "    #define NUMBER_OF_INTEGERS 0\n";
    modelSourceStream << "    #define NUMBER_OF_BOOLEANS 0\n";
    modelSourceStream << "    #define NUMBER_OF_STRINGS 0\n";
    modelSourceStream << "    #define NUMBER_OF_STATES "<< inputVariables.size() + outputVariables.size() << "\n";        //!< @todo Does number of variables equal number of states?
    modelSourceStream << "    #define NUMBER_OF_EVENT_INDICATORS 0\n\n";
    modelSourceStream << "    // Include fmu header files, typedefs and macros\n";
    modelSourceStream << "    #include \"fmuTemplate.h\"\n";
    modelSourceStream << "    #include \"HopsanFMU.h\"\n\n";
    modelSourceStream << "    // Define all model variables and their value references\n";
    for(i=0; i<inputVariables.size(); ++i)
        modelSourceStream << "    #define " << inputVariables.at(i) << "_ " << i << "\n\n";
    for(j=0; j<outputVariables.size(); ++j)
        modelSourceStream << "    #define " << outputVariables.at(j) << "_ " << j+i << "\n\n";
    modelSourceStream << "    // Define state vector as vector of value references\n";
    modelSourceStream << "    #define STATES { ";
    i=0;
    j=0;
    if(!inputVariables.isEmpty())
    {
        modelSourceStream << inputVariables.at(0) << "_";
        ++i;
    }
    else if(!outputVariables.isEmpty())
    {
        modelSourceStream << outputVariables.at(0) << "_";
        ++j;
    }
    for(; i<inputVariables.size(); ++i)
        modelSourceStream << ", " << inputVariables.at(i) << "_";
    for(; j<outputVariables.size(); ++j)
        modelSourceStream << ", " << outputVariables.at(j) << "_";
    modelSourceStream << " }\n\n";
    modelSourceStream << "    //Set start values\n";
    modelSourceStream << "    void setStartValues(ModelInstance *comp) \n";
    modelSourceStream << "    {\n";
    for(i=0; i<inputVariables.size(); ++i)
        modelSourceStream << "        r(" << inputVariables.at(i) << "_) = 0;\n";        //!< Fix start value handling
    for(j=0; j<outputVariables.size(); ++j)
        modelSourceStream << "        r(" << outputVariables.at(j) << "_) = 0;\n";        //!< Fix start value handling
    modelSourceStream << "    }\n\n";
    modelSourceStream << "    //Initialize\n";
    modelSourceStream << "    void initialize(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
    modelSourceStream << "    {\n";
    modelSourceStream << "        initializeHopsanWrapper(\""+realModelName+".hmf\");\n";
    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";
    modelSourceStream << "    }\n\n";
    modelSourceStream << "    //Return variable of real type\n";
    modelSourceStream << "    fmiReal getReal(ModelInstance* comp, fmiValueReference vr)\n";
    modelSourceStream << "    {\n";
    modelSourceStream << "        switch (vr) \n";
    modelSourceStream << "       {\n";
    for(i=0; i<inputVariables.size(); ++i)
        modelSourceStream << "           case " << inputVariables.at(i) << "_: return getVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ");\n";
    for(j=0; j<outputVariables.size(); ++j)
        modelSourceStream << "           case " << outputVariables.at(j) << "_: return getVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ");\n";
    modelSourceStream << "            default: return 1;\n";
    modelSourceStream << "       }\n";
    modelSourceStream << "    }\n\n";
    modelSourceStream << "    void setReal(ModelInstance* comp, fmiValueReference vr, fmiReal value)\n";
    modelSourceStream << "    {\n";
    modelSourceStream << "        switch (vr) \n";
    modelSourceStream << "       {\n";
    for(i=0; i<inputVariables.size(); ++i)
        modelSourceStream << "           case " << inputVariables.at(i) << "_: setVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ", value);\n";
    for(j=0; j<outputVariables.size(); ++j)
        modelSourceStream << "           case " << outputVariables.at(j) << "_: setVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ", value);\n";
    modelSourceStream << "            default: return;\n";
    modelSourceStream << "       }\n";
    modelSourceStream << "    }\n\n";
    modelSourceStream << "    //Update at time event\n";
    modelSourceStream << "    void eventUpdate(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
    modelSourceStream << "    {\n";
    modelSourceStream << "        simulateOneStep();\n";
    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";      //!< @todo Hardcoded timestep
    modelSourceStream << "    }\n\n";
    modelSourceStream << "    // Include code that implements the FMI based on the above definitions\n";
    modelSourceStream << "    #include \"fmuTemplate.c\"\n";
    modelSourceFile.close();


    progressBar.setLabelText("Writing HopsanFMU.h");
    progressBar.setValue(4);


    QTextStream fmuHeaderStream(&fmuHeaderFile);
    QTextLineStream fmuHeaderLines(fmuHeaderStream);
    fmuHeaderLines << "#ifndef HOPSANFMU_H";
    fmuHeaderLines << "#define HOPSANFMU_H";
    fmuHeaderLines << "";
    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
    //fmuHeaderLines << "    #define DLLEXPORT __declspec(dllexport)";
    fmuHeaderLines << "    extern \"C\" {";
    fmuHeaderLines << "#else";
    fmuHeaderLines << "    #define DLLEXPORT";
    fmuHeaderLines << "#endif";
    fmuHeaderLines << "";
    fmuHeaderLines << "DLLEXPORT void initializeHopsanWrapper(char* filename);";
    fmuHeaderLines << "DLLEXPORT void simulateOneStep();";
    fmuHeaderLines << "DLLEXPORT double getVariable(char* component, char* port, size_t idx);";
    fmuHeaderLines << "";
    fmuHeaderLines << "DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);";
    fmuHeaderLines << "";
    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
    fmuHeaderLines << "}";
    fmuHeaderLines << "#endif";
    fmuHeaderLines << "#endif // HOPSANFMU_H";
    fmuHeaderFile.close();


    progressBar.setLabelText("Writing HopsanFMU.cpp");
    progressBar.setValue(5);


    QTextStream fmuSourceStream(&fmuSourceFile);
    QTextLineStream fmuSrcLines(fmuSourceStream);

    fmuSrcLines << "#include <iostream>";
    fmuSrcLines << "#include <assert.h>";
    fmuSrcLines << "#include \"HopsanCore.h\"";
    fmuSrcLines << "#include \"HopsanFMU.h\"";
    //fmuSrcLines << "#include \"include/ComponentEssentials.h\"";
    //fmuSrcLines << "#include \"include/ComponentUtilities.h\"";
    fmuSrcLines << "";
    fmuSrcLines << "static double fmu_time=0;";
    fmuSrcLines << "static hopsan::ComponentSystem *spCoreComponentSystem;";
    fmuSrcLines << "static std::vector<std::string> sComponentNames;";
    fmuSrcLines << "hopsan::HopsanEssentials gHopsanCore;";
    fmuSrcLines << "";
    fmuSrcLines << "void initializeHopsanWrapper(char* filename)";
    fmuSrcLines << "{";
    fmuSrcLines << "    double startT;      //Dummy variable";
    fmuSrcLines << "    double stopT;       //Dummy variable";
    fmuSrcLines << "    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);\n";
    fmuSrcLines << "    assert(spCoreComponentSystem);";
    fmuSrcLines << "    spCoreComponentSystem->setDesiredTimestep(0.001);";           //!< @todo Time step should not be hard coded
    fmuSrcLines << "    spCoreComponentSystem->initialize(0,10);";
    fmuSrcLines << "}";
    fmuSrcLines << "";
    fmuSrcLines << "void simulateOneStep()";
    fmuSrcLines << "{";
    fmuSrcLines << "    if(spCoreComponentSystem->checkModelBeforeSimulation())";
    fmuSrcLines << "    {";
    fmuSrcLines << "        double timestep = spCoreComponentSystem->getDesiredTimeStep();";
    fmuSrcLines << "        spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);";
    fmuSrcLines << "        fmu_time = fmu_time+timestep;\n";
    fmuSrcLines << "    }";
    fmuSrcLines << "    else";
    fmuSrcLines << "    {";
    fmuSrcLines << "        std::cout << \"Simulation failed!\";";
    fmuSrcLines << "    }";
    fmuSrcLines << "}";
    fmuSrcLines << "";
    fmuSrcLines << "double getVariable(char* component, char* port, size_t idx)";
    fmuSrcLines << "{";
    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->readNode(idx);";
    fmuSrcLines << "}";
    fmuSrcLines << "";
    fmuSrcLines << "void setVariable(char* component, char* port, size_t idx, double value)";
    fmuSrcLines << "{";
    fmuSrcLines << "    assert(spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port) != 0);";
    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->writeNode(idx, value);";
    fmuSrcLines << "}";
    fmuSourceFile.close();

#ifdef win32
    progressBar.setLabelText("Writing to compile.bat");
    progressBar.setValue(6);



    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
//    if(gccCompiler)
//    {
        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
    clBatchStream << "g++ -DWRAPPERCOMPILATION -c -Wl,--rpath,'$ORIGIN/.' HopsanFMU.cpp -I./include\n";
    clBatchStream << "g++ -shared -Wl,--rpath,'$ORIGIN/.' -o HopsanFMU.dll HopsanFMU.o -L./ -lHopsanCore";
//    }
//    else
//    {
//        //! @todo Check that Visual Studio is installed, and warn user if not
//        clBatchStream << "echo Compiling Visual Studio libraries...\n";
//        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
//        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
//        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
//    }
    clBatchFile.close();
#endif

    progressBar.setLabelText("Copying binary files");
    progressBar.setValue(7);


    //Copy binaries to export directory
#ifdef win32
    QFile dllFile;
    QFile libFile;
    QFile expFile;
//    if(gccCompiler)
//    {
        dllFile.setFileName(gExecPath + "HopsanCore.dll");
        dllFile.copy(savePath + "/HopsanCore.dll");
//    }
//    else
//    {
//        //! @todo this seem a bit hardcoded
//        dllFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.dll");
//        dllFile.copy(savePath + "/HopsanCore.dll");
//        libFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.lib");
//        libFile.copy(savePath + "/HopsanCore.lib");
//        expFile.setFileName(QString(MSVC2008_X86_PATH) + "HopsanCore.exp");
//        expFile.copy(savePath + "/HopsanCore.exp");
//    }
#elif linux
    QFile soFile;
    soFile.setFileName(gExecPath + "libHopsanCore.so");
    soFile.copy(savePath + "/libHopsanCore.so");
#endif


    progressBar.setLabelText("Copying include files");
    progressBar.setValue(8);


    //Copy include files to export directory
    copyIncludeFilesToDir(savePath);


    progressBar.setLabelText("Writing "+realModelName+".hmf");
    progressBar.setValue(9);


    //Save model to hmf in export directory
    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, "0");
    saveToDomElement(hmfRoot);
    const int IndentSize = 4;
    QFile xmlhmf(savePath + "/" + mModelFileInfo.fileName());
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, IndentSize);


#ifdef win32
    progressBar.setLabelText("Compiling HopsanFMU.dll");
#elif linux
    progressBar.setLabelText("Compiling HopsanFMU.so");
#endif
    progressBar.setValue(11);


#ifdef win32
    //Execute HopsanFMU compile script
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");
    p.waitForFinished();
#elif linux
    QString gccCommand1 = "cd "+savePath+" && g++ -DWRAPPERCOMPILATION -fPIC -Wl,--rpath,'$ORIGIN/.' -c HopsanFMU.cpp -I./include\n";
    QString gccCommand2 = "cd "+savePath+" && g++ -shared -Wl,--rpath,'$ORIGIN/.' -o libHopsanFMU.so HopsanFMU.o -L./ -lHopsanCore";

    qDebug() << "Command 1 = " << gccCommand1;
    qDebug() << "Command 2 = " << gccCommand2;

    char line[130];
    gccCommand1 +=" 2>&1";
    FILE *fp = popen(  (const char *) gccCommand1.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + gccCommand1 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }

    gccCommand2 +=" 2>&1";
    fp = popen(  (const char *) gccCommand2.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + gccCommand2 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }
#endif


    progressBar.setLabelText("Copying compilation files");
    progressBar.setValue(14);


    //Copy FMI compilation files to export directory
#ifdef win32
    QFile buildFmuFile;
//    if(gccCompiler)
//    {
        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_gcc.bat");
//    }
//    else
//    {
//        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_vc.bat");
//    }
    buildFmuFile.copy(savePath + "/build_fmu.bat");
#endif
    QFile fmuModelFunctionsHFile(gExecPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmiModelTypesHFile(gExecPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");

#ifdef win32
    progressBar.setLabelText("Compiling "+modelName+".dll");
#elif linux
    progressBar.setLabelText("Compiling "+modelName+".so");
#endif
    progressBar.setValue(15);

#ifdef win32
    //Execute FMU compile script
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & build_fmu.bat me " + modelName);
    p.waitForFinished();
#elif linux
    gccCommand1 = "cd "+savePath+" && gcc -c -fPIC -Wl,--rpath,'$ORIGIN/.' "+modelName+".c";
    gccCommand2 = "cd "+savePath+" && gcc -shared -Wl,--rpath,'$ORIGIN/.' -o "+modelName+".so "+modelName+".o -L./ -lHopsanFMU";

    qDebug() << "Command 1 = " << gccCommand1;
    qDebug() << "Command 2 = " << gccCommand2;

    gccCommand1 +=" 2>&1";
    fp = popen(  (const char *) gccCommand1.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + gccCommand1 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }

    gccCommand2 +=" 2>&1";
    fp = popen(  (const char *) gccCommand2.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + gccCommand2 + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }
#endif

    progressBar.setLabelText("Sorting files");
    progressBar.setValue(18);


#ifdef win32
    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
    dllFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.dll");
//    if(!gccCompiler)
//    {
//        libFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.lib");
//    }
    QFile hopsanFMUdllFile(savePath + "/HopsanFMU.dll");
    hopsanFMUdllFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.dll");
    QFile hopsanFMUlibFile(savePath + "/HopsanFMU.lib");
    hopsanFMUlibFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.lib");
#elif linux && __i386__
    saveDir.mkpath("fmu/binaries/linux32");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux32/" + modelName + ".so");
    QFile hopsanFMUsoFile(savePath + "/libHopsanFMU.so");
    hopsanFMUsoFile.copy(savePath + "/fmu/binaries/linux32/libHopsanFMU.so");
#elif linux && __x86_64__
    saveDir.mkpath("fmu/binaries/linux64");
    saveDir.mkpath("fmu/resources");
    QFile modelSoFile(savePath + "/" + modelName + ".so");
    modelSoFile.copy(savePath + "/fmu/binaries/linux64/" + modelName + ".so");
    QFile hopsanFMUsoFile(savePath + "/libHopsanFMU.so");
    hopsanFMUsoFile.copy(savePath + "/fmu/binaries/linux64/libHopsanFMU.so");
#endif
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/modelDescription.xml");

    QString fmuFileName = savePath + "/" + modelName + ".fmu";


    progressBar.setLabelText("Compressing files");
    progressBar.setValue(19);


#ifdef win32
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/modelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources");
    p.waitForFinished();
    qDebug() << "Called: " << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/modelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources";
#elif linux
    QString command = "cd "+savePath+"/fmu && zip -r ../"+modelName+".fmu *";
    qDebug() << "Command = " << command;
    command +=" 2>&1";
    fp = popen(  (const char *) command.toStdString().c_str(), "r");
    if ( !fp )
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not execute '" + command + "'! err=%d");
        return;
    }
    else
    {
        while ( fgets( line, sizeof line, fp))
        {
            gpMainWindow->mpMessageWidget->printGUIInfoMessage((const QString &)line);
        }
    }
#endif

    progressBar.setLabelText("Cleaning up");
    progressBar.setValue(20);


    //Clean up temporary files
//    saveDir.setPath(savePath);
//    saveDir.remove("compile.bat");
//    saveDir.remove("HopsanFMU.cpp");
//    saveDir.remove("HopsanFMU.obj");
//    //saveDir.remove("HopsanFMU.lib");
//    //saveDir.remove("HopsanCore.lib");
//    saveDir.remove("HopsanCore.exp");
//    saveDir.remove("build_fmu.bat");
//    saveDir.remove("fmiModelFunctions.h");
//    saveDir.remove("fmiModelTypes.h");
//    saveDir.remove("fmuTemplate.c");
//    saveDir.remove("fmuTemplate.h");
//    saveDir.remove(modelName + ".c");
//    saveDir.remove(modelName + ".exp");
//    //saveDir.remove(modelName + ".lib");
//    saveDir.remove(modelName + ".obj");
//    saveDir.remove("HopsanFMU.exp");
//    saveDir.remove("HopsanFMU.h");
//    removeDir(savePath + "/include");
//    removeDir(savePath + "/fmu");
}


void SystemContainer::createSimulinkSourceFiles()
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

    QProgressDialog progressBar(tr("Initializing"), QString(), 0, 0, gpMainWindow);
    progressBar.show();
    progressBar.setMaximum(10);
    progressBar.setWindowModality(Qt::WindowModal);
    progressBar.setWindowTitle(tr("Creating Simulink Source Files"));
    progressBar.setMaximum(10);
    progressBar.setValue(0);

    QStringList tunableParameters = this->getParameterNames();

    QStringList inputComponents;
    QStringList inputPorts;
    QStringList outputComponents;
    QStringList outputPorts;
    QStringList mechanicQComponents;
    QStringList mechanicQPorts;
    QStringList mechanicCComponents;
    QStringList mechanicCPorts;
    QStringList mechanicRotationalQComponents;
    QStringList mechanicRotationalQPorts;
    QStringList mechanicRotationalCComponents;
    QStringList mechanicRotationalCPorts;

    ModelObjectMapT::iterator it;
    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
    {
        if(it.value()->getTypeName() == "SignalInputInterface")
        {
            inputComponents.append(it.value()->getName());
            inputPorts.append("out");
        }
        else if(it.value()->getTypeName() == "SignalOutputInterface")
        {
            outputComponents.append(it.value()->getName());
            outputPorts.append("in");
        }
        else if(it.value()->getTypeName() == "MechanicInterfaceQ")
        {
            mechanicQComponents.append(it.value()->getName());
            mechanicQPorts.append("P1");
        }
        else if(it.value()->getTypeName() == "MechanicInterfaceC")
        {
            mechanicCComponents.append(it.value()->getName());
            mechanicCPorts.append("P1");
        }
        else if(it.value()->getTypeName() == "MechanicRotationalInterfaceQ")
        {
            mechanicRotationalQComponents.append(it.value()->getName());
            mechanicRotationalQPorts.append("P1");
        }
        else if(it.value()->getTypeName() == "MechanicRotationalInterfaceC")
        {
            mechanicRotationalCComponents.append(it.value()->getName());
            mechanicRotationalCPorts.append("P1");
        }
        //! @todo what about pneumatic and electric nodes
        //! @todo this should not be hardcoded
    }

    int nInputs = inputComponents.size();
    QString nInputsString;
    nInputsString.setNum(nInputs);

    int nOutputs = outputComponents.size();
    QString nOutputsString;
    nOutputsString.setNum(nOutputs);

    int nMechanicQ = mechanicQComponents.size();
    QString nMechanicQString;
    nMechanicQString.setNum(nMechanicQ);

    int nMechanicC = mechanicCComponents.size();
    QString nMechanicCString;
    nMechanicCString.setNum(nMechanicC);

    int nMechanicRotationalQ = mechanicRotationalQComponents.size();
    QString nMechanicRotationalQString;
    nMechanicRotationalQString.setNum(nMechanicRotationalQ);

    int nMechanicRotationalC = mechanicRotationalCComponents.size();
    QString nMechanicRotationalCString;
    nMechanicRotationalCString.setNum(nMechanicRotationalC);

    int nTotalInputs = nInputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2;
    QString nTotalInputsString;
    nTotalInputsString.setNum(nTotalInputs);

    int nTotalOutputs = nOutputs+nMechanicQ*2+nMechanicC*2+nMechanicRotationalQ*2+nMechanicRotationalC*2+1;
    QString nTotalOutputsString;
    nTotalOutputsString.setNum(nTotalOutputs);


    qDebug() << "Selected path: " << savePath;
    QDir saveDir;
    saveDir.setPath(savePath);    


    progressBar.setValue(2);
    progressBar.setLabelText("Generating files");


    QFile wrapperFile;
    wrapperFile.setFileName(savePath + "/HopsanSimulink.cpp");
    if(!wrapperFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open HopsanSimulink.cpp for writing.");
        return;
    }

    QFile portLabelsFile;
    portLabelsFile.setFileName(savePath + "/HopsanSimulinkPortLabels.m");
    if(!portLabelsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open HopsanSimulinkPortLabels.m for writing.");
        return;
    }


    QFile compileFile;
    compileFile.setFileName(savePath + "/HopsanSimulinkCompile.m");
    if(!compileFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open HopsanSimulinkCompile.m for writing.");
        return;
    }


    progressBar.setValue(3);
    progressBar.setLabelText("Writing HopsanSimulinkPortLabels.m");


    QTextStream portLabelsStream(&portLabelsFile);
    portLabelsStream << "set_param(gcb,'Mask','on')\n";
    portLabelsStream << "set_param(gcb,'MaskDisplay','";


    progressBar.setValue(4);
    progressBar.setLabelText("Writing HopsanSimulink.cpp");


    //How to access dialog parameters:
    //double par1 = (*mxGetPr(ssGetSFcnParam(S, 0)));

    QTextStream wrapperStream(&wrapperFile);
    QTextLineStream wrapperLines(wrapperStream);

    //! @todo writing the copyright notice should be a subfunction as it may be used in many places, preferably it should read from file, so that we do not forget to change in many places on changes
    wrapperLines << "/*-----------------------------------------------------------------------------";
    wrapperLines << "This source file is part of Hopsan NG";
    wrapperLines << "";
    wrapperLines << "Copyright (c) 2011";
    wrapperLines << "Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,";
    wrapperLines << "Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack";
    wrapperLines << "";
    wrapperLines << "This file is provided \"as is\", with no guarantee or warranty for the";
    wrapperLines << "functionality or reliability of the contents. All contents in this file is";
    wrapperLines << "the original work of the copyright holders at the Division of Fluid and";
    wrapperLines << "Mechatronic Systems (Flumes) at Linköping University. Modifying, using or";
    wrapperLines << "redistributing any part of this file is prohibited without explicit";
    wrapperLines << "permission from the copyright holders.";
    wrapperLines << "-----------------------------------------------------------------------------*/";
    wrapperLines << "";
    wrapperLines << "#define S_FUNCTION_NAME HopsanSimulink";
    wrapperLines << "#define S_FUNCTION_LEVEL 2";
    wrapperLines << "";
    wrapperLines << "#include <sstream>";
    wrapperLines << "#include <string>";
    wrapperLines << "#include <vector>";
    wrapperLines << "#include <fstream>";
    wrapperLines << "#include \"simstruc.h\"";
    wrapperLines << "#include \"include/HopsanCore.h\"";
    wrapperLines << "using namespace hopsan;";
    wrapperLines << "";

    //! @todo need to be able to error report if file not fond, or maybe not, if no external libs used you dont want error message
    wrapperLines << "void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames)";
    wrapperLines << "{";
    wrapperLines << "    rExtLibFileNames.clear();";
    wrapperLines << "    std::string line;";
    wrapperLines << "    std::ifstream file;";
    wrapperLines << "    file.open(filePath.c_str());";
    wrapperLines << "    if ( file.is_open() )";
    wrapperLines << "    {";
    wrapperLines << "        while ( file.good() )";
    wrapperLines << "        {";
    wrapperLines << "            getline(file, line);";
    wrapperLines << "            if ((*line.begin() != '#') && !line.empty())";
    wrapperLines << "            {";
    wrapperLines << "                rExtLibFileNames.push_back(line);";
    wrapperLines << "            }";
    wrapperLines << "       }";
    wrapperLines << "        file.close();";
    wrapperLines << "    }";
    wrapperLines << "    else";
    wrapperLines << "    {";
    wrapperLines << "        //cout << \"error, could not open file: \" << filePath << endl;";
    wrapperLines << "    }";
    wrapperLines << "}";
    wrapperLines << "";

    wrapperLines << "HopsanEssentials gHopsanCore";
    wrapperLines << "ComponentSystem* pComponentSystem;";
    wrapperLines << "bool isOkToSimulate = false;";
    wrapperLines << "";

    wrapperLines << "static void mdlInitializeSizes(SimStruct *S)";
    wrapperLines << "{";
    wrapperLines << "    ssSetNumSFcnParams(S, 0);";
    wrapperLines << "    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))";
    wrapperLines << "    {";
    wrapperLines << "        return;";
    wrapperLines << "    }";
    wrapperLines << "";
    wrapperLines << "    //Define S-function input signals";
    wrapperStream << "    if (!ssSetNumInputPorts(S," << nTotalInputsString << ")) return;				//Number of input signals\n";
    int i,j;
    size_t tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetInputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Input signal " << j << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicQComponents.at(i) << ".x''); ";
        wrapperStream << "    ssSetInputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Input signal " << j+1 << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j+1 << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicQComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetInputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Input signal " << j << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicCComponents.at(i) << ".cx''); ";
        wrapperStream << "    ssSetInputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Input signal " << j+1 << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j+1 << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetInputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Input signal " << j << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".a''); ";
        wrapperStream << "    ssSetInputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Input signal " << j+1 << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j+1 << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalQ*2;
    progressBar.setValue(5);
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetInputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Input signal " << j << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".cx''); ";
        wrapperStream << "    ssSetInputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Input signal " << j+1 << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j+1 << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nInputs; ++i)
    {
        j=tot+i;
        wrapperStream << "    ssSetInputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Input signal " << j << "\n";
        wrapperStream << "    ssSetInputPortDirectFeedThrough(S, " << j << ", 1);\n";
        portLabelsStream << "port_label(''input''," << j+1 << ",''" << inputComponents.at(i) << "''); ";
    }
    wrapperStream << "\n";
    wrapperStream << "    //Define S-function output signals\n";
    wrapperStream << "    if (!ssSetNumOutputPorts(S," + nTotalOutputsString + ")) return;				//Number of output signals\n";
    tot=0;
    for(i=0; i<nMechanicQ; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Output signal " << j << "\n";
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicQComponents.at(i) << ".cx''); ";
        wrapperStream << "    ssSetOutputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Output signal " << j+1 << "\n";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicQComponents.at(i) << ".Zx''); ";
    }
    tot+=nMechanicQ*2;
    for(i=0; i<nMechanicC; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Output signal " << j << "\n";
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicCComponents.at(i) << ".x''); ";
        wrapperStream << "    ssSetOutputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Output signal " << j+1 << "\n";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicCComponents.at(i) << ".v''); ";
    }
    tot+=nMechanicC*2;
    for(i=0; i<nMechanicRotationalQ; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Output signal " << j << "\n";
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalQComponents.at(i) << ".Zx''); ";
        wrapperStream << "    ssSetOutputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Output signal " << j+1 << "\n";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalQComponents.at(i) << ".cx''); ";
    }
    tot+=nMechanicRotationalQ*2;
    for(i=0; i<nMechanicRotationalC; ++i)
    {
        j=tot+i*2;
        wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Output signal " << j << "\n";
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << mechanicRotationalCComponents.at(i) << ".a''); ";
        wrapperStream << "    ssSetOutputPortWidth(S, " << j+1 << ", DYNAMICALLY_SIZED);		//Output signal " << j+1 << "\n";
        portLabelsStream << "port_label(''output''," << j+2 << ",''" << mechanicRotationalCComponents.at(i) << ".w''); ";
    }
    tot+=nMechanicRotationalC*2;
    for(i=0; i<nOutputs; ++i)
    {
        j=tot+i;
        wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Output signal " << j << "\n";
        portLabelsStream << "port_label(''output''," << j+1 << ",''" << outputComponents.at(i) << "''); ";
    }
    j=nTotalOutputs-1;
    wrapperStream << "    ssSetOutputPortWidth(S, " << j << ", DYNAMICALLY_SIZED);		//Debug output signal\n";
    portLabelsStream << "port_label(''output''," << j+1 << ",''DEBUG'')'); \n";
    wrapperStream << "    ssSetNumSampleTimes(S, 1);\n\n";
    wrapperStream << "    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);\n";
    wrapperStream << endl;

    wrapperLines << "    std::vector<std::string> extLibs;";
    wrapperLines << "    readExternalLibsFromTxtFile(\"externalLibs.txt\",extLibs);";
    wrapperLines << "    for (size_t i=0; i<extLibs.size(); ++i)";
    wrapperLines << "    {";
    wrapperLines << "        gHopsanCore->loadExternalComponentLib(extLibs[i]);";
    wrapperLines << "    }";
    wrapperLines << "";

    wrapperStream << "    std::string hmfFilePath = \"" << fileName << "\";\n";
    wrapperLines << "    double startT, stopT;";
    wrapperLines << "    gHopsanCore->loadHMFModel(hmfFilePath, startT, stopT);";
    wrapperLines << "    if (pComponentSystem==0)";
    wrapperLines << "    {";
    wrapperStream << "        ssSetErrorStatus(S,\"Error could not open model: " << fileName << "\");" << endl;
    wrapperLines << "        return;";
    wrapperLines << "    }";
    wrapperLines << "    startT = ssGetTStart(S);";
    wrapperLines << "    stopT = ssGetTFinal(S);";
    wrapperLines << "    pComponentSystem->setDesiredTimestep(0.001);";
    if(!pDisablePortLabels->isChecked())
    {
        wrapperLines << "    mexCallMATLAB(0, 0, 0, 0, \"HopsanSimulinkPortLabels\");                              //Run the port label script";
    }
    wrapperLines << "}";
    wrapperLines << "";

    wrapperLines << "static void mdlInitializeSampleTimes(SimStruct *S)";
    wrapperLines << "{";
    wrapperLines << "    ssSetSampleTime(S, 0, 0.001);";
    wrapperLines << "    ssSetOffsetTime(S, 0, 0.0);";
    wrapperLines << "";
    wrapperLines << "    //Update tunable parameters";
    wrapperLines << "    const mxArray* in;";
    wrapperLines << "    const char* c_str;";
    wrapperLines << "    std::string str;";

    /////////////////////////////////////////////////////////////////////

    for(int p=0; p<tunableParameters.size(); ++p)
    {
    wrapperStream << "    in = mexGetVariable(\"caller\",\"" << tunableParameters[p] << "\");\n";
    wrapperLines << "    if(in == NULL )";
    wrapperLines << "    {";
    wrapperStream << "        mexErrMsgTxt(\"Unable to read parameter \\\""+tunableParameters[p]+"\\\"!\");\n";
    wrapperLines << "    	return;";
    wrapperLines << "    }";
    wrapperLines << "";
    wrapperLines << "    c_str = (const char*)mxGetData(in);";
    wrapperLines << "";
    wrapperLines << "    str = \"\";";
    wrapperLines << "    for(int i=0; i<mxGetNumberOfElements(in); ++i)";
    wrapperLines << "    {";
    wrapperLines << "    	str.append(c_str);";
    wrapperLines << "    	c_str += 2*sizeof(char);";
    wrapperLines << "    }";
    wrapperLines << "";
    wrapperStream << "    pComponentSystem->setParameterValue(\""+tunableParameters[p]+"\", str);\n";
    }

    /////////////////////////////////////////////////////////////////////

    wrapperLines << "";
    wrapperLines << "";
    wrapperLines << "    isOkToSimulate = pComponentSystem->isSimulationOk();";
    wrapperLines << "    if (isOkToSimulate)";
    wrapperLines << "    {";
    wrapperLines << "        pComponentSystem->initialize(0,10);";
    wrapperLines << "    }";
    wrapperLines << "    else";
    wrapperLines << "    {";
    wrapperLines << "        ssSetErrorStatus(S,\"Error isSimulationOk() returned False! Most likely some components could not be loaded or some connections could not be established.\");";
    wrapperLines << "        return;";
    wrapperLines << "    }";

    wrapperLines << "}\n";

    wrapperLines << "static void mdlOutputs(SimStruct *S, int_T tid)";
    wrapperLines << "{";
    wrapperLines << "    //S-function input signals";
    wrapperLines << "    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);\n";
    wrapperLines << "    //S-function output signals";
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperStream << "    real_T *y" << i << " = ssGetOutputPortRealSignal(S," << i << ");\n";
    }
    wrapperStream << "    int_T width1 = ssGetOutputPortWidth(S,0);\n";
    wrapperStream << "\n";
    wrapperStream << "    //Input parameters\n";
    for(int i=0; i<nTotalInputs; ++i)
    {
        wrapperStream << "    double input" << i << " = (*uPtrs1[" << i << "]);\n";
    }
    wrapperStream << "\n";
    wrapperStream << "    //Equations\n";
    progressBar.setValue(6);
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperStream << "    double output" << i << ";\n";
    }
    wrapperStream << "    output" << nTotalOutputs-1 << " = 0;		//Error code 0: Nothing is wrong\n";
    wrapperStream << "    if(pComponentSystem == 0)\n";
    wrapperStream << "    {\n";
    wrapperStream << "      output" << nTotalOutputs-1 << " = -1;		//Error code -1: Component system failed to load\n";
    wrapperStream << "    }\n";
    //! @todo should remove this check from here
    wrapperStream << "    else if(!pComponentSystem->isSimulationOk())\n";
    wrapperStream << "    {\n";
    wrapperStream << "      output" << nTotalOutputs-1 << " = -2;		//Error code -2: Simulation not possible due to errors in model\n";
    wrapperStream << "    }\n";
    wrapperStream << "    else\n";
    wrapperStream << "    {\n";
    tot = 0;
    for(int i=0; i<nMechanicQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->writeNode(2, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->writeNode(0, input" << j+1 << ");\n";
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->writeNode(3, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->writeNode(4, input" << j+1 << ");\n";
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->writeNode(2, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->writeNode(0, input" << j+1 << ");\n";
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->writeNode(3, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->writeNode(4, input" << j+1 << ");\n";
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nInputs; ++i)
    {
        j = tot+i;
        wrapperStream << "        pComponentSystem->getSubComponent(\"" << inputComponents.at(i) << "\")->getPort(\"" << inputPorts.at(i) << "\")->writeNode(0, input" << i << ");\n";
    }
    wrapperStream << "        double timestep = pComponentSystem->getDesiredTimeStep();\n";
    wrapperStream << "        double time = ssGetT(S);\n";
    wrapperStream << "        pComponentSystem->simulate(time, time+timestep);\n";
    wrapperStream << "\n";
    tot = 0;
    for(int i=0; i<nMechanicQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getSubComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->readNode(3);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getSubComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->readNode(4);\n";
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getSubComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->readNode(2);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getSubComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->readNode(0);\n";
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getSubComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->readNode(3);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getSubComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->readNode(4);\n";
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getSubComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->readNode(2);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getSubComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->readNode(0);\n";
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nOutputs; ++i)
    {
        j = tot+i;
        wrapperStream << "        output" << j << " = pComponentSystem->getSubComponent(\"" << outputComponents.at(i) << "\")->getPort(\"" << outputPorts.at(i) << "\")->readNode(0);\n";
    }
    wrapperStream << "    }\n";
    wrapperStream << "\n";
    wrapperStream << "    //Output parameters\n";
    for(int i=0; i<nTotalOutputs; ++i)
    {
        wrapperStream << "    *y" << i << " = output" << i << ";\n";
    }
    wrapperStream << "}\n";
    wrapperStream << "\n";
    wrapperStream << "static void mdlTerminate(SimStruct *S){}\n";
    wrapperStream << "\n";
    wrapperStream << "\n";
    wrapperStream << "/* Simulink/Simulink Coder Interfaces */\n";
    wrapperStream << "#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */\n";
    wrapperStream << "#include \"simulink.c\" /* MEX-file interface mechanism */\n";
    wrapperStream << "#else\n";
    wrapperStream << "#include \"cg_sfun.h\" /* Code generation registration function */\n";
    wrapperStream << "#endif\n";
    wrapperStream << "\n";
    wrapperFile.close();

    portLabelsStream << "set_param(gcb,'BackgroundColor','[0.721569, 0.858824, 0.905882]')\n";
    portLabelsStream << "set_param(gcb,'Name','" << this->getName() << "')\n";
    portLabelsStream << "set_param(gcb,'MaskPrompts',{";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << "'"+tunableParameters[p]+"'";
        if(p<tunableParameters.size()-1)
            portLabelsStream << ",";
    }
    portLabelsStream << "})\n";
    portLabelsStream << "set_param(gcb,'MaskVariables','";
    for(int p=0; p<tunableParameters.size(); ++p)
    {
        portLabelsStream << tunableParameters[p]+"=&"+QString().setNum(p+1)+";";
    }
    portLabelsStream << "')\n";

    portLabelsFile.close();

    progressBar.setValue(7);
    progressBar.setLabelText("Writing HopsanSimulinkCompile.m");


    QTextStream compileStream(&compileFile);
#ifdef WIN32
    //compileStream << "%mex -DWIN32 -DSTATICCORE HopsanSimulink.cpp /include/Component.cc /include/ComponentSystem.cc /include/HopsanEssentials.cc /include/Node.cc /include/Port.cc /include/Components/Components.cc /include/CoreUtilities/HmfLoader.cc /include/CoreUtilities/HopsanCoreMessageHandler.cc /include/CoreUtilities/LoadExternal.cc /include/Nodes/Nodes.cc /include/ComponentUtilities/AuxiliarySimulationFunctions.cpp /include/ComponentUtilities/Delay.cc /include/ComponentUtilities/DoubleIntegratorWithDamping.cpp /include/ComponentUtilities/FirstOrderFilter.cc /include/ComponentUtilities/Integrator.cc /include/ComponentUtilities/IntegratorLimited.cc /include/ComponentUtilities/ludcmp.cc /include/ComponentUtilities/matrix.cc /include/ComponentUtilities/SecondOrderFilter.cc /include/ComponentUtilities/SecondOrderTransferFunction.cc /include/ComponentUtilities/TurbulentFlowFunction.cc /include/ComponentUtilities/ValveHysteresis.cc\n";
    compileStream << "mex -DWIN32 -DSTATICCORE -L./ -Iinclude -lHopsanCore HopsanSimulink.cpp\n";

    progressBar.setValue(8);
    progressBar.setLabelText("Copying Visual Studio binaries");


    //Select path to MSVC library depending on user selection
    QString msvcPath;
    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        msvcPath = QString(MSVC2008_X86_PATH);
    }
    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        msvcPath = QString(MSVC2008_X64_PATH);
    }
    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
    {
        msvcPath = QString(MSVC2010_X86_PATH);
    }
    else if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())
    {
        msvcPath = QString(MSVC2010_X64_PATH);
    }


    //Copy MSVC binaries to export folder
    QFile dllFile(msvcPath + "HopsanCore.dll");
    dllFile.copy(savePath + "/HopsanCore.dll");
    QFile libFile(msvcPath + "HopsanCore.lib");
    libFile.copy(savePath + "/HopsanCore.lib");
    QFile expFile(msvcPath + "HopsanCore.exp");
    expFile.copy(savePath + "/HopsanCore.exp");

#else
    compileStream << "% You need to copy the .so files here or change the -L lib search path" << endl;
    compileStream << "mex -L./ -Iinclude -lHopsanCore HopsanSimulink.cpp" << endl;

    //! @todo copy all of the symolic links and the .so

#endif
    compileFile.close();

    progressBar.setValue(9);
    progressBar.setLabelText("Copying include files");

    copyIncludeFilesToDir(savePath);

    //! @todo should not overwrite this wile if it already exists
    QFile externalLibsFile;
    externalLibsFile.setFileName(savePath + "/externalLibs.txt");
    if(!externalLibsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open externalLibs.txt for writing.");
        return;
    }
    QTextStream externalLibsFileStream(&externalLibsFile);
    externalLibsFileStream << "#Enter the relative path to each external component lib that needs to be loaded" << endl;
    externalLibsFileStream << "#Enter one per line, the filename is enough if you put the lib file (.dll or.so) in this directory.";
    externalLibsFile.close();


    progressBar.setValue(10);
    progressBar.setLabelText("Writing " + mModelFileInfo.fileName() + " .hmf");


    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
        //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getCoreSystemAccessPtr()->getHopsanCoreVersion());
    saveToDomElement(hmfRoot);
    const int IndentSize = 4;
    QFile xmlhmf(savePath + "/" + fileName);
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, IndentSize);
    xmlhmf.close();

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
