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
#include "../Widgets/LibraryWidget.h"
#include "../Widgets/MessageWidget.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Dialogs/ContainerPropertiesDialog.h"
#include "../Utilities/GUIUtilities.h"
#include "../Widgets/PyDockWidget.h"

GUISystem::GUISystem(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected, graphicsType gfxType)
    : GUIContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpParentProjectTab = pParentContainer->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
GUISystem::GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *pParent)
    : GUIContainerObject(QPoint(0,0), 0, 0, DESELECTED, USERGRAPHICS, 0, pParent)
{
    this->mGUIModelObjectAppearance = *(gpMainWindow->mpLibrary->getAppearanceData(HOPSANGUISYSTEMTYPENAME)); //This will crash if Subsystem not already loaded
    this->mpParentProjectTab = parentProjectTab;
    this->commonConstructorCode();
    this->mUndoStack->newPost(); //!< @todo why do we need undostack new post here
}

GUISystem::~GUISystem()
{
    qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,GUISystem destructor";
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
        //mpCoreSystemAccess->setRootTypeCQS("S");
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

////! @brief Set the system cqs type
////! @param[in] typestring A string containgin the CQS type, C Q or S is accepted
//void GUISystem::setTypeCQS(QString typestring)
//{
//    mpCoreSystemAccess->setRootTypeCQS(typestring);
//}

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
    //GUIModelObject::saveCoreDataToDomElement(rDomElement);
    //We dont need to save the type in systems
    rDomElement.setAttribute(HMF_NAMETAG, getName());
    rDomElement.setAttribute(HMF_CQSTYPETAG, this->getTypeCQS());
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

    QMap<std::string, double>::iterator it;
    QMap<std::string, double> parMap = mpCoreSystemAccess->getSystemParametersMap();
    for(it = parMap.begin(); it != parMap.end(); ++it)
    {
        QDomElement mappedElement = appendDomElement(parElement, HMF_PARAMETERTAG);
        mappedElement.setAttribute("name", QString(it.key().c_str()));
        mappedElement.setAttribute("value", it.value());
    }

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
        if (this->mpParentProjectTab->mpGraphicsView != 0)
        {
            qreal x,y,zoom;
            this->mpParentProjectTab->mpGraphicsView->getViewPort(x,y,zoom);
            appendViewPortTag(guiStuff, x, y, zoom);
        }
        QDomElement portsHiddenElement = appendDomElement(guiStuff, HMF_PORTSTAG);
        portsHiddenElement.setAttribute("hidden", mPortsHidden);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, HMF_NAMESTAG);
        namesHiddenElement.setAttribute("hidden", mNamesHidden);

        QDomElement scriptFileElement = appendDomElement(guiStuff, HMF_SCRIPTFILETAG);
        scriptFileElement.setAttribute("path", mScriptFilePath);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendDomElement(guiStuff, CAF_ROOTTAG);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mGUIModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mGUIModelObjectAppearance.saveToDomElement(xmlApp);
    }
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
        xmlSubsystem.setAttribute( HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentContainerObject->mModelFileInfo.absolutePath()) );

        //Save the name that we have set for this subsystem, this name will overwrite the defualt one in the external file
        xmlSubsystem.setAttribute( HMF_NAMETAG, this->getName());
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

            //Save all text widgets
        for(int i = 0; i != mTextWidgetList.size(); ++i)
        {
            mTextWidgetList[i]->saveToDomElement(xmlObjects);
        }

            //Save all box widgets
        for(int i = 0; i != mBoxWidgetList.size(); ++i)
        {
            mBoxWidgetList[i]->saveToDomElement(xmlObjects);
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
    //Check if the subsystem is external or internal, and load appropriately
    QString external_path = rDomElement.attribute(HMF_EXTERNALPATHTAG);
    if (external_path.isEmpty())
    {
        //Load embedded subsystem
        //0. Load core and gui stuff
        //! @todo might need some error checking here incase some fields are missing
        //Now load the core specific data, might need inherited function for this
        this->setName(rDomElement.attribute(HMF_NAMETAG));
        //this->setTypeCQS(rDomElement.attribute(HMF_CQSTYPETAG));

        //Load the GUI stuff like appearance data and viewport
        QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
        this->mGUIModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOTTAG).firstChildElement("modelobject"));
        this->mNamesHidden = guiStuff.firstChildElement(HMF_NAMESTAG).attribute("hidden").toInt();
        this->mPortsHidden = guiStuff.firstChildElement(HMF_PORTSTAG).attribute("hidden").toInt();
        gpMainWindow->toggleNamesAction->setChecked(!mNamesHidden);
        gpMainWindow->togglePortsAction->setChecked(!mPortsHidden);
        double x = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("x").toDouble();
        double y = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("y").toDouble();
        double zoom = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("zoom").toDouble();
        setScriptFile(guiStuff.firstChildElement(HMF_SCRIPTFILETAG).attribute("path"));

        mpParentProjectTab->mpGraphicsView->scale(zoom, zoom);
        mpParentProjectTab->mpGraphicsView->mZoomFactor = zoom;
        //emit mpParentProjectTab->mpGraphicsView->zoomChange(zoom);

        qDebug() << "Center on " << x << ", " << y;
        mpParentProjectTab->mpGraphicsView->centerOn(x, y);
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
            GUIModelObject* pObj = loadGUIModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            if(pObj == NULL)
            {
                gpMainWindow->mpMessageWidget->printGUIErrorMessage("Model contains components from a library that has not been included.");
                break;
            }
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

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }
        qDebug() << "Loading text widgets!";
        //3. Load all text widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_TEXTWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextWidget(xmlSubObject, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTWIDGETTAG);
        }

        //4. Load all box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_BOXWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadBoxWidget(xmlSubObject, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_BOXWIDGETTAG);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadGUISystemObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
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
        while (!xmlSubObject.isNull())
        {
            loadConnector(xmlSubObject, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_CONNECTORTAG);
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
        this->mUndoStack->clear();
        //Only do this for the root system
        //! @todo maybe can do this for subsystems to (even if we dont see them right now)
        if (this->mpParentContainerObject == 0)
        {
            //mpParentProjectTab->mpGraphicsView->centerView();
            mpParentProjectTab->mpGraphicsView->updateViewPort();
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
