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
#include "Widgets/PyDockWidget.h"
#include "Configuration.h"

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
}

SystemContainer::~SystemContainer()
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
    mStartTime = 0;     //! @todo These default values should be options for the user
    mTimeStep = 0.001;
    mStopTime = 10;
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
        qDebug() << "creating subsystem and setting name in " << mpParentContainerObject->getCoreSystemAccessPtr()->getRootSystemName();
        mModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->createSubSystem(this->getName()));
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentContainerObject->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentContainerObject->getCoreSystemAccessPtr());
    }

    mpCoreSystemAccess->setDesiredTimeStep(mTimeStep);
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
        setDisplayName(mpCoreSystemAccess->setRootSystemName(newName));
    }
    else
    {
        mpParentContainerObject->renameModelObject(this->getName(), newName);
    }
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
    return mpCoreSystemAccess->getRootSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QVector containing the parameter names
QStringList SystemContainer::getParameterNames()
{
    return mpCoreSystemAccess->getParameterNames(this->getName());
}

//! @brief Get a pointer the the CoreSystemAccess object that this system is representing
CoreSystemAccess* SystemContainer::getCoreSystemAccessPtr()
{
    return this->mpCoreSystemAccess;
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
    appendSimulationTimeTag(rDomElement, this->mStartTime, this->mTimeStep, this->mStopTime);

    QDomElement parElement = appendDomElement(rDomElement, HMF_PARAMETERS);
    QList<QStringList> favPars = this->getFavoriteVariables();
    QList<QStringList>::iterator itf;
    for(itf = favPars.begin(); itf != favPars.end(); ++itf)
    {
        QDomElement favoriteElement = appendDomElement(parElement, HMF_FAVORITEVARIABLETAG);
        favoriteElement.setAttribute("componentname", (*itf).at(0));
        favoriteElement.setAttribute("portname", (*itf).at(1));
        favoriteElement.setAttribute("dataname", (*itf).at(2));
        favoriteElement.setAttribute("dataunit", (*itf).at(3));
    }

    QVector<QString> parameterNames, parameterValues, descriptions, units, types;
    mpCoreSystemAccess->getSystemParameters(parameterNames, parameterValues, descriptions, units, types);
    for(int i=0; i<parameterNames.size(); ++i)
    {
        QDomElement mappedElement = appendDomElement(parElement, HMF_PARAMETERTAG);
        mappedElement.setAttribute(HMF_NAMETAG, parameterNames[i]);
        mappedElement.setAttribute(HMF_VALUETAG, parameterValues[i]);
        mappedElement.setAttribute(HMF_TYPETAG, types[i]);
    }


//    QMap<std::string, std::string>::iterator it;
//    QMap<std::string, std::string> parMap = mpCoreSystemAccess->getSystemParametersMap();
//    for(it = parMap.begin(); it != parMap.end(); ++it)
//    {
//        QDomElement mappedElement = appendDomElement(parElement, HMF_PARAMETERTAG);
//        mappedElement.setAttribute("name", QString(it.key().c_str()));
//        mappedElement.setAttribute("value", QString(it.value().c_str()));
//    }

    QMap<QString, QStringList>::iterator ita;
    for(ita=mPlotAliasMap.begin(); ita!=mPlotAliasMap.end(); ++ita)
    {
        QDomElement aliasElement = appendDomElement(parElement, "alias");
        aliasElement.setAttribute("alias", ita.key());
        aliasElement.setAttribute("component", ita.value().at(0));
        aliasElement.setAttribute("port",ita.value().at(1));
        aliasElement.setAttribute("data",ita.value().at(2));
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
            QDomElement XMLObjectiveVariable = appendDomElement(XMLObjectiveVariables, "variable");
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mVariableInfo.size(); ++j)
            {
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
        Objectives objectives;

        QDomElement XMLobj = rDomElement.firstChildElement("objectives").firstChildElement("objective");
        while (!XMLobj.isNull())
        {
            objectives.mFunctionName = XMLobj.firstChildElement("functionname").text();
            objectives.mWeight = XMLobj.firstChildElement("weight").text().toDouble();
            objectives.mNorm = XMLobj.firstChildElement("norm").text().toDouble();
            objectives.mExp = XMLobj.firstChildElement("exp").text().toDouble();

            if(!XMLobj.firstChildElement("variables").isNull())
            {
                QDomElement XMLVars = XMLobj.firstChildElement("variables").firstChildElement("variable");
                QStringList variableInfo;
                while (!XMLVars.isNull())
                {
                    variableInfo.append(XMLVars.firstChildElement("componentname").text());
                    variableInfo.append(XMLVars.firstChildElement("portname").text());
                    variableInfo.append(XMLVars.firstChildElement("variablename").text());

                    XMLVars = XMLVars.nextSiblingElement("variable");
                }
                objectives.mVariableInfo.append(variableInfo);
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
        }
        mOptSettings.mObjectives.append(objectives);
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
        QDomElement xmlApp = appendDomElement(guiStuff, CAF_ROOT);
        xmlApp.setAttribute(CAF_VERSION, CAF_VERSIONNUM);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mModelObjectAppearance.saveToDomElement(xmlApp);
    }

    saveOptSettingsToDomElement(guiStuff);

    guiStuff.appendChild(mpUndoStack->toXml());

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveToDomElement(QDomElement &rDomElement)
{
    //qDebug() << "Saving to dom node in: " << this->mGUIModelObjectAppearance.getName();
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

    //! @todo do we really need both systemtype and external path, en empty path could indicate embeded
    if ((mpParentContainerObject != 0) && (mLoadType=="EXTERNAL"))
    {
        //This information should ONLY be used to indicate that a system is external, it SHOULD NOT be included in the actual external system
        //If it would be, the load function will fail
        xmlSubsystem.setAttribute( HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentContainerObject->getModelFileInfo().absolutePath()) );

        //Save the name and type that we have set for this subsystem, this name will overwrite the defualt one in the external file
        ModelObject::saveCoreDataToDomElement(xmlSubsystem); //!< @todo Not sure why we should not use savecoredata in GUISystem instead, but it seems to be embeded specific
    }

    //Save gui object stuff
    this->saveGuiDataToDomElement(xmlSubsystem);

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save Core related stuff
        this->saveCoreDataToDomElement(xmlSubsystem); //Only save core stuff in root and embeded systems

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
        for(int i = 0; i != mSubConnectorList.size(); ++i)
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
        QString realName = this->getName();

        //Load the GUI stuff like appearance data and viewport
        QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
        this->mModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT));
        this->setDisplayName(realName); // This must be done becouse in some occations the loadAppearanceDataline above will overwrite the correct name
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
        }

        mpParentProjectTab->getGraphicsView()->setZoomFactor(zoom);

        qDebug() << "Center on " << x << ", " << y;
        mpParentProjectTab->getGraphicsView()->centerOn(x, y);
        //! @todo load viewport and pose and stuff

        //Load simulation time
        parseSimulationTimeTag(rDomElement.firstChildElement(HMF_SIMULATIONTIMETAG), mStartTime, mTimeStep, mStopTime);
        gpMainWindow->setStartTimeInToolBar(mStartTime);
        gpMainWindow->setTimeStepInToolBar(mTimeStep);
        gpMainWindow->setFinishTimeInToolBar(mStopTime);

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
                                                                    xmlSubObject.attribute(HMF_TYPETAG) + QString(", Name: ") + xmlSubObject.attribute(HMF_NAMETAG));

                // Insert missing component dummy instead
                xmlSubObject.setAttribute(HMF_TYPETAG, "MissingComponent");
                pObj = loadModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            }
            else
            {
                //Load parameter values
                QDomElement xmlParameters = xmlSubObject.firstChildElement(HMF_PARAMETERS);
                QDomElement xmlParameter = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
                while (!xmlParameter.isNull())
                {
                    loadParameterValue(xmlParameter, pObj, NOUNDO);
                    xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
                }

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
        xmlSubObject = xmlParameters.firstChildElement(HMF_FAVORITEVARIABLETAG);
        while (!xmlSubObject.isNull())
        {
            loadFavoriteVariable(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_FAVORITEVARIABLETAG);
        }

        //9. Load plot variable aliases
        xmlSubObject = xmlParameters.firstChildElement("alias");
        while (!xmlSubObject.isNull())
        {
            loadPlotAlias(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement("alias");
        }

        //10. Load optimization settings
        xmlSubObject = guiStuff.firstChildElement("optimization");
        loadOptSettingsFromDomElement(xmlSubObject);


        //Refresh the appearnce of the subsystemem and create the GUIPorts based on the loaded portappearance information
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

        emit checkMessages();
    }
    else
    {
        //Load external system
        //! @todo use some code that opens the atual file in this case
        //! @todo this code does not seem to ever run right now, this will probalby never be called as loadobjects handle this
//        QFile file(external_path);
//        QDomElement externalRoot = loadXMLDomDocument(file, HMF_ROOTTAG);
//        QDomElement systemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
//        loadSubsystemGUIObject(systemRoot, gpMainWindow->mpLibrary, this, NOUNDO);

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


void SystemContainer::createFMUSourceFiles()
{
    QDialog *pExportFmuDialog = new QDialog(gpMainWindow);
    pExportFmuDialog->setWindowTitle("Export to Functional Mockup Interface");

    QLabel *pExportFmuLabel = new QLabel(gpMainWindow->tr("This will create a Functional Mockup Unit of\ncurrent model. Please choose compiler:"), pExportFmuDialog);

    mpExportFmuGccRadioButton = new QRadioButton(gpMainWindow->tr("GCC"), pExportFmuDialog);
    mpExportFmuGccRadioButton->setChecked(true);
    mpExportFmuMsvcRadioButton = new QRadioButton(gpMainWindow->tr("Microsoft Visual C"), pExportFmuDialog);

    QPushButton *pOkButton = new QPushButton("Okay", pExportFmuDialog);
    QPushButton *pCancelButton = new QPushButton("Cancel", pExportFmuDialog);

    QGridLayout *pExportFmuLayout = new QGridLayout(pExportFmuDialog);
    pExportFmuLayout->addWidget(pExportFmuLabel,            0, 0, 1, 2);
    pExportFmuLayout->addWidget(mpExportFmuGccRadioButton,  1, 0, 1, 2);
    pExportFmuLayout->addWidget(mpExportFmuMsvcRadioButton, 2, 0, 1, 2);
    pExportFmuLayout->addWidget(pOkButton,                  3, 0, 1, 1);
    pExportFmuLayout->addWidget(pCancelButton,              3, 1, 1, 1);

    pExportFmuDialog->setLayout(pExportFmuLayout);

    pExportFmuDialog->show();

    connect(pOkButton,      SIGNAL(clicked()), pExportFmuDialog,    SLOT(close()));
    connect(pOkButton,      SIGNAL(clicked()), this,                SLOT(createFMUSourceFilesFromDialog()));
    connect(pCancelButton,  SIGNAL(clicked()), pExportFmuDialog,    SLOT(close()));
}


void SystemContainer::createFMUSourceFilesFromDialog()
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
    bool gccCompiler = mpExportFmuGccRadioButton->isChecked();


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
    modelDescriptionFile.setFileName(savePath + "/ModelDescription.xml");
    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open ModelDescription.xml for writing.");
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

    QFile clBatchFile;
    clBatchFile.setFileName(savePath + "/compile.bat");
    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Failed to open compile.bat for writing.");
        return;
    }

    progressBar.setLabelText("Writing ModelDescription.xml");
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
    fmuHeaderStream << "#ifndef HOPSANFMU_H\n";
    fmuHeaderStream << "#define HOPSANFMU_H\n\n";
    fmuHeaderStream << "#ifdef WRAPPERCOMPILATION\n";
    fmuHeaderStream << "    #define DLLEXPORT __declspec(dllexport)\n";
    fmuHeaderStream << "    extern \"C\" {\n";
    fmuHeaderStream << "#else\n";
    fmuHeaderStream << "    #define DLLEXPORT\n";
    fmuHeaderStream << "#endif\n\n";
    fmuHeaderStream << "DLLEXPORT void initializeHopsanWrapper(char* filename);\n";
    fmuHeaderStream << "DLLEXPORT void simulateOneStep();\n";
    fmuHeaderStream << "DLLEXPORT double getVariable(char* component, char* port, size_t idx);\n\n";
    fmuHeaderStream << "DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);\n\n";
    fmuHeaderStream << "#ifdef WRAPPERCOMPILATION\n";
    fmuHeaderStream << "}\n";
    fmuHeaderStream << "#endif\n\n";
    fmuHeaderStream << "#endif // HOPSANFMU_H\n";
    fmuHeaderFile.close();


    progressBar.setLabelText("Writing HopsanFMU.cpp");
    progressBar.setValue(5);


    QTextStream fmuSourceStream(&fmuSourceFile);
    fmuSourceStream << "#include <iostream>\n";
    fmuSourceStream << "#include <assert.h>\n";
    fmuSourceStream << "#include \"HopsanFMU.h\"\n";
    fmuSourceStream << "#include \"include/HopsanCore.h\"\n";
    fmuSourceStream << "#include \"include/HopsanEssentials.h\"\n";
    fmuSourceStream << "#include \"include/ComponentEssentials.h\"\n";
    fmuSourceStream << "#include \"include/ComponentUtilities.h\"\n";
    fmuSourceStream << "#include \"include/CoreUtilities/HmfLoader.h\"\n\n";
    fmuSourceStream << "static double time=0;\n";
    fmuSourceStream << "static hopsan::ComponentSystem *spCoreComponentSystem;\n";
    fmuSourceStream << "static std::vector<string> sComponentNames;\n\n";
    fmuSourceStream << "void initializeHopsanWrapper(char* filename)\n";
    fmuSourceStream << "{\n";
    fmuSourceStream << "    hopsan::HmfLoader coreHmfLoader;\n";
    fmuSourceStream << "    double startT;      //Dummy variable\n";
    fmuSourceStream << "    double stopT;       //Dummy variable\n";
    fmuSourceStream << "    spCoreComponentSystem = coreHmfLoader.loadModel(filename, startT, stopT);\n";
    fmuSourceStream << "    spCoreComponentSystem->setDesiredTimestep(0.001);\n";           //!< @todo Time step should not be hard coded
    fmuSourceStream << "    spCoreComponentSystem->initialize(0,10,0);\n";
    fmuSourceStream << "}\n\n";
    fmuSourceStream << "void simulateOneStep()\n";
    fmuSourceStream << "{\n";
    fmuSourceStream << "    if(spCoreComponentSystem->isSimulationOk())\n";
    fmuSourceStream << "    {\n";
    fmuSourceStream << "        double timestep = spCoreComponentSystem->getDesiredTimeStep();\n";
    fmuSourceStream << "        spCoreComponentSystem->simulate(time, time+timestep);\n";
    fmuSourceStream << "        time = time+timestep;\n";
    fmuSourceStream << "    }\n";
    fmuSourceStream << "    else\n";
    fmuSourceStream << "    {\n";
    fmuSourceStream << "        cout << \"Simulation failed!\";\n";
    fmuSourceStream << "    }\n";
    fmuSourceStream << "}\n\n";
    fmuSourceStream << "double getVariable(char* component, char* port, size_t idx)\n";
    fmuSourceStream << "{\n";
    fmuSourceStream << "    return spCoreComponentSystem->getComponent(component)->getPort(port)->readNode(idx);\n";
    fmuSourceStream << "}\n\n";
    fmuSourceStream << "void setVariable(char* component, char* port, size_t idx, double value)\n";
    fmuSourceStream << "{\n";
    fmuSourceStream << "    assert(spCoreComponentSystem->getComponent(component)->getPort(port) != 0);\n";
    fmuSourceStream << "    return spCoreComponentSystem->getComponent(component)->getPort(port)->writeNode(idx, value);\n";
    fmuSourceStream << "}\n";
    fmuSourceFile.close();


    progressBar.setLabelText("Writing compile.bat");
    progressBar.setValue(6);


    //Write the compilation script file
    QTextStream clBatchStream(&clBatchFile);
    if(gccCompiler)
    {
        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
        clBatchStream << "g++ -DWRAPPERCOMPILATION -c HopsanFMU.cpp\n";
        clBatchStream << "g++ -shared -o HopsanFMU.dll HopsanFMU.o -L./ -lHopsanCore";
    }
    else
    {
        //! @todo Check that Visual Studio is installed, and warn user if not
        clBatchStream << "echo Compiling Visual Studio libraries...\n";
        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
    }
    clBatchFile.close();


    progressBar.setLabelText("Copying binary files");
    progressBar.setValue(7);


    //Copy binaries to export directory
    QFile dllFile;
    QFile libFile;
    QFile expFile;
    if(gccCompiler)
    {
        dllFile.setFileName(gExecPath + "HopsanCore_d.dll");
        dllFile.copy(savePath + "/HopsanCore.dll");
    }
    else
    {
        dllFile.setFileName(gExecPath + "/../binVC/HopsanCore.dll");
        dllFile.copy(savePath + "/HopsanCore.dll");
        libFile.setFileName(gExecPath + "/../binVC/HopsanCore.lib");
        libFile.copy(savePath + "/HopsanCore.lib");
        expFile.setFileName(gExecPath + "/../binVC/HopsanCore.exp");
        expFile.copy(savePath + "/HopsanCore.exp");
    }


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


    progressBar.setLabelText("Compiling HopsanFMU.dll");
    progressBar.setValue(11);



    //Execute HopsanFMU compile script
    QProcess p;
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & compile.bat");
    p.waitForFinished();


    progressBar.setLabelText("Copying compilation files");
    progressBar.setValue(14);


    //Copy FMI compilation files to export directory
    QFile buildFmuFile;
    if(gccCompiler)
    {
        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_gcc.bat");
    }
    else
    {
        buildFmuFile.setFileName(gExecPath + "/../ThirdParty/fmi/build_fmu_vc.bat");
    }
    buildFmuFile.copy(savePath + "/build_fmu.bat");
    QFile fmuModelFunctionsHFile(gExecPath + "/../ThirdParty/fmi/fmiModelFunctions.h");
    fmuModelFunctionsHFile.copy(savePath + "/fmiModelFunctions.h");
    QFile fmiModelTypesHFile(gExecPath + "/../ThirdParty/fmi/fmiModelTypes.h");
    fmiModelTypesHFile.copy(savePath + "/fmiModelTypes.h");
    QFile fmiTemplateCFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.c");
    fmiTemplateCFile.copy(savePath + "/fmuTemplate.c");
    QFile fmiTemplateHFile(gExecPath + "/../ThirdParty/fmi/fmuTemplate.h");
    fmiTemplateHFile.copy(savePath + "/fmuTemplate.h");


    progressBar.setLabelText("Compiling "+modelName+".dll");
    progressBar.setValue(15);


    //Execute FMU compile script
    p.start("cmd.exe", QStringList() << "/c" << "cd " + savePath + " & build_fmu.bat me " + modelName);
    p.waitForFinished();


    progressBar.setLabelText("Sorting files");
    progressBar.setValue(18);


    saveDir.mkpath("fmu/binaries/win32");
    saveDir.mkpath("fmu/resources");
    QFile modelDllFile(savePath + "/" + modelName + ".dll");
    modelDllFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".dll");
    QFile modelLibFile(savePath + "/" + modelName + ".lib");
    modelLibFile.copy(savePath + "/fmu/binaries/win32/" + modelName + ".lib");
    dllFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.dll");
    if(!gccCompiler)
    {
        libFile.copy(savePath + "/fmu/binaries/win32/HopsanCore.lib");
    }
    QFile hopsanFMUdllFile(savePath + "/HopsanFMU.dll");
    hopsanFMUdllFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.dll");
    QFile hopsanFMUlibFile(savePath + "/HopsanFMU.lib");
    hopsanFMUlibFile.copy(savePath + "/fmu/binaries/win32/HopsanFMU.lib");
    QFile modelFile(savePath + "/" + realModelName + ".hmf");
    modelFile.copy(savePath + "/fmu/resources/" + realModelName + ".hmf");
    modelDescriptionFile.copy(savePath + "/fmu/ModelDescription.xml");

    QString fmuFileName = savePath + "/" + modelName + ".fmu";


    progressBar.setLabelText("Compressing files");
    progressBar.setValue(19);



    p.start("cmd.exe", QStringList() << "/c" << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/ModelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources");
    p.waitForFinished();
    qDebug() << "Called: " << gExecPath + "../ThirdParty/7z/7z.exe a -tzip " + fmuFileName + " " + savePath + "/fmu/ModelDescription.xml " + savePath + "/fmu/binaries/ " + savePath + "/fmu/resources";


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
    QMessageBox::information(gpMainWindow, gpMainWindow->tr("Create Simulink Source Files"),
                             gpMainWindow->tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console.\n\nVisual Studio 2008 compiler is supported, although other versions might work as well.."));

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
    QDir fileDialogSaveDir;
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
    wrapperStream << "/*-----------------------------------------------------------------------------\n";
    wrapperStream << "This source file is part of Hopsan NG\n\n";
    wrapperStream << "Copyright (c) 2011\n";
    wrapperStream << "Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,\n";
    wrapperStream << "Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack\n";
    wrapperStream << "\n";
    wrapperStream << "This file is provided \"as is\", with no guarantee or warranty for the\n";
    wrapperStream << "functionality or reliability of the contents. All contents in this file is\n";
    wrapperStream << "the original work of the copyright holders at the Division of Fluid and\n";
    wrapperStream << "Mechatronic Systems (Flumes) at Linköping University. Modifying, using or\n";
    wrapperStream << "redistributing any part of this file is prohibited without explicit\n";
    wrapperStream << "permission from the copyright holders.\n";
    wrapperStream << "-----------------------------------------------------------------------------*/\n\n";
    wrapperStream << "#define S_FUNCTION_NAME HopsanSimulink\n";
    wrapperStream << "#define S_FUNCTION_LEVEL 2\n\n";
    wrapperStream << "#include \"simstruc.h\"\n";
    wrapperStream << "#include <sstream>\n";
    wrapperStream << "#include \"include/HopsanCore.h\"\n";
    wrapperStream << "#include \"include/Component.h\"\n";
    wrapperStream << "#include \"include/ComponentSystem.h\"\n";
    wrapperStream << "#include \"include/ComponentEssentials.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities.h\"\n";
    wrapperStream << "#include \"include/HopsanEssentials.h\"\n";
    wrapperStream << "#include \"include/Node.h\"\n";
    wrapperStream << "#include \"include/Port.h\"\n";
    wrapperStream << "#include \"include/Nodes.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/AuxiliarySimulationFunctions.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/CSVParser.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/Delay.hpp\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/DoubleIntegratorWithDamping.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/FirstOrderTransferFunction.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/Integrator.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/IntegratorLimited.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/ludcmp.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/matrix.h\"\n";
    //wrapperStream << "#include \"include/ComponentUtilities/ReadDataCurve.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/SecondOrderTransferFunction.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/TurbulentFlowFunction.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/ValveHysteresis.h\"\n";
    wrapperStream << "#include \"include/ComponentUtilities/WhiteGaussianNoise.h\"\n";
    wrapperStream << "#include \"include/CoreUtilities/HmfLoader.h\"\n";
    wrapperStream << "#include \"include/CoreUtilities/ClassFactory.hpp\"\n";
    wrapperStream << "#include \"include/CoreUtilities/ClassFactoryStatusCheck.hpp\"\n";
    wrapperStream << "#include \"include/CoreUtilities/FindUniqueName.h\"\n";
    wrapperStream << "#include \"include/CoreUtilities/HopsanCoreMessageHandler.h\"\n";
    wrapperStream << "#include \"include/CoreUtilities/LoadExternal.h\"\n";
    //wrapperStream << "#include \"include/Components/Components.h\"\n\n";
    wrapperStream << "using namespace hopsan;\n\n";
    wrapperStream << "ComponentSystem* pComponentSystem;\n\n";
    wrapperStream << "static void mdlInitializeSizes(SimStruct *S)\n";
    wrapperStream << "{\n";
    wrapperStream << "    ssSetNumSFcnParams(S, 0);\n";
    wrapperStream << "    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))\n";
    wrapperStream << "    {\n";
    wrapperStream << "        return;\n";
    wrapperStream << "    }\n\n";
    wrapperStream << "    //Define S-function input signals\n";
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
    wrapperStream << "    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);\n\n";
    wrapperStream << "    std::string hmfFilePath = \"" << fileName << "\";\n";
    wrapperStream << "    hopsan::HmfLoader coreHmfLoader;\n";
    wrapperStream << "    double startT = ssGetTStart(S);\n";
    wrapperStream << "    double stopT = ssGetTFinal(S);\n";
    wrapperStream << "    pComponentSystem = coreHmfLoader.loadModel(hmfFilePath, startT, stopT);\n";
    wrapperStream << "    pComponentSystem->setDesiredTimestep(0.001);\n";
    wrapperStream << "    pComponentSystem->initialize(0,10,0);\n\n";
    wrapperStream << "    mexCallMATLAB(0, 0, 0, 0, \"HopsanSimulinkPortLabels\");                               //Run the port label script\n";
    wrapperStream << "}\n";
    wrapperStream << "\n";
    wrapperStream << "static void mdlInitializeSampleTimes(SimStruct *S)\n";
    wrapperStream << "{\n";
    wrapperStream << "    ssSetSampleTime(S, 0, 0.001);\n";
    wrapperStream << "    ssSetOffsetTime(S, 0, 0.0);\n";
    wrapperStream << "}\n\n";
    wrapperStream << "static void mdlOutputs(SimStruct *S, int_T tid)\n";
    wrapperStream << "{\n";
    wrapperStream << "    //S-function input signals\n";
    wrapperStream << "    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);\n\n";
    wrapperStream << "    //S-function output signals\n";
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
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->writeNode(2, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->writeNode(0, input" << j+1 << ");\n";
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->writeNode(3, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->writeNode(4, input" << j+1 << ");\n";
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->writeNode(2, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->writeNode(0, input" << j+1 << ");\n";
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->writeNode(3, input" << j << ");\n";
        wrapperStream << "        pComponentSystem->getComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->writeNode(4, input" << j+1 << ");\n";
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nInputs; ++i)
    {
        j = tot+i;
        wrapperStream << "        pComponentSystem->getComponent(\"" << inputComponents.at(i) << "\")->getPort(\"" << inputPorts.at(i) << "\")->writeNode(0, input" << i << ");\n";
    }
    wrapperStream << "        double timestep = pComponentSystem->getDesiredTimeStep();\n";
    wrapperStream << "        double time = ssGetT(S);\n";
    wrapperStream << "        pComponentSystem->simulate(time, time+timestep);\n";
    wrapperStream << "\n";
    tot = 0;
    for(int i=0; i<nMechanicQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->readNode(3);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getComponent(\"" << mechanicQComponents.at(i) << "\")->getPort(\"" << mechanicQPorts.at(i) << "\")->readNode(4);\n";
    }
    tot+=nMechanicQ*2;
    for(int i=0; i<nMechanicC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->readNode(2);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getComponent(\"" << mechanicCComponents.at(i) << "\")->getPort(\"" << mechanicCPorts.at(i) << "\")->readNode(0);\n";
    }
    tot+=nMechanicC*2;
    for(int i=0; i<nMechanicRotationalQ; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->readNode(3);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getComponent(\"" << mechanicRotationalQComponents.at(i) << "\")->getPort(\"" << mechanicRotationalQPorts.at(i) << "\")->readNode(4);\n";
    }
    tot+=nMechanicRotationalQ*2;
    for(int i=0; i<nMechanicRotationalC; ++i)
    {
        j = tot+i*2;
        wrapperStream << "        output" << j << " = pComponentSystem->getComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->readNode(2);\n";
        wrapperStream << "        output" << j+1 << " = pComponentSystem->getComponent(\"" << mechanicRotationalCComponents.at(i) << "\")->getPort(\"" << mechanicRotationalCPorts.at(i) << "\")->readNode(0);\n";
    }
    tot+=nMechanicRotationalC*2;
    for(int i=0; i<nOutputs; ++i)
    {
        j = tot+i;
        wrapperStream << "        output" << j << " = pComponentSystem->getComponent(\"" << outputComponents.at(i) << "\")->getPort(\"" << outputPorts.at(i) << "\")->readNode(0);\n";
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
    portLabelsStream << "set_param(gcb,'Name','" << this->getName() << "')";
    portLabelsFile.close();


    progressBar.setValue(7);
    progressBar.setLabelText("Writing HopsanSimulinkCompile.m");


    QTextStream compileStream(&compileFile);
    //compileStream << "%mex -DWIN32 -DSTATICCORE HopsanSimulink.cpp /include/Component.cc /include/ComponentSystem.cc /include/HopsanEssentials.cc /include/Node.cc /include/Port.cc /include/Components/Components.cc /include/CoreUtilities/HmfLoader.cc /include/CoreUtilities/HopsanCoreMessageHandler.cc /include/CoreUtilities/LoadExternal.cc /include/Nodes/Nodes.cc /include/ComponentUtilities/AuxiliarySimulationFunctions.cpp /include/ComponentUtilities/Delay.cc /include/ComponentUtilities/DoubleIntegratorWithDamping.cpp /include/ComponentUtilities/FirstOrderFilter.cc /include/ComponentUtilities/Integrator.cc /include/ComponentUtilities/IntegratorLimited.cc /include/ComponentUtilities/ludcmp.cc /include/ComponentUtilities/matrix.cc /include/ComponentUtilities/SecondOrderFilter.cc /include/ComponentUtilities/SecondOrderTransferFunction.cc /include/ComponentUtilities/TurbulentFlowFunction.cc /include/ComponentUtilities/ValveHysteresis.cc\n";
    compileStream << "mex -DWIN32 -DSTATICCORE -L./ -Iinclude -lHopsanCore HopsanSimulink.cpp\n";
    compileFile.close();


    progressBar.setValue(8);
    progressBar.setLabelText("Copying Visual Studio binaries");


    QFile dllFile(gExecPath + "/../binVC/HopsanCore.dll");
    dllFile.copy(savePath + "/HopsanCore.dll");
    QFile libFile(gExecPath + "/../binVC/HopsanCore.lib");
    libFile.copy(savePath + "/HopsanCore.lib");
    QFile expFile(gExecPath + "/../binVC/HopsanCore.exp");
    expFile.copy(savePath + "/HopsanCore.exp");


    progressBar.setValue(9);
    progressBar.setLabelText("Copying include files");


    copyIncludeFilesToDir(savePath);


    progressBar.setValue(10);
    progressBar.setLabelText("Writing " + mModelFileInfo.fileName() + " .hmf");


    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
        //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, "0");
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
}


//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
void SystemContainer::setModelFileInfo(QFile &rFile)
{
    this->mModelFileInfo.setFile(rFile);
}

//! Function that updates start time value of the current project to the one in the simulation setup widget.
//! @see updateTimeStep()
//! @see updateStopTime()
void SystemContainer::updateStartTime()
{
    mStartTime = gpMainWindow->getStartTimeFromToolBar();
}


//! Function that updates time step value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateStopTime()
void SystemContainer::updateTimeStep()
{
    mTimeStep = gpMainWindow->getTimeStepFromToolBar();
}


//! Function that updates stop time value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateTimeStep()
void SystemContainer::updateStopTime()
{
    mStopTime = gpMainWindow->getFinishTimeFromToolBar();
}


//! Returns the start time value of the current project.
//! @see getTimeStep()
//! @see getStopTime()
double SystemContainer::getStartTime()
{
    return mStartTime;
}


//! Returns the time step value of the current project.
//! @see getStartTime()
//! @see getStopTime()
double SystemContainer::getTimeStep()
{
    return mTimeStep;
}


//! Returns the stop time value of the current project.
//! @see getStartTime()
//! @see getTimeStep()
double SystemContainer::getStopTime()
{
    return mStopTime;
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

//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void SystemContainer::updateSimulationParametersInToolBar()
{
    gpMainWindow->setStartTimeInToolBar(mStartTime);
    gpMainWindow->setTimeStepInToolBar(mTimeStep);
    gpMainWindow->setFinishTimeInToolBar(mStopTime);
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
