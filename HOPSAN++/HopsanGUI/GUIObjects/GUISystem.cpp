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
#include "../MainWindow.h"
#include "../GraphicsView.h"
#include "../CoreAccess.h"
#include "../loadObjects.h"
#include "../GUIConnector.h"
#include "../UndoStack.h"
#include "../version.h"
#include "../Widgets/LibraryWidget.h"
#include "../Widgets/MessageWidget.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Dialogs/ContainerPropertiesDialog.h"
#include "../Utilities/GUIUtilities.h"
#include "../Widgets/PyDockWidget.h"

GUISystem::GUISystem(QPointF position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
    : GUIContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpParentProjectTab = pParentContainer->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
GUISystem::GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *pParent)
    : GUIContainerObject(QPointF(0,0), 0, 0, DESELECTED, USERGRAPHICS, 0, pParent)
{
    this->mGUIModelObjectAppearance = *(gpMainWindow->mpLibrary->getAppearanceData(HOPSANGUISYSTEMTYPENAME)); //This will crash if Subsystem not already loaded
    this->mpParentProjectTab = parentProjectTab;
    this->commonConstructorCode();
    this->mpUndoStack->newPost();
}

GUISystem::~GUISystem()
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
        //mpParentContainerObject->getCoreSystemAccessPtr()->deleteRootSystemPtr();
        mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @brief This code is common among the two constructors, we use one function to avoid code duplication
void GUISystem::commonConstructorCode()
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
        mGUIModelObjectAppearance.setName(mpParentContainerObject->getCoreSystemAccessPtr()->createSubSystem(this->getName()));
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
void GUISystem::setName(QString newName)
{
    if (mpParentContainerObject == 0)
    {
        setDisplayName(mpCoreSystemAccess->setRootSystemName(newName));
    }
    else
    {
        mpParentContainerObject->renameGUIModelObject(this->getName(), newName);
    }
}


//! Returns a string with the sub system type.
QString GUISystem::getTypeName()
{
    //! @todo is this OK should really ask the subsystem but result should be subsystem i think
    return HOPSANGUISYSTEMTYPENAME;
}

