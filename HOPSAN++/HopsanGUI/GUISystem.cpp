#include <float.h>
#include "GUISystem.h"

#include "GUIObject.h"
#include "GUIComponent.h"
#include "ProjectTabWidget.h"
#include "MainWindow.h"
#include "ParameterDialog.h"
#include "GUIPort.h"
#include "GUIConnector.h"
#include "GUIUtilities.h"
#include "UndoStack.h"
#include "MessageWidget.h"
#include "GraphicsScene.h"
#include "GraphicsView.h"
#include "LibraryWidget.h"
#include "loadObjects.h"
#include "CoreSystemAccess.h"
#include "GUIGroup.h"
#include "GUISystemPort.h"
#include "GUIWidgets.h"

GUISystem::GUISystem(QPoint position, qreal rotation, const AppearanceData* pAppearanceData, GUISystem *system, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
    : GUIContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, system, parent)
{
    this->mpParentProjectTab = system->mpParentProjectTab;
    this->commonConstructorCode();
}

//Root system specific constructor
GUISystem::GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent)
    : GUIContainerObject(QPoint(0,0), 0, &AppearanceData(), DESELECTED, USERGRAPHICS, 0, parent)
{
    this->mpParentProjectTab = parentProjectTab;
    this->commonConstructorCode();
}

