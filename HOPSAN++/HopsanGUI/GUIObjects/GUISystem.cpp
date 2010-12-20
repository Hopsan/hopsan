//$Id$

//#include <float.h>
#include "GUISystem.h"

//! @todo clean these up
#include "../Widgets/ProjectTabWidget.h"
#include "../MainWindow.h"
#include "../Dialogs/ContainerPropertiesDialog.h"
#include "../GUIPort.h"
#include "../GUIConnector.h"
#include "../Utilities/GUIUtilities.h"
#include "../UndoStack.h"
#include "../Widgets/MessageWidget.h"
#include "../GraphicsView.h"
#include "../Widgets/LibraryWidget.h"
#include "../loadObjects.h"
#include "../CoreAccess.h"
#include "GUIComponent.h"
#include "GUIGroup.h"
#include "GUISystemPort.h"
#include "GUIWidgets.h"
#include "Widgets/PlotWidget.h"

GUISystem::GUISystem(QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
    : GUIContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    this->mpParentProjectTab = system->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
GUISystem::GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent)
    : GUIContainerObject(QPoint(0,0), 0, 0, DESELECTED, USERGRAPHICS, 0, parent)
{
    this->mGUIModelObjectAppearance = *(gpMainWindow->mpLibrary->getAppearanceData("Subsystem")); //This will crash if Subsystem not already loaded
    this->mpParentProjectTab = parentProjectTab;
    this->commonConstructorCode();
    this->mUndoStack->newPost();
}

GUISystem::~GUISystem()
{
    //! @todo should remove all subcomponents first then run the code bellow, to cleanup nicely in the correct order

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
        mpCoreSystemAccess->setRootTypeCQS("S");
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
    return "Subsystem";
}

void GUISystem::setTypeCQS(QString typestring)
{
    mpCoreSystemAccess->setRootTypeCQS(typestring);
}

QString GUISystem::getTypeCQS()
{
    return mpCoreSystemAccess->getRootSystemTypeCQS();
}

QVector<QString> GUISystem::getParameterNames()
{
    return mpCoreSystemAccess->getParameterNames(this->getName());
}

CoreSystemAccess* GUISystem::getCoreSystemAccessPtr()
{
    return this->mpCoreSystemAccess;
}