//! @brief Get the system cqs type
//! @returns A string containing the CQS type
QString GUISystem::getTypeCQS()
{
    return mpCoreSystemAccess->getRootSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QVector containing the parameter names
QVector<QString> GUISystem::getParameterNames()
{
    return mpCoreSystemAccess->getParameterNames(this->getName());
}

//! @brief Get a pointer the the CoreSystemAccess object that this system is representing
CoreSystemAccess* GUISystem::getCoreSystemAccessPtr()
{
    return this->mpCoreSystemAccess;
}

//! @brief Overloaded version that returns self if root system
GUIContainerObject *GUISystem::getParentContainerObject()
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


int GUISystem::type() const
{
    return Type;
}


//! @brief Opens the GUISystem properties dialog
void GUISystem::openPropertiesDialog()
{
    //! @todo shouldnt this be in the containerproperties class, right now groups are not working thats is why it is here, the containerproperties dialog only works with systems for now
    ContainerPropertiesDialog dialog(this, gpMainWindow);
    dialog.setAttribute(Qt::WA_DeleteOnClose, false);
    dialog.exec();
}


//! @brief Saves the System specific coredata to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
void GUISystem::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    GUIModelObject::saveCoreDataToDomElement(rDomElement);
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

//! @brief Saves the System specific GUI data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
QDomElement GUISystem::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    QDomElement guiStuff = GUIModelObject::saveGuiDataToDomElement(rDomElement);

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
        portsHiddenElement.setAttribute("hidden", mPortsHidden);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, HMF_NAMESTAG);
        namesHiddenElement.setAttribute("hidden", mNamesHidden);

        QDomElement scriptFileElement = appendDomElement(guiStuff, HMF_SCRIPTFILETAG);
        scriptFileElement.setAttribute("path", mScriptFilePath);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendDomElement(guiStuff, CAF_ROOT);
        xmlApp.setAttribute(CAF_VERSION, CAF_VERSIONNUM);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mGUIModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mGUIModelObjectAppearance.saveToDomElement(xmlApp);
    }

    guiStuff.appendChild(mpUndoStack->toXml());

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void GUISystem::saveToDomElement(QDomElement &rDomElement)
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
        GUIModelObject::saveCoreDataToDomElement(xmlSubsystem); //!< @todo Not sure why we should not use savecoredata in GUISystem instead, but it seems to be embeded specific
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
        GUIModelObjectMapT::iterator it;
        for(it = mGUIModelObjectMap.begin(); it!=mGUIModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlObjects);
        }

            //Save all widgets
        QMap<size_t, GUIWidget *>::iterator itw;
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
void GUISystem::loadFromDomElement(QDomElement &rDomElement)
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
        this->mGUIModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT));
        this->setDisplayName(realName); // This must be done becouse in some occations the loadAppearanceDataline above will overwrite the correct name
        this->mNamesHidden = guiStuff.firstChildElement(HMF_NAMESTAG).attribute("hidden").toInt();
        this->mPortsHidden = guiStuff.firstChildElement(HMF_PORTSTAG).attribute("hidden").toInt();
        gpMainWindow->mpToggleNamesAction->setChecked(!mNamesHidden);
        gpMainWindow->mpTogglePortsAction->setChecked(!mPortsHidden);
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
            loadSystemParameter(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_PARAMETERTAG);
        }

        //2. Load all sub-components
        QDomElement xmlSubObjects = rDomElement.firstChildElement(HMF_OBJECTS);
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_COMPONENTTAG);
        while (!xmlSubObject.isNull())
        {
            verifyHmfSubComponentCompatibility(xmlSubObject, hmfVersion);
            GUIModelObject* pObj = loadGUIModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            if(pObj == NULL)
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Model contains components from a library that has not been included.");
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

                //Load start values
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
            loadGUIModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
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


void GUISystem::saveToWrappedCode()
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

        //Write initial comment
    fileStream << "// Code from exported Hopsan model. This can be used in conjunction with HopsanCore by using HopsanWrapper. Subsystems probably don't work.\n\n";

        //Write system initialization
    fileStream << "initSystem(1e-3);\n\n";

        //Save components
    GUIModelObjectMapT::iterator it;
    for(it = mGUIModelObjectMap.begin(); it!=mGUIModelObjectMap.end(); ++it)
    {
        fileStream << "addComponent(\"" << it.value()->getName() << "\", \"" << it.value()->getTypeName() << "\");\n";
    }

    fileStream << "\n";

        //Save connectors
    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        fileStream << "connect(\"" << mSubConnectorList[i]->getStartComponentName() << "\", \"" << mSubConnectorList[i]->getStartPortName() <<
                      "\", \"" << mSubConnectorList[i]->getEndComponentName() << "\", \"" << mSubConnectorList[i]->getEndPortName() << "\");\n";
    }

    fileStream << "\n";

        //Save parameters
    for(it = mGUIModelObjectMap.begin(); it!=mGUIModelObjectMap.end(); ++it)
    {
        for(int i=0; i<it.value()->getParameterNames().size(); ++i)
        {
            fileStream << "setParameter(\"" << it.value()->getName() << "\", \"" << it.value()->getParameterNames().at(i) <<  "\", " << it.value()->getParameterValue(it.value()->getParameterNames().at(i)) << ");\n";
        }
    }

    fileStream << "\n";

        //Initialize components
    fileStream << "initComponents();";

    file.close();
}