GUISystem::~GUISystem()
{
    //! @todo should remove all subcomponents first then run the code bellow, to cleanup nicely in the correct order

    if (mpParentSystem != 0)
    {
        mpParentSystem->mpCoreSystemAccess->removeSubComponent(this->getName(), true);
    }
    else
    {
        mpParentSystem->mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @todo ugly temporary hack
void GUISystem::commonConstructorCode()
{
    //Set the hmf save tag name
    mHmfTagName = HMF_SYSTEMTAG;

    mpScene = new GraphicsScene();
    mpScene->addItem(this);     //! Detta kan gå åt helsike
                                //! @todo Should systems belong to their own scene?! This is why display names appear in the system's scene...

    mpMainWindow = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow;

    mUndoStack = new UndoStack(this);

        //Initialize booleans
    setIsCreatingConnector(false);
    mIsRenamingObject = false;
    mPortsHidden = false;
    mUndoDisabled = false;

        //Set default values
    mLoadType = "EMBEDED";
    //mModelFileInfo.setFile();
    mStartTime = 0;     //! @todo These default values should be options for the user
    mTimeStep = 0.001;
    mStopTime = 10;
    mGfxType = USERGRAPHICS;
    mNumberOfSamples = 2048;

        //Establish connections
    //connect(this->systemPortAction, SIGNAL(triggered()), SLOT(addSystemPort()));
    connect(this, SIGNAL(checkMessages()), mpMainWindow->mpMessageWidget, SLOT(checkMessages()));
    connect(mpMainWindow->undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    connect(mpMainWindow->redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    connect(mpMainWindow->mpUndoWidget->getUndoButton(), SIGNAL(pressed()), this, SLOT(undo()));
    connect(mpMainWindow->mpUndoWidget->getRedoButton(), SIGNAL(pressed()), this, SLOT(redo()));
    connect(mpMainWindow->mpUndoWidget->getClearButton(), SIGNAL(pressed()), this, SLOT(clearUndo()));
    connect(mpMainWindow->hidePortsAction, SIGNAL(triggered(bool)), this, SLOT(hidePorts(bool)));

    //Create the object in core, and update name
    if (this->mpParentSystem == 0)
    {
        //Create root system
        qDebug() << "creating ROOT access system";
        mpCoreSystemAccess = new CoreSystemAccess();
        mpCoreSystemAccess->setRootSystemName("RootSystem");
        this->setDisplayName(mpCoreSystemAccess->getRootSystemName());
    }
    else
    {
        //Create subsystem
        qDebug() << "creating subsystem and setting name in " << mpParentSystem->mpCoreSystemAccess->getRootSystemName();
        mAppearanceData.setName(mpParentSystem->mpCoreSystemAccess->createSubSystem(this->getName()));
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentSystem->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentSystem->mpCoreSystemAccess);
    }

    mpCoreSystemAccess->setDesiredTimeStep(mTimeStep);
    mpCoreSystemAccess->setRootTypeCQS("S");

    refreshDisplayName(); //Make sure name window is correct size for center positioning
}


//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//!
void GUISystem::setName(QString newName)
{
    if (mpParentSystem == 0)
    {
        //! @todo This means that the display name will appearn in the graphics view, since it belongs to the systems scene for some reason...
        setDisplayName(mpCoreSystemAccess->setRootSystemName(newName));
    }
    else
    {
        mpParentSystem->renameGUIObject(this->getName(), newName);
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
        if (mpParentSystem != 0)
        {
            //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
            //! @todo what if the parent system does not have a path (embeded systems)
            file.setFileName(this->mpParentSystem->mModelFileInfo.absolutePath() + "/" + modelFilePath);
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
    HeaderLoadData headerData = readHeader(textStreamFile, mpMainWindow->mpMessageWidget);

    //Only set this stuff if this is the root system, (that is if no systemparent exist)
    if (this->mpParentSystem == 0)
    {
        //It is assumed that these data have been successfully read
        mpMainWindow->setStartTimeInToolBar(headerData.startTime);
        mpMainWindow->setTimeStepInToolBar(headerData.timeStep);
        mpMainWindow->setFinishTimeInToolBar(headerData.stopTime);

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
        mAppearanceData.setIconPathUser(sysappdata.usericon_path);
    }
    if (!sysappdata.isoicon_path.isEmpty())
    {
        mAppearanceData.setIconPathISO(sysappdata.isoicon_path);
    }

    //! @todo reading portappearance should have a common function and be shared with the setappearancedata read function that reads from caf files
    PortAppearanceMapT* portappmap = &(mAppearanceData.getPortAppearanceMap());
    for (int i=0; i<sysappdata.portnames.size(); ++i)
    {
        GUIPortAppearance portapp;
        portapp.x = sysappdata.port_xpos[i];
        portapp.y = sysappdata.port_ypos[i];
        portapp.rot = sysappdata.port_angle[i];
//        if( (portapp.rot == 0) || (portapp.rot == 180) )
//        {
//            portapp.direction = LEFTRIGHT;
//        }
//        else
//        {
//            portapp.direction = TOPBOTTOM;
//        }
//        //! @todo portdirection in portapperance should have an initial default value to avoid crash if not set when creating connector
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
            loadSubsystemGUIObject(textStreamFile, mpMainWindow->mpLibrary, this, NOUNDO);
        }

        if ( (inputWord == "COMPONENT") || (inputWord == "SYSTEMPORT") )
        {
            loadGUIModelObject(textStreamFile, mpMainWindow->mpLibrary, this, NOUNDO);
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
    this->createPorts();

    //Deselect all components
    this->deselectAll();
    this->mUndoStack->clear();
    //Only do this for the root system
    //! @todo maybe can do this for subsystems to (even if we dont see them right now)
    if (this->mpParentSystem == 0)
    {
        //mpParentProjectTab->mpGraphicsView->centerView();
        mpParentProjectTab->mpGraphicsView->updateViewPort();
    }

    file.close();
    emit checkMessages();
}

////! @todo Maybe should be somewhere else and be called load subsystem
//void GUISystem::loadFromFileNOGUI(QString modelFileName)
//{
//    QFile file;
//    QFileInfo fileInfo;
//    if (modelFileName.isEmpty())
//    {
//        QDir fileDialog;
//        modelFileName = QFileDialog::getOpenFileName(mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
//                                                             fileDialog.currentPath() + QString("/../../Models"),
//                                                             tr("Hopsan Model Files (*.hmf)"));
//        if (modelFileName.isEmpty())
//            return;

//        file.setFileName(modelFileName);
//        fileInfo.setFile(file);

//        for(int t=0; t!=mpParentProjectTab->mpParentProjectTabWidget->count(); ++t)
//        {
//            if( (mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == fileInfo.fileName()) or (mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == (fileInfo.fileName() + "*")) )
//            {
//                QMessageBox::StandardButton reply;
//                reply = QMessageBox::information(mpParentProjectTab->mpParentProjectTabWidget, tr("Error"), tr("Unable to load model. File is already open."));
//                return;
//            }
//        }
//    }
//    else
//    {
//         file.setFileName(modelFileName);
//         fileInfo.setFile(file);
//    }

//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
//    {
//        qDebug() << "Failed to open file or not a text file: " + modelFileName;
//        return;
//    }
//    QTextStream textStreamFile(&file); //Converts to QTextStream
//    mModelFilePath = modelFileName;

//    //Set the name
//    this->mpParentSystem->renameGUIObject(this->getName(), fileInfo.baseName());

//    //Now read the file data
//    SystemAppearanceLoadData sysappdata;
//    HeaderLoadData header;

//    header.read(textStreamFile);
//    //qDebug() << "Header read";
//    //! @todo check so that version OK!
//    sysappdata.read(textStreamFile);
//    //qDebug() << "Sysapp data read";

//    if (!sysappdata.usericon_path.isEmpty())
//    {
//        mAppearanceData.setQString(ICONPATH)User(sysappdata.usericon_path);
//    }
//    if (!sysappdata.isoicon_path.isEmpty())
//    {
//        mAppearanceData.setQString(ICONPATH)ISO(sysappdata.isoicon_path);
//    }

//    //! @todo reading portappearance should have a common function and be shared with the setappearancedata rad function that reads from caf files
//    PortAppearanceMapT* portappmap = &(mAppearanceData.getPortAppearanceMap());
//    for (int i=0; i<sysappdata.portnames.size(); ++i)
//    {
//        PortAppearance portapp;
//        portapp.x = sysappdata.port_xpos[i];
//        portapp.y = sysappdata.port_ypos[i];
//        portapp.rot = sysappdata.port_angle[i];
//        if( (portapp.rot == 0) || (portapp.rot == 180) )
//        {
//            portapp.direction = LEFTRIGHT;
//        }
//        else
//        {
//            portapp.direction = TOPBOTTOM;
//        }
//        //! @todo portdirection in portapperance should have an initial default value to avoid crash if not set when creating connector
//        portapp.selectPortIcon("","",""); //!< @todo fix this

//        portappmap->insert(sysappdata.portnames[i], portapp);
//        qDebug() << sysappdata.portnames[i];
//    }
//    qDebug() << "Appearance set";

//    //Load the contents of the subsystem from the external file
//    mpParentProjectTab->mpSystem->mpCoreSystemAccess->loadSystemFromFileCoreOnly(this->getName(), modelFileName);
//    qDebug() << "Loaded in core";

//    this->refreshAppearance();
//    this->createPorts();
//    this->refreshDisplayName();
//    file.close();
//}


int GUISystem::type() const
{
    return Type;
}


//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUISystem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;
        if (!this->scene()->selectedItems().empty())
            groupAction = menu.addAction(tr("Group components"));

        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *showNameAction = menu.addAction(tr("Show name"));
        showNameAction->setCheckable(true);
        showNameAction->setChecked(mpNameText->isVisible());

        QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
        if(!mModelFileInfo.filePath().isEmpty()) loadAction->setDisabled(true);

        QAction *selectedAction = menu.exec(event->screenPos());



        if (selectedAction == parameterAction)
        {
            openParameterDialog();
        }
        else if (selectedAction == groupAction)
        {
            //groupComponents(mpParentGraphicsScene->selectedItems());
            AppearanceData appdata;
            appdata.setIconPathUser("subsystemtmp.svg");
            appdata.setBasePath("../../HopsanGUI/"); //!< @todo This is EXTREAMLY BAD
            GUIGroup *pGroup = new GUIGroup(this->scene()->selectedItems(), &appdata, mpParentSystem);
            mpScene->addItem(pGroup);
        }
        else if (selectedAction == showNameAction)
        {
            if(mpNameText->isVisible())
            {
                this->hideName();
            }
            else
            {
                this->showName();
            }
        }
        else if (selectedAction == loadAction)
        {
            loadFromHMF();
        }
    }



void GUISystem::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this);
    dialog->exec();
}

void GUISystem::createPorts()
{
    //! @todo make sure that all old ports and connections are cleared, (in case we reload, but maybe we can discard old system and create new in that case)
    //Create the graphics for the ports but do NOT create new ports, use the system ports within the subsystem
    PortAppearanceMapT::iterator it;
    for (it = mAppearanceData.getPortAppearanceMap().begin(); it != mAppearanceData.getPortAppearanceMap().end(); ++it)
    {
        //! @todo fix this
        qDebug() << "getNode and portType for " << it.key();
        QString nodeType = mpCoreSystemAccess->getNodeType(this->getName(), it.key());
        QString portType = mpCoreSystemAccess->getPortType(this->getName(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        qreal x = it.value().x;
        qreal y = it.value().y;

        qDebug() << "this-type(): " << this->type();
        GUIPort *pNewPort = new GUIPort(it.key(), x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height(), &(it.value()), this);
        mPortListPtrs.append(pNewPort);
    }
}

//! @brief Save GUISystem to a text stream
//! @todo here we are NOT using the save function in the guiobject base class becouse subsystems are saved completely differently, need to make this more uniform in the future
void GUISystem::saveToTextStream(QTextStream &rStream, QString prepend)
{
    QPointF pos = mapToScene(boundingRect().center());
    if (!prepend.isEmpty())
    {
        rStream << prepend << " ";
    }

    if (!mModelFileInfo.filePath().isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    rStream << addQuotes(mLoadType) << " " << addQuotes(getName()) << " " << addQuotes(getTypeCQS()) << " "
            << addQuotes(relativePath(mModelFileInfo.absoluteFilePath(), mpParentSystem->mModelFileInfo.absolutePath())) << " "
            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << " " << mpNameText->isVisible() << "\n";
}

//! @todo maybe have a inherited function in some other base class that are specific for guiobjects with core equivalent
void GUISystem::saveCoreDataToDomElement(QDomElement &rDomElement)
{
    appendDomTextNode(rDomElement, HMF_TYPETAG, getTypeName());
    appendDomTextNode(rDomElement, HMF_NAMETAG, getName());
}

void GUISystem::saveToDomElement(QDomElement &rDomElement)
{
    //! @todo maybe use enums instead
    //! @todo should not need to set this here
    if (mpParentSystem==0)
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

    qDebug() << "Saving to dom node in: " << this->mAppearanceData.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, mHmfTagName);

    //Save Core related stuff
    this->saveCoreDataToDomElement(xmlSubsystem);

    //! @todo do we really need both systemtype and external path, en empty path could indicate embeded
    //appendDomTextNode(subsysContainerNode, "SystemType", mLoadType);
    appendDomTextNode(xmlSubsystem, HMF_CQSTYPETAG, getTypeCQS());
    if ((mpParentSystem != 0) && (mLoadType=="EXTERNAL"))
    {
        appendDomTextNode(xmlSubsystem, HMF_EXTERNALPATHTAG, relativePath(mModelFileInfo.absoluteFilePath(), mpParentSystem->mModelFileInfo.absolutePath()));
    }
    else
    {
        //appendDomTextNode(subsysContainerNode, "ExternalPath", QString()); //Maybe dont need this on root systems
    }

    //Save gui object stuff
    saveGuiDataToDomElement(xmlSubsystem);

    //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
        GUIModelObjectMapT::iterator it;
        for(it = mGUIModelObjectMap.begin(); it!=mGUIModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlSubsystem);
        }

        //Save the connectors
        for(int i = 0; i != mSubConnectorList.size(); ++i)
        {
            mSubConnectorList[i]->saveToDomElement(xmlSubsystem);
        }

        //Save all text widgets
        for(int i = 0; i != mTextWidgetList.size(); ++i)
        {
            mTextWidgetList[i]->saveToDomElement(xmlSubsystem);
        }
    }
}

void GUISystem::loadFromDomElement(QDomElement &rDomElement)
{
    //! @todo might need some error checking here incase some fields are missing
    //First load the core specific data, might need inherited function for this
    this->setName(rDomElement.firstChildElement(HMF_NAMETAG).text());
    this->setTypeCQS(rDomElement.firstChildElement(HMF_CQSTYPETAG).text());

    //Check if the subsystem is external or internal, and load appropriately
    QString external_path = rDomElement.firstChildElement(HMF_EXTERNALPATHTAG).text();
    if (external_path.isEmpty())
    {
        //Load embedded subsystem
        //1. Load all sub-components
        QDomElement xmlSubObject = rDomElement.firstChildElement(HMF_COMPONENTTAG);
        while (!xmlSubObject.isNull())
        {
            GUIModelObject* pObj = loadGUIModelObject(xmlSubObject, mpMainWindow->mpLibrary, this, NOUNDO);

            //Load parameter values
            QDomElement xmlParameter = xmlSubObject.firstChildElement(HMF_PARAMETERTAG);
            while (!xmlParameter.isNull())
            {
                loadParameterValue(xmlParameter, pObj, NOUNDO);
                xmlParameter = xmlParameter.nextSiblingElement(HMF_PARAMETERTAG);
            }

            //Load start values

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }

        //2. Load all sub-systems
        xmlSubObject = rDomElement.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadSubsystemGUIObject(xmlSubObject, mpMainWindow->mpLibrary, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //3. Load all systemports
        xmlSubObject = rDomElement.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadGUIModelObject(xmlSubObject, mpMainWindow->mpLibrary, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMPORTTAG);
        }

        //4. Load all connectors
        xmlSubObject = rDomElement.firstChildElement(HMF_CONNECTORTAG);
        while (!xmlSubObject.isNull())
        {
            loadConnector(xmlSubObject, this, NOUNDO);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_CONNECTORTAG);
        }

        //5. Load all text widgets
        xmlSubObject = rDomElement.firstChildElement(HMF_TEXTWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextWidget(xmlSubObject, this);



//            qreal x, y;
//            parseDomValueNode2(guiData.firstChildElement("pose"), x, y);
//            this->addTextWidget(QPoint(x,y));
//            mTextWidgetList.last()->setText(guiData.firstChildElement("text").text());
//            QFont tempFont;
//            tempFont.fromString(guiData.firstChildElement("font").text());
//            qDebug() << "Font = " << tempFont.toString();
//            mTextWidgetList.last()->setTextFont(tempFont);
//            mTextWidgetList.last()->setTextColor(QColor(guiData.firstChildElement("fontcolor").text()));
//            mTextWidgetList.last()->setPos(QPoint(x,y));

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTWIDGETTAG);
        }
    }
    else
    {
        //Load external system
        //! @todo do this code
    }


}

void GUISystem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mModelFileInfo.filePath().isEmpty())
    {
        loadFromHMF();
    }
    else
    {
        return;
    }
}