void GUISystem::loadFromHMF(QString modelFilePath)
{
    //! @todo maybe break out the load file function it is used in many places (with some diffeerenses every time), should be enough to return file and filinfo obejct maybe
    QFile file;
    if (modelFilePath.isEmpty())
    {
        QFileInfo fileInfo;
        QDir fileDialog;
        modelFilePath = QFileDialog::getOpenFileName(mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
                                                             fileDialog.currentPath() + QString(MODELPATH),
                                                             tr("Hopsan Model Files (*.hmf)"));
        if (modelFilePath.isEmpty())
            return;

        file.setFileName(modelFilePath);
        fileInfo.setFile(file);
        for(int t=0; t!=mpParentProjectTab->mpParentProjectTabWidget->count(); ++t)
        {
            if( (mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == fileInfo.fileName()) ||  (mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == (fileInfo.fileName() + "*")) )
            {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::information(mpParentProjectTab->mpParentProjectTabWidget, tr("Error"), tr("Unable to load model. File is already open."));
                return;
            }
        }
    }
    else
    {
        //Open the file with a path relative to the parent system, in case of rootsystem assume that the given path is absolute and correct
        if (mpParentContainerObject != 0)
        {
            //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
            //! @todo what if the parent system does not have a path (embeded systems)
            file.setFileName(this->mpParentContainerObject->mModelFileInfo.absolutePath() + "/" + modelFilePath);
        }
        else
        {
            file.setFileName(modelFilePath);
        }
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + file.fileName();
        return;
    }
    mModelFileInfo.setFile(file);

    QTextStream textStreamFile(&file); //Converts to QTextStream
    mUndoStack->newPost();

    //Set the name
    this->setName(mModelFileInfo.baseName());

    //Now read the file data
    //Read the header data, also checks version numbers
    //! @todo maybe not check the version numbers in there
    HeaderLoadData headerData = readHeader(textStreamFile, gpMainWindow->mpMessageWidget);

    //Only set this stuff if this is the root system, (that is if no systemparent exist)
    if (this->mpParentContainerObject == 0)
    {
        //It is assumed that these data have been successfully read
        gpMainWindow->setStartTimeInToolBar(headerData.startTime);
        gpMainWindow->setTimeStepInToolBar(headerData.timeStep);
        gpMainWindow->setFinishTimeInToolBar(headerData.stopTime);

        //It is assumed that these data have been successfully read
        mpParentProjectTab->mpGraphicsView->centerOn(headerData.viewport_x, headerData.viewport_y);
        mpParentProjectTab->mpGraphicsView->scale(headerData.viewport_zoomfactor, headerData.viewport_zoomfactor);
        mpParentProjectTab->mpGraphicsView->mZoomFactor = headerData.viewport_zoomfactor;
        mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

    //Read the system appearance data
    SystemAppearanceLoadData sysappdata;
    sysappdata.read(textStreamFile);

    if (!sysappdata.usericon_path.isEmpty())
    {
        mGUIModelObjectAppearance.setIconPathUser(sysappdata.usericon_path);
    }
    if (!sysappdata.isoicon_path.isEmpty())
    {
        mGUIModelObjectAppearance.setIconPathISO(sysappdata.isoicon_path);
    }

    //! @todo reading portappearance should have a common function and be shared with the setappearancedata read function that reads from caf files
    PortAppearanceMapT* portappmap = &(mGUIModelObjectAppearance.getPortAppearanceMap());
    for (int i=0; i<sysappdata.portnames.size(); ++i)
    {
        GUIPortAppearance portapp;
        portapp.x = sysappdata.port_xpos[i];
        portapp.y = sysappdata.port_ypos[i];
        portapp.rot = sysappdata.port_angle[i];

        portapp.selectPortIcon("","",""); //!< @todo fix this, (maybe not necessary to fix)

        portappmap->insert(sysappdata.portnames[i], portapp);
        //qDebug() << sysappdata.portnames[i];
    }

    qDebug() << "Appearance set for system: " << this->getName();
    qDebug() << "loadFromHMF contents, name: " << this->getName();
    //Now load the contens of the subsystem
    while ( !textStreamFile.atEnd() )
    {
        //Extract first word on line
        QString inputWord;
        textStreamFile >> inputWord;

        //! @todo what about NOUNDO here should we be able to select this from the outside maybe

        if ( (inputWord == "SUBSYSTEM") ||  (inputWord == "BEGINSUBSYSTEM") )
        {
            loadSubsystemGUIObject(textStreamFile, gpMainWindow->mpLibrary, this, NOUNDO);
        }

        if ( (inputWord == "COMPONENT") || (inputWord == "SYSTEMPORT") )
        {
            loadGUIModelObject(textStreamFile, gpMainWindow->mpLibrary, this, NOUNDO);
        }

        if ( inputWord == "PARAMETER" )
        {
            loadParameterValues(textStreamFile, this, NOUNDO);
        }

        if ( inputWord == "CONNECT" )
        {
            loadConnector(textStreamFile, this, NOUNDO);
        }
    }

    //Refresh the appearnce of the subsystemem and create the GUIPorts
    //! @todo This is a bit strange, refreshAppearance MUST be run before create ports or create ports will not know some necessary stuff
    this->refreshAppearance();
    //this->createPorts();
    this->refreshExternalPortsAppearanceAndPosition();

    //Deselect all components
    this->deselectAll(); //! @todo maybe this should be a signal
    this->mUndoStack->clear();
    //Only do this for the root system
    //! @todo maybe can do this for subsystems to (even if we dont see them right now)
    if (this->mpParentContainerObject == 0)
    {
        //mpParentProjectTab->mpGraphicsView->centerView();
        mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

    file.close();
    emit checkMessages();
}


int GUISystem::type() const
{
    return Type;
}


void GUISystem::openPropertiesDialog()
{
    ContainerPropertiesDialog dialog(this, gpMainWindow);
    dialog.setAttribute(Qt::WA_DeleteOnClose, false);
    dialog.exec();
}


//! @todo maybe have a inherited function in some other base class that are specific for guiobjects with core equivalent
void GUISystem::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    GUIModelObject::saveCoreDataToDomElement(rDomElement);
    rDomElement.setAttribute(HMF_CQSTYPETAG, this->getTypeCQS());
    appendSimulationTimeTag(rDomElement, this->mStartTime, this->mTimeStep, this->mStopTime);

    QDomElement parElement = appendDomElement(rDomElement, HMF_PARAMETERS);
    QList<QStringList> favPars = this->getFavoriteParameters();
    QList<QStringList>::iterator itf;
    for(itf = favPars.begin(); itf != favPars.end(); ++itf)
    {
        QDomElement favoriteElement = appendDomElement(parElement, HMF_FAVORITEPARAMETERTAG);
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
}

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

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendDomElement(guiStuff, CAF_ROOTTAG);
        this->mGUIModelObjectAppearance.saveToDomElement(xmlApp);
    }
    return guiStuff;
}

void GUISystem::saveToDomElement(QDomElement &rDomElement)
{
    //qDebug() << "Saving to dom node in: " << this->mGUIModelObjectAppearance.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, mHmfTagName);

    //! @todo maybe use enums instead
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
        this->setTypeCQS(rDomElement.attribute(HMF_CQSTYPETAG));

        //Load the GUI stuff like appearance data and viewport
        QDomElement guiStuff = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
        this->mGUIModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOTTAG).firstChildElement("modelobject"));
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
                //! @todo load start values
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
            loadSubsystemGUIObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //6. Load all systemports
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadGUIModelObject(xmlSubObject, gpMainWindow->mpLibrary, this, NOUNDO);
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

        //8. Load favorite parameters
        xmlSubObject = xmlParameters.firstChildElement(HMF_FAVORITEPARAMETERTAG);
        while (!xmlSubObject.isNull())
        {
            loadFavoriteParameter(xmlSubObject, this);

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_FAVORITEPARAMETERTAG);
        }

        //Refresh the appearnce of the subsystemem and create the GUIPorts based on the loaded portappearance information
        //! @todo This is a bit strange, refreshAppearance MUST be run before create ports or create ports will not know some necessary stuff
        this->refreshAppearance();
        this->refreshExternalPortsAppearanceAndPosition();
        //this->createPorts();

        //Deselect all components
        this->deselectAll(); //! @todo maybe this should be a signal
        this->mUndoStack->clear();
        //Only do this for the root system
        //! @todo maybe can do this for subsystems to (even if we dont see them right now)
        if (this->mpParentContainerObject == 0)
        {
            //mpParentProjectTab->mpGraphicsView->centerView();
            mpParentProjectTab->mpGraphicsView->updateViewPort();
        }
        this->mpParentProjectTab->setSaved(true);

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