void GUISystem::createSimulinkSourceFiles()
{
    QMessageBox::information(gpMainWindow, gpMainWindow->tr("Create Simulink Source Files"),
                             gpMainWindow->tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console.\n\nVisual Studio 2008 compiler is supported, although other versions might work as well.."));

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

    GUIModelObjectMapT::iterator it;
    for(it = mGUIModelObjectMap.begin(); it!=mGUIModelObjectMap.end(); ++it)
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


        //Open file dialog and initialize the file stream
    QDir fileDialogSaveDir;
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Simulink Source Files"),
                                                    fileDialogSaveDir.currentPath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

    qDebug() << "Selected path: " << savePath;
    QDir saveDir;
    saveDir.setPath(savePath);

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


    QTextStream portLabelsStream(&portLabelsFile);
    portLabelsStream << "set_param(gcb,'Mask','on')\n";
    portLabelsStream << "set_param(gcb,'MaskDisplay','";

    QTextStream wrapperStream(&wrapperFile);
    wrapperStream << "/*-----------------------------------------------------------------------------";
    wrapperStream << "This source file is part of Hopsan NG\n";
    wrapperStream << "\n";
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
    wrapperStream << "-----------------------------------------------------------------------------*/\n";
    wrapperStream << "\n";
    wrapperStream << "#define S_FUNCTION_NAME HopsanSimulink\n";
    wrapperStream << "#define S_FUNCTION_LEVEL 2\n";
    wrapperStream << "\n";
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
    wrapperStream << "#include \"include/Nodes/Nodes.h\"\n";
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
    wrapperStream << "#include \"include/ComponentUtilities/ReadDataCurve.h\"\n";
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
    wrapperStream << "#include \"include/Components/Components.h\"\n";
    wrapperStream << "\n";
    wrapperStream << "using namespace hopsan;\n";
    wrapperStream << "\n";
    wrapperStream << "ComponentSystem* pComponentSystem;\n";
    wrapperStream << "\n";
    wrapperStream << "static void mdlInitializeSizes(SimStruct *S)\n";
    wrapperStream << "{\n";
    wrapperStream << "    ssSetNumSFcnParams(S, 0);\n";
    wrapperStream << "    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))\n";
    wrapperStream << "    {\n";
    wrapperStream << "        return;\n";
    wrapperStream << "    }\n";
    wrapperStream << "\n";
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
    portLabelsStream << "port_label(''output''," << j+1 << ",''DEBUG''); ";
    wrapperStream << "\n";
    wrapperStream << "    ssSetNumSampleTimes(S, 1);\n";
    wrapperStream << "\n";
    wrapperStream << "    ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);\n";
    wrapperStream << "\n";
    wrapperStream << "    std::string hmfFilePath = \"" << mModelFileInfo.fileName() << "\";\n";
    wrapperStream << "    hopsan::HmfLoader coreHmfLoader;\n";
    wrapperStream << "    double startT = ssGetTStart(S);\n";
    wrapperStream << "    double stopT = ssGetTFinal(S);\n";
    wrapperStream << "    pComponentSystem = coreHmfLoader.loadModel(hmfFilePath, startT, stopT);\n";
    wrapperStream << "    pComponentSystem->setDesiredTimestep(0.001);\n";
    wrapperStream << "    pComponentSystem->initializeComponentsOnly();\n";
    wrapperStream << "\n";
    wrapperStream << "    mexCallMATLAB(0, 0, 0, 0, \"HopsanSimulinkPortLabels\");                               //Run the port label script\n";
    wrapperStream << "}\n";
    wrapperStream << "\n";
    wrapperStream << "static void mdlInitializeSampleTimes(SimStruct *S)\n";
    wrapperStream << "{\n";
    wrapperStream << "    ssSetSampleTime(S, 0, 0.001);\n";
    wrapperStream << "    ssSetOffsetTime(S, 0, 0.0);\n";
    wrapperStream << "}\n";
    wrapperStream << "\n";
    wrapperStream << "static void mdlOutputs(SimStruct *S, int_T tid)\n";
    wrapperStream << "{\n";
    wrapperStream << "    //S-function input signals\n";
    wrapperStream << "    InputRealPtrsType uPtrs1 = ssGetInputPortRealSignalPtrs(S,0);\n";
    wrapperStream << "\n";
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

    portLabelsStream << "')\n";
    portLabelsStream << "set_param(gcb,'BackgroundColor','[0.721569, 0.858824, 0.905882]')\n";
    portLabelsStream << "set_param(gcb,'Name','" << this->getName() << "')";
    portLabelsFile.close();

    QTextStream compileStream(&compileFile);
    compileStream << "%mex -DWIN32 -DSTATICCORE HopsanSimulink.cpp /include/Component.cc /include/ComponentSystem.cc /include/HopsanEssentials.cc /include/Node.cc /include/Port.cc /include/Components/Components.cc /include/CoreUtilities/HmfLoader.cc /include/CoreUtilities/HopsanCoreMessageHandler.cc /include/CoreUtilities/LoadExternal.cc /include/Nodes/Nodes.cc /include/ComponentUtilities/AuxiliarySimulationFunctions.cpp /include/ComponentUtilities/Delay.cc /include/ComponentUtilities/DoubleIntegratorWithDamping.cpp /include/ComponentUtilities/FirstOrderFilter.cc /include/ComponentUtilities/Integrator.cc /include/ComponentUtilities/IntegratorLimited.cc /include/ComponentUtilities/ludcmp.cc /include/ComponentUtilities/matrix.cc /include/ComponentUtilities/SecondOrderFilter.cc /include/ComponentUtilities/SecondOrderTransferFunction.cc /include/ComponentUtilities/TurbulentFlowFunction.cc /include/ComponentUtilities/ValveHysteresis.cc\n";
    compileStream << "mex -DWIN32 -DSTATICCORE -L./ -lHopsanCore HopsanSimulink.cpp\n";
    compileFile.close();


    QFile dllFile(gExecPath + "/../binVC/HopsanCore.dll");
    dllFile.copy(savePath + "/HopsanCore.dll");
    QFile libFile(gExecPath + "/../binVC/HopsanCore.lib");
    libFile.copy(savePath + "/HopsanCore.lib");
    QFile expFile(gExecPath + "/../binVC/HopsanCore.exp");
    expFile.copy(savePath + "/HopsanCore.exp");

    saveDir.mkdir("include");
    QFile componentH(gExecPath + QString(INCLUDEPATH) + "Component.h");
    componentH.copy(saveDir.path() + "/include/Component.h");
    QFile componentSystemH(gExecPath + QString(INCLUDEPATH) + "ComponentSystem.h");
    componentSystemH.copy(saveDir.path() + "/include/ComponentSystem.h");
    QFile componentEssentialsH(gExecPath + QString(INCLUDEPATH) + "ComponentEssentials.h");
    componentEssentialsH.copy(saveDir.path() + "/include/ComponentEssentials.h");
    QFile componentUtilitiesH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities.h");
    componentUtilitiesH.copy(saveDir.path() + "/include/ComponentUtilities.h");
    QFile hopsanCoreH(gExecPath + QString(INCLUDEPATH) + "HopsanCore.h");
    hopsanCoreH.copy(saveDir.path() + "/include/HopsanCore.h");
    QFile hopsanEssentialsH(gExecPath + QString(INCLUDEPATH) + "HopsanEssentials.h");
    hopsanEssentialsH.copy(saveDir.path() + "/include/HopsanEssentials.h");
    QFile nodeH(gExecPath + QString(INCLUDEPATH) + "Node.h");
    nodeH.copy(saveDir.path() + "/include/Node.h");
    QFile portH(gExecPath + QString(INCLUDEPATH) + "Port.h");
    portH.copy(saveDir.path() + "/include/Port.h");

    QDir componentsDir;
    componentsDir.mkdir(savePath + "/include/Components");
    QFile componentsH(gExecPath + QString(INCLUDEPATH) + "Components/Components.h");
    componentsH.copy(saveDir.path() + "/include/Components/Components.h");

    QDir componentUtilitiesDir;
    componentUtilitiesDir.mkdir(savePath + "/include/ComponentUtilities");
    QFile auxiliarySimulationFunctionsH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/AuxiliarySimulationFunctions.h");
    auxiliarySimulationFunctionsH.copy(saveDir.path() + "/include/ComponentUtilities/AuxiliarySimulationFunctions.h");
    QFile csvParserH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/CSVParser.h");
    csvParserH.copy(saveDir.path() + "/include/ComponentUtilities/CSVParser.h");
    QFile delayH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/Delay.hpp");
    delayH.copy(saveDir.path() + "/include/ComponentUtilities/Delay.hpp");
    QFile doubleIntegratorWithDampingH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/DoubleIntegratorWithDamping.h");
    doubleIntegratorWithDampingH.copy(saveDir.path() + "/include/ComponentUtilities/DoubleIntegratorWithDamping.h");
    QFile doubleIntegratorWithDampingAndCoulumbFrictionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h");
    doubleIntegratorWithDampingAndCoulumbFrictionH.copy(saveDir.path() + "/include/ComponentUtilities/DoubleIntegratorWithDampingAndCoulumbFriction.h");
    QFile firstOrderTransferFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/FirstOrderTransferFunction.h");
    firstOrderTransferFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/FirstOrderTransferFunction.h");
    QFile integratorH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/Integrator.h");
    integratorH.copy(saveDir.path() + "/include/ComponentUtilities/Integrator.h");
    QFile integratorLimitedH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/IntegratorLimited.h");
    integratorLimitedH.copy(saveDir.path() + "/include/ComponentUtilities/IntegratorLimited.h");
    QFile ludcmpH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ludcmp.h");
    ludcmpH.copy(saveDir.path() + "/include/ComponentUtilities/ludcmp.h");
    QFile matrixH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/matrix.h");
    matrixH.copy(saveDir.path() + "/include/ComponentUtilities/matrix.h");
    QFile readDataCurveH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ReadDataCurve.h");
    readDataCurveH.copy(saveDir.path() + "/include/ComponentUtilities/ReadDataCurve.h");
    QFile secondOrderTransferFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/SecondOrderTransferFunction.h");
    secondOrderTransferFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/SecondOrderTransferFunction.h");
    QFile turbulentFlowFunctionH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/TurbulentFlowFunction.h");
    turbulentFlowFunctionH.copy(saveDir.path() + "/include/ComponentUtilities/TurbulentFlowFunction.h");
    QFile valveHysteresisH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/ValveHysteresis.h");
    valveHysteresisH.copy(saveDir.path() + "/include/ComponentUtilities/ValveHysteresis.h");
    QFile whiteGaussianNoiseH(gExecPath + QString(INCLUDEPATH) + "ComponentUtilities/WhiteGaussianNoise.h");
    whiteGaussianNoiseH.copy(saveDir.path() + "/include/ComponentUtilities/WhiteGaussianNoise.h");

    QDir coreUtilitiesDir;
    coreUtilitiesDir.mkdir(savePath + "/include/CoreUtilities");
    QFile hmfLoaderH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/HmfLoader.h");
    hmfLoaderH.copy(saveDir.path() + "/include/CoreUtilities/HmfLoader.h");
    QFile classFactoryHpp(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/ClassFactory.hpp");
    classFactoryHpp.copy(saveDir.path() + "/include/CoreUtilities/ClassFactory.hpp");
    QFile classFactoryStatusCheckHpp(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/ClassFactoryStatusCheck.hpp");
    classFactoryStatusCheckHpp.copy(saveDir.path() + "/include/CoreUtilities/ClassFactoryStatusCheck.hpp");
    QFile findUniqueNameH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/FindUniqueName.h");
    findUniqueNameH.copy(saveDir.path() + "/include/CoreUtilities/FindUniqueName.h");
    QFile hopsanCoreMessageHandlerH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/HopsanCoreMessageHandler.h");
    hopsanCoreMessageHandlerH.copy(saveDir.path() + "/include/CoreUtilities/HopsanCoreMessageHandler.h");
    QFile loadExternalH(gExecPath + QString(INCLUDEPATH) + "/CoreUtilities/LoadExternal.h");
    loadExternalH.copy(saveDir.path() + "/include/CoreUtilities/LoadExternal.h");

    QDir nodesDir;
    nodesDir.mkdir(savePath + "/include/Nodes");
    QFile nodesH(gExecPath + QString(INCLUDEPATH) + "/Nodes/Nodes.h");
    nodesH.copy(saveDir.path() + "/include/Nodes/Nodes.h");




    //! @todo This code is duplicated from ProjectTab::saveModel(), make it a common function somehow
        //Save xml document
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
}


//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
void GUISystem::setModelFileInfo(QFile &rFile)
{
    this->mModelFileInfo.setFile(rFile);
}

//! Function that updates start time value of the current project to the one in the simulation setup widget.
//! @see updateTimeStep()
//! @see updateStopTime()
void GUISystem::updateStartTime()
{
    mStartTime = gpMainWindow->getStartTimeFromToolBar();
}


//! Function that updates time step value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateStopTime()
void GUISystem::updateTimeStep()
{
    mTimeStep = gpMainWindow->getTimeStepFromToolBar();
}


//! Function that updates stop time value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateTimeStep()
void GUISystem::updateStopTime()
{
    mStopTime = gpMainWindow->getFinishTimeFromToolBar();
}


//! Returns the start time value of the current project.
//! @see getTimeStep()
//! @see getStopTime()
double GUISystem::getStartTime()
{
    return mStartTime;
}


//! Returns the time step value of the current project.
//! @see getStartTime()
//! @see getStopTime()
double GUISystem::getTimeStep()
{
    return mTimeStep;
}


//! Returns the stop time value of the current project.
//! @see getStartTime()
//! @see getTimeStep()
double GUISystem::getStopTime()
{
    return mStopTime;
}


//! Returns the number of samples value of the current project.
//! @see setNumberOfLogSamples(double)
size_t GUISystem::getNumberOfLogSamples()
{
    return mNumberOfLogSamples;
}


//! Sets the number of samples value for the current project
//! @see getNumberOfLogSamples()
void GUISystem::setNumberOfLogSamples(size_t nSamples)
{
    mNumberOfLogSamples = nSamples;
}

//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void GUISystem::updateSimulationParametersInToolBar()
{
    gpMainWindow->setStartTimeInToolBar(mStartTime);
    gpMainWindow->setTimeStepInToolBar(mTimeStep);
    gpMainWindow->setFinishTimeInToolBar(mStopTime);
}