//! @brief Temporary addSubSystem functin should be same later on
//! Adds a new component to the GraphicsView.
//! @param componentType is a string defining the type of component.
//! @param position is the position where the component will be created.
//! @param name will be the name of the component.
//! @returns a pointer to the created and added object
//! @todo only modelobjects for now
GUIModelObject* GUISystem::addGUIObject(AppearanceData* pAppearanceData, QPoint position, qreal rotation, selectionStatus startSelected, undoStatus undoSettings)
{
        //Deselect all other components and connectors
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();

    QString componentTypeName = pAppearanceData->getTypeName();
    qDebug()  << "Add GUIObject, typename: " << componentTypeName << " displayname: " << pAppearanceData->getName() << " systemname: " << this->getName();
    if (componentTypeName == "Subsystem")
    {
        mpTempGUIObject= new GUISystem(position, rotation, pAppearanceData, this, startSelected, mGfxType);
    }
    else if (componentTypeName == "SystemPort")
    {
        qDebug() << "======================================================Loading systemport";
        mpTempGUIObject = new GUISystemPort(pAppearanceData, position, rotation, this, startSelected, mGfxType);
    }
    else //Assume some standard component type
    {
        mpTempGUIObject = new GUIComponent(pAppearanceData, position, rotation, this, startSelected, mGfxType);
    }

    mpScene->addItem(mpTempGUIObject);

    emit checkMessages();

    if ( mGUIModelObjectMap.contains(mpTempGUIObject->getName()) )
    {
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Trying to add component with name: " + mpTempGUIObject->getName() + " that already exist in GUIObjectMap, (Not adding)");
        //! @todo Is this check really necessary? Two objects cannot have the same name anyway...
    }
    else
    {
        mGUIModelObjectMap.insert(mpTempGUIObject->getName(), mpTempGUIObject);
    }

    if(undoSettings == UNDO)
    {
        mUndoStack->registerAddedObject(mpTempGUIObject);
    }

    //this->setFocus();

    return mpTempGUIObject;
}



void GUISystem::addTextWidget(QPoint position)
{
    GUITextWidget *tempTextWidget;
    tempTextWidget = new GUITextWidget("Text", position, 0, DESELECTED, this);
    mTextWidgetList.append(tempTextWidget);
}


//! Delete GUIObject with specified name
//! @param objectName is the name of the componenet to delete
void GUISystem::deleteGUIModelObject(QString objectName, undoStatus undoSettings)
{
    qDebug() << "deleteGUIModelObject(): " << objectName << " in: " << this->getName() << " coresysname: " << this->mpCoreSystemAccess->getRootSystemName() ;
    GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(objectName);
    GUIModelObject* obj_ptr = it.value();

    QList<GUIConnector *> pConnectorList = obj_ptr->getGUIConnectorPtrs();
    for(size_t i=0; i<pConnectorList.size(); ++i)
    {
        this->removeConnector(pConnectorList[i], undoSettings);
    }

    if (undoSettings == UNDO)
    {
        //Register removal of connector in undo stack (must be done after removal of connectors or the order of the commands in the undo stack will be wrong!)
        this->mUndoStack->registerDeletedObject(it.value());
        emit componentChanged();
    }

    if (it != mGUIModelObjectMap.end())
    {
        //qDebug() << "Höns från Korea";
        mGUIModelObjectMap.erase(it);
        mSelectedGUIObjectsList.removeOne(obj_ptr);
        mpScene->removeItem(obj_ptr);
        delete(obj_ptr);
        emit checkMessages();
    }
    else
    {
        //qDebug() << "In delete GUIObject: could not find object with name " << objectName;
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpMessageWidget->printGUIErrorMessage("Error: Could not delete object with name " + objectName + ", object not found");
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! This function is used to rename a SubGUIObject
void GUISystem::renameGUIObject(QString oldName, QString newName, undoStatus undoSettings)
{
    //Avoid work if no change is requested
    if (oldName != newName)
    {
        QString modNewName;
            //First find record with old name
        GUIModelObjectMapT::iterator it = mGUIModelObjectMap.find(oldName);
        if (it != mGUIModelObjectMap.end())
        {
                //Make a backup copy
            GUIModelObject* obj_ptr = it.value();
                //Erase old record
            mGUIModelObjectMap.erase(it);
                //Set new name, first in core then in gui object
            qDebug() << "Renaming: " << oldName << " " << newName << " type: " << obj_ptr->type();
            switch (obj_ptr->type())
            {
            case GUICOMPONENT:
                //qDebug() << "GUICOMPONENT";
            case GUISYSTEM :
                //qDebug() << "GUISYSTEM";
                modNewName = this->mpCoreSystemAccess->renameSubComponent(oldName, newName);
                break;
            case GUISYSTEMPORT :
                //qDebug() << "GUISYSTEMPORT";
                modNewName = this->mpCoreSystemAccess->renameSystemPort(oldName, newName);
                break;
            //default :
                //qDebug() << "default";
                    //No Core rename action
            }
            qDebug() << "modNewName: " << modNewName;
            obj_ptr->setDisplayName(modNewName);
                //Re insert
            mGUIModelObjectMap.insert(obj_ptr->getName(), obj_ptr);
        }
        else
        {
            //qDebug() << "Old name: " << oldName << " not found";
            //! @todo Maybe we should give the user a message?
        }

        if (undoSettings == UNDO)
        {
            mUndoStack->newPost();
            mUndoStack->registerRenameObject(oldName, modNewName);
            emit componentChanged();
        }
    }
    emit checkMessages();
}


//! Tells whether or not a component with specified name exist in the GraphicsView
bool GUISystem::haveGUIObject(QString name)
{
    return (mGUIModelObjectMap.count(name) > 0);
}


//! Returns a pointer to the component with specified name.
GUIModelObject *GUISystem::getGUIModelObject(QString name)
{
    if(!mGUIModelObjectMap.contains(name))
    {
        qDebug() << "Request for pointer to non-existing component" << endl;
        assert(false);
    }
    return mGUIModelObjectMap.find(name).value();
}


//! @brief Find a connector in the connector vector
GUIConnector* GUISystem::findConnector(QString startComp, QString startPort, QString endComp, QString endPort)
{
    GUIConnector *item;
    for(int i = 0; i < mSubConnectorList.size(); ++i)
    {
        if((mSubConnectorList[i]->getStartComponentName() == startComp) &&
           (mSubConnectorList[i]->getStartPortName() == startPort) &&
           (mSubConnectorList[i]->getEndComponentName() == endComp) &&
           (mSubConnectorList[i]->getEndPortName() == endPort))
        {
            item = mSubConnectorList[i];
            break;
        }
        //Find even if the caller mixed up start and stop
        else if((mSubConnectorList[i]->getStartComponentName() == endComp) &&
                (mSubConnectorList[i]->getStartPortName() == endPort) &&
                (mSubConnectorList[i]->getEndComponentName() == startComp) &&
                (mSubConnectorList[i]->getEndPortName() == startPort))
        {
            item = mSubConnectorList[i];
            break;
        }
    }
    return item;
}


//! Removes the connector from the model.
//! @param pConnector is a pointer to the connector to remove.
//! @param undoSettings is true if the removal of the connector shall not be registered in the undo stack, for example if this function is called by a redo-function.
void GUISystem::removeConnector(GUIConnector* pConnector, undoStatus undoSettings)
{
    bool doDelete = false;
    bool startPortHasMoreConnections = false;
    bool endPortWasConnected = false;
    bool endPortHasMoreConnections = false;
    int i;

    qDebug() << "Svampar i min diskho";

    if(undoSettings == UNDO)
    {
        mUndoStack->registerDeletedConnector(pConnector);
    }

    for(i = 0; i < mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i] == pConnector)
        {
             //! @todo some error handling both ports must exist and be connected to each other
             if(pConnector->isConnected())
             {
                 GUIPort *pStartP = pConnector->getStartPort();
                 GUIPort *pEndP = pConnector->getEndPort();
                 mpCoreSystemAccess->disconnect(pStartP->getGUIComponentName(), pStartP->getName(), pEndP->getGUIComponentName(), pEndP->getName());
                 emit checkMessages();
                 endPortWasConnected = true;
             }
             doDelete = true;
        }
        else if( (pConnector->getStartPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getStartPort() == mSubConnectorList[i]->getEndPort()) )
        {
            startPortHasMoreConnections = true;
        }
        else if( (pConnector->getEndPort() == mSubConnectorList[i]->getStartPort()) ||
                 (pConnector->getEndPort() == mSubConnectorList[i]->getEndPort()) )
        {
            endPortHasMoreConnections = true;
        }
        if(mSubConnectorList.empty())
        {
            break;
        }
    }

    if(endPortWasConnected && !endPortHasMoreConnections)
    {
        pConnector->getEndPort()->setVisible(!mPortsHidden);
        pConnector->getEndPort()->setIsConnected(false);
    }

    if(!startPortHasMoreConnections)
    {
        pConnector->getStartPort()->setVisible(!mPortsHidden);
        pConnector->getStartPort()->setIsConnected(false);
    }
    else if(startPortHasMoreConnections && !endPortWasConnected)
    {
        pConnector->getStartPort()->setVisible(false);
        pConnector->getStartPort()->setIsConnected(true);
    }

    if(doDelete)
    {
        mSubConnectorList.removeAll(pConnector);
        mSelectedSubConnectorsList.removeAll(pConnector);
        mpScene->removeItem(pConnector);
        delete pConnector;
    }
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


////! @brief A function that adds a system port to the current system
//void GUISystem::addSystemPort()
//{
//    QCursor cursor;
//    QPointF position = mpParentProjectTab->mpGraphicsView->mapToScene(mpParentProjectTab->mpGraphicsView->mapFromGlobal(cursor.pos()));
//    mpParentProjectTab->mpGraphicsView->updateViewPort();
//    //QPoint position = QPoint(2300,2400);
//
//    AppearanceData appearanceData;
//    QTextStream appstream;
//
//    appstream << "TypeName SystemPort";
//    appstream << "QString(ICONPATH) ../../HopsanGUI/systemporttmp.svg";
//    appstream >> appearanceData;
//
//    addGUIObject(appearanceData, position.toPoint());
//}


//! Begins creation of connector or complete creation of connector depending on the mIsCreatingConnector flag.
//! @param pPort is a pointer to the clicked port, either start or end depending on the mIsCreatingConnector flag.
//! @param undoSettings is true if the added connector shall not be registered in the undo stack, for example if this function is called by a redo function.
void GUISystem::createConnector(GUIPort *pPort, undoStatus undoSettings)
{
    qDebug() << "mIsCreatingConnector: " << getIsCreatingConnector();
        //When clicking start port
    if (!getIsCreatingConnector())
    {
        qDebug() << "CreatingConnector in: " << this->getName() << " startPortName: " << pPort->getName();
        //GUIConnectorAppearance *pConnApp = new GUIConnectorAppearance(pPort->getPortType(), mpParentProjectTab->setGfxType);
        mpTempConnector = new GUIConnector(pPort, this);
        emit deselectAllGUIObjects();
        emit deselectAllGUIConnectors();
        setIsCreatingConnector(true);
        mpTempConnector->drawConnector();
    }
        //When clicking end port
    else
    {
        qDebug() << "clicking end port: " << pPort->getName();
        GUIPort *pStartPort = mpTempConnector->getStartPort();

        bool success = mpCoreSystemAccess->connect(pStartPort->getGUIComponentName(), pStartPort->getName(), pPort->getGUIComponentName(), pPort->getName() );
        qDebug() << "GUI Connect: " << success;
        if (success)
        {
            setIsCreatingConnector(false);
            QPointF newPos = pPort->mapToScene(pPort->boundingRect().center());
            mpTempConnector->updateEndPoint(newPos);
            pPort->getGuiModelObject()->rememberConnector(mpTempConnector);
            mpTempConnector->setEndPort(pPort);

                //Hide ports; connected ports shall not be visible
            mpTempConnector->getStartPort()->hide();
            mpTempConnector->getEndPort()->hide();

            mSubConnectorList.append(mpTempConnector);

            mUndoStack->newPost();
            mpParentProjectTab->hasChanged();
            if(undoSettings == UNDO)
            {
                mUndoStack->registerAddedConnector(mpTempConnector);
            }
        }
        emit checkMessages();        
     }
}



//! Copies the selected components, and then deletes them.
//! @see copySelected()
//! @see paste()
void GUISystem::cutSelected()
{
    this->copySelected();
    emit deleteSelected();
    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Puts the selected components in the copy stack, and their positions in the copy position stack.
//! @see cutSelected()
//! @see paste()
void GUISystem::copySelected()
{
    mUndoStack->newPost();

    delete(mpParentProjectTab->mpParentProjectTabWidget->mpCopyData);
    mpParentProjectTab->mpParentProjectTabWidget->mpCopyData = new QString;

    QTextStream copyStream;
    copyStream.setString(mpParentProjectTab->mpParentProjectTabWidget->mpCopyData);

    QList<GUIObject *>::iterator it;
    for(it = mSelectedGUIObjectsList.begin(); it!=mSelectedGUIObjectsList.end(); ++it)
    {
        (*it)->saveToTextStream(copyStream, "COMPONENT");
    }

    for(int i = 0; i != mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->getStartPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->getEndPort()->getGuiModelObject()->isSelected() && mSubConnectorList[i]->isActive())
        {
            mSubConnectorList[i]->saveToTextStream(copyStream, "CONNECT");
        }
    }
}


//! Creates each item in the copy stack, and places it on its respective position in the position copy stack.
//! @see cutSelected()
//! @see copySelected()
void GUISystem::paste()
{
    mUndoStack->newPost();
    mpParentProjectTab->hasChanged();

    QTextStream copyStream;
    copyStream.setString(mpParentProjectTab->mpParentProjectTabWidget->mpCopyData);

        //Deselect all components
    emit deselectAllGUIObjects();

        //Deselect all connectors
    emit deselectAllGUIConnectors();


    QHash<QString, QString> renameMap;       //Used to track name changes, so that connectors will know what components are called
    QString inputWord;

    //! @todo Could we not use some common load function for the stuff bellow

    while ( !copyStream.atEnd() )
    {
        //Extract first word on line
        copyStream >> inputWord;

        if(inputWord == "COMPONENT")
        {
            //QString oldname;
            ModelObjectLoadData data;

            //Read the data from stream
            data.read(copyStream);

            //Remember old name
            //oldname = data.name;

            //Add offset to pos (to avoid pasting on top of old data)
            //! @todo maybe take pos from mouse cursor
            data.posX -= 50;
            data.posY -= 50;

            //Load (create new) object
            GUIObject *pObj = loadGUIModelObject(data,mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->mpLibrary,this, NOUNDO);

            //Remember old name, in case we want to connect later
            renameMap.insert(data.name, pObj->getName());
            //! @todo FINDOUT WHY: Cant select here because then the select all components bellow wont auto select the connectors DONT KNOW WHY, need to figure this out and clean up, (not that I realy nead to set selected here)
            //pObj->setSelected(true);

            mUndoStack->registerAddedObject(pObj);
        }
        else if ( inputWord == "PARAMETER" )
        {
            ParameterLoadData data;
                //Read parameter data
            data.read(copyStream);
                //Replace the component name to the actual new name
            data.componentName = renameMap.find(data.componentName).value();
                //Load it into the new copy
            loadParameterValues(data,this, NOUNDO);
        }
        else if(inputWord == "CONNECT")
        {
            ConnectorLoadData data;
                //Read the data
            data.read(copyStream);
                //Replace component names with posiibly renamed names
            data.startComponentName = renameMap.find(data.startComponentName).value();
            data.endComponentName = renameMap.find(data.endComponentName).value();

                //Apply offset
            //! @todo maybe use mose pointer location
            for (int i=0; i < data.pointVector.size(); ++i)
            {
                data.pointVector[i].rx() -= 50;
                data.pointVector[i].ry() -= 50;
            }

            loadConnector(data, this, NOUNDO);
        }
    }

        //Select all pasted comonents
    QHash<QString, QString>::iterator itn;
    for(itn = renameMap.begin(); itn != renameMap.end(); ++itn)
    {
        mGUIModelObjectMap.find(itn.value()).value()->setSelected(true);
    }

    mpParentProjectTab->mpGraphicsView->updateViewPort();
}


//! Selects all objects and connectors.
void GUISystem::selectAll()
{
    emit selectAllGUIObjects();
    emit selectAllGUIConnectors();
}


//! Deselects all objects and connectors.
void GUISystem::deselectAll()
{
    emit deselectAllGUIObjects();
    emit deselectAllGUIConnectors();
}


//! Hides all component names.
//! @see showNames()
void GUISystem::hideNames()
{
    mIsRenamingObject = false;
    emit deselectAllNameText();
    emit hideAllNameText();
}


//! Shows all component names.
//! @see hideNames()
void GUISystem::showNames()
{
    emit showAllNameText();
}


//! Slot that sets hide ports flag to true or false
void GUISystem::hidePorts(bool doIt)
{
    mPortsHidden = doIt;
}


//! Slot that tells the mUndoStack to execute one undo step. Necessary because the undo stack is not a QT object and cannot use its own slots.
//! @see redo()
//! @see clearUndo()
void GUISystem::undo()
{
    mUndoStack->undoOneStep();
}


//! Slot that tells the mUndoStack to execute one redo step. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see clearUndo()
void GUISystem::redo()
{
    mUndoStack->redoOneStep();
}

//! Slot that tells the mUndoStack to clear itself. Necessary because the redo stack is not a QT object and cannot use its own slots.
//! @see undo()
//! @see redo()
void GUISystem::clearUndo()
{
    mUndoStack->clear();
}


//! Returns true if at least one GUIObject is selected
bool GUISystem::isObjectSelected()
{
    return (mSelectedGUIObjectsList.size() > 0);
}


//! Returns true if at least one GUIConnector is selected
bool GUISystem::isConnectorSelected()
{
    return (mSelectedSubConnectorsList.size() > 0);
}


//! Function that updates start time value of the current project to the one in the simulation setup widget.
//! @see updateTimeStep()
//! @see updateStopTime()
void GUISystem::updateStartTime()
{
    mStartTime = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->getStartTimeFromToolBar();
}


//! Function that updates time step value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateStopTime()
void GUISystem::updateTimeStep()
{
    mTimeStep = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->getTimeStepFromToolBar();
}


//! Function that updates stop time value of the current project to the one in the simulation setup widget.
//! @see updateStartTime()
//! @see updateTimeStep()
void GUISystem::updateStopTime()
{
    mStopTime = mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->getFinishTimeFromToolBar();
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
//! @see setNumberOfSamples(double)
size_t GUISystem::getNumberOfSamples()
{
    return mNumberOfSamples;
}


//! Sets the number of samples value for the current project
//! @see getNumberOfSamples()
void GUISystem::setNumberOfSamples(size_t nSamples)
{
    mNumberOfSamples = nSamples;
}


QString GUISystem::getUserIconPath()
{
    return mUserIconPath;
}

QString GUISystem::getIsoIconPath()
{
    return mIsoIconPath;
}

void GUISystem::setUserIconPath(QString path)
{
    mUserIconPath = path;
}

void GUISystem::setIsoIconPath(QString path)
{
    mIsoIconPath = path;
}


void GUISystem::updateExternalPortPositions()
{
    GUIModelObjectMapT::iterator it;
    QLineF line;
    double angle, x, y, val;

    //Set the initial values to be overwriten by the if bellow
//    double xMax = mpSystem->mGUIObjectMap.begin().value()->x() + mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
//    double xMin = mpSystem->mGUIObjectMap.begin().value()->x() + mpSystem->mGUIObjectMap.begin().value()->rect().width()/2.0;
//    double yMax = mpSystem->mGUIObjectMap.begin().value()->y() + mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;
//    double yMin = mpSystem->mGUIObjectMap.begin().value()->y() + mpSystem->mGUIObjectMap.begin().value()->rect().height()/2.0;

    //! @todo maybe declare these or something like really big number in HopsanGUI to avoid needing to include float.h
    double xMin=FLT_MAX, xMax=FLT_MIN, yMin=FLT_MAX, yMax=FLT_MIN;

    for(it = mGUIModelObjectMap.begin(); it != mGUIModelObjectMap.end(); ++it)
    {
        //val is the center of each component, its coordinates topleft plus half of the size in x and y
        //! @todo maybe we need a function in guiobject that returns this i think it is calculated all over the place
        //check x max and min
        val = it.value()->x()+it.value()->rect().width()/2.0;
        if (val < xMin)
        {
            xMin = val;
        }
        if (val > xMax)
        {
            xMax = val;
        }
        //check y max and min
        val = it.value()->y()+it.value()->rect().height()/2.0;
        if (val < yMin)
        {
            yMin = val;
        }
        if (val > yMax)
        {
            yMax = val;
        }
    }
    //! @todo Find out if it is possible to ask the scene or wive for this information instead of calulating it ourselves
    QPointF center = QPointF((xMax+xMin)/2.0, (yMax+yMin)/2.0);
    double w = xMax-xMin;
    double h = yMax-yMin;

    //rExtPortMap.clear(); //Make sure this is clean
    for(it = mGUIModelObjectMap.begin(); it != mGUIModelObjectMap.end(); ++it)
    {
        if(it.value()->type() == GUISYSTEMPORT)
        {
            //! @todo here we extract center pos again, shuold have function for it
            line = QLineF(center.x(), center.y(), it.value()->x()+it.value()->rect().width()/2, it.value()->y()+it.value()->rect().height()/2);
            //getCurrentTab()->mpGraphicsScene->addLine(line); //debug-grej
            angle = deg2rad(line.angle());
            calcSubsystemPortPosition(w, h, angle, x, y);

            qDebug() << "--------------------x,y unchanged: " << x << " " << y;
            //! @todo what about this coordinate switch should we not use the same coordinate system everywhere
            x = (x/w+1.0)/2.0; //Change coordinate system
            y = (-y/h+1.0)/2.0; //Change coordinate system

            qDebug() << "--------------------x,y changed: " << x << " " << y;

            //modelFile << "PORT " << addQuotes(it.value()->getName()) <<" " << x << " " << y << " " << it.value()->rotation() << "\n";
            //! @todo mybe we should update the ports as we now know where they should be
            GUIPort* pPort = this->getPort(it.value()->getName());
            //! @todo need some kind of function that wraps this somehow "x*mpIcon->sceneBoundingRect().width()"
            //pPort->updatePosition(x*mpIcon->sceneBoundingRect().width(), y*mpIcon->sceneBoundingRect().height());
            pPort->updatePositionByFraction(x,y);
            //pPort->updatePosition(x, y);
            //pPort->setRotation(line.angle());
            //! @todo maybe we should be able to update rotation also

            //Populate the output vector
            //! @todo maybe we can read the updated actual port appearance instead of returning data here
            //rExtPortMap.insert(it.value()->getName(), QPointF(x,y));
        }
    }


}


//! Access function for mIsCreatingConnector
//! @param isConnected is the new value
void GUISystem::setIsCreatingConnector(bool isCreatingConnector)
{
    mIsCreatingConnector = isCreatingConnector;
}


//! Access function for mIsCreatingConnector
bool GUISystem::getIsCreatingConnector()
{
    return mIsCreatingConnector;
}


//! Disables the undo function for the current model
void GUISystem::disableUndo()
{
    if(!mUndoDisabled)
    {
        QMessageBox disableUndoWarningBox(QMessageBox::Warning, tr("Warning"),tr("Disabling undo history will clear all undo history for this model. Do you want to continue?"), 0, 0);
        disableUndoWarningBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
        disableUndoWarningBox.addButton(tr("&No"), QMessageBox::RejectRole);
        disableUndoWarningBox.setWindowIcon(mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->windowIcon());

        if (disableUndoWarningBox.exec() == QMessageBox::AcceptRole)
        {
            this->clearUndo();
            mUndoDisabled = true;
            mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->undoAction->setDisabled(true);
            mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->redoAction->setDisabled(true);
        }
        else
        {
            return;
        }
    }
    else
    {
        mUndoDisabled = false;
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->undoAction->setDisabled(false);
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->redoAction->setDisabled(false);
    }
}


//! Enables or disables the undo buttons depending on whether or not undo is disabled in current tab
void GUISystem::updateUndoStatus()
{
    if(mUndoDisabled)
    {
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->undoAction->setDisabled(true);
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->redoAction->setDisabled(true);
    }
    else
    {
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->undoAction->setDisabled(false);
        mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->redoAction->setDisabled(false);
    }
}


//! Slot that updates the values in the simulation setup widget to display new values when current project tab is changed.
void GUISystem::updateSimulationParametersInToolBar()
{
    mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->setStartTimeInToolBar(mStartTime);
    mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->setTimeStepInToolBar(mTimeStep);
    mpParentProjectTab->mpParentProjectTabWidget->mpParentMainWindow->setFinishTimeInToolBar(mStopTime);
}


//! Sets the iso graphics option for the model
void GUISystem::setGfxType(graphicsType gfxType)
{
    this->mGfxType = gfxType;
    this->mpParentProjectTab->mpGraphicsView->updateViewPort();
    emit setAllGfxType(mGfxType);
}


//! Slot that tells all selected name texts to deselect themselves
void GUISystem::deselectSelectedNameText()
{
    emit deselectAllNameText();
}
