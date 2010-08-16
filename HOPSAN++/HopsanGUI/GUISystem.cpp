#include "GUISystem.h"

#include "GUIObject.h"
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

GUISystem::GUISystem(AppearanceData appearanceData, QPoint position, qreal rotation, GraphicsScene *scene, selectionStatus startSelected, graphicsType gfxType, QGraphicsItem *parent)
    : GUIContainerObject(position, rotation, appearanceData, startSelected, gfxType, scene, parent)
{
    //Set default values
    mLoadType = "Empty";
    mModelFilePath = "";

    //Create subsystem in core and get its name
//    QString corename = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.createSubSystem();
//    if ( getName().isEmpty() )
//    {
//        //If the displayname has not been decided then use the name from core
//        mAppearanceData.setName(corename);
//    }
//    else
//    {
//        //Lets rename the core object to the gui name that is set in the txt description file, we take the name that this function returns
//        mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.rename(corename, getName())); //Cant use setName here as thewould call an aditional rename (of someone else)
//    }
    mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.createSubSystem(this->getName()));

    refreshDisplayName(); //Make sure name window is correct size for center positioning

    //! @todo Write some code here maybe!

//    std::cout << "GUISystem: " << mComponentTypeName.toStdString() << std::endl;
}


//!
//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
//! @param [in] renameSettings  Dont use this if you dont know what you are doing
//!
//! @todo This function is almost exactly identical to the one for GUIcomponents need to make sure that we dont dublicate functions like this, maybe this should be directly in GUIObject
//!
//! The desired new name will be sent to the the core component and may be modified. Rename will be called in the graphics view to make sure that the guicomponent map key value is up to date.
//! renameSettings is a somewhat ugly hack, we need to be able to force setName without calling rename in some very special situations, it defaults to false
//!
void GUISystem::setName(QString newName, renameRestrictions renameSettings)
{
    QString oldName = getName();
    //If name same as before do nothing
    if (newName != oldName)
    {
        //Check if we want to avoid trying to rename in the graphics view map
        if (renameSettings == CORERENAMEONLY)
        {
            mAppearanceData.setName(mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.setSystemName(oldName, newName));
            refreshDisplayName();
        }
        else
        {
            //Rename
            mpParentGraphicsView->renameGUIObject(oldName, newName);
        }
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
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.setSystemTypeCQS(this->getName(), typestring.toStdString()); //ehhh this will set the CQS type for the paren system (the root even) we want to set this partiular systems CQS type
}

QString GUISystem::getTypeCQS()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getSystemTypeCQS(this->getName());  //ehhh this will get the CQS type for the paren system (the root even) we want this partiular systems CQS type
}

QVector<QString> GUISystem::getParameterNames()
{
    return mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getParameterNames(this->getName());
}

//void GUISystem::refreshAppearance();

//! @todo Maybe should be somewhere else and be called load subsystem
void GUISystem::loadFromFile(QString modelFileName)
{
    QFile file;
    QFileInfo fileInfo;
    if (modelFileName.isEmpty())
    {
        QDir fileDialog;
        modelFileName = QFileDialog::getOpenFileName(mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget, tr("Choose Subsystem File"),
                                                             fileDialog.currentPath() + QString("/../../Models"),
                                                             tr("Hopsan Model Files (*.hmf)"));
        if (modelFileName.isEmpty())
            return;

        file.setFileName(modelFileName);
        fileInfo.setFile(file);

        for(int t=0; t!=mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->count(); ++t)
        {
            if( (mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == fileInfo.fileName()) or (mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget->tabText(t) == (fileInfo.fileName() + "*")) )
            {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::information(mpParentGraphicsView->mpParentProjectTab->mpParentProjectTabWidget, tr("Error"), tr("Unable to load model. File is already open."));
                return;
            }
        }
    }
    else
    {
         file.setFileName(modelFileName);
         fileInfo.setFile(file);
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file or not a text file: " + modelFileName;
        return;
    }
    QTextStream textStreamFile(&file); //Converts to QTextStream
    mModelFilePath = modelFileName;

    //Set the name
    this->setName(fileInfo.baseName());

    //Now read the file data
    SystemAppearanceLoadData sysappdata;
    HeaderLoadData header;

    header.read(textStreamFile);
    //qDebug() << "Header read";
    //! @todo check so that version OK!
    sysappdata.read(textStreamFile);
    //qDebug() << "Sysapp data read";

    if (!sysappdata.usericon_path.isEmpty())
    {
        mAppearanceData.setIconPathUser(sysappdata.usericon_path);
    }
    if (!sysappdata.isoicon_path.isEmpty())
    {
        mAppearanceData.setIconPathISO(sysappdata.isoicon_path);
    }

    //! @todo reading portappearance should have a common function and be shared with the setappearancedata rad function that reads from caf files
    PortAppearanceMapT* portappmap = &(mAppearanceData.getPortAppearanceMap());
    for (int i=0; i<sysappdata.portnames.size(); ++i)
    {
        PortAppearance portapp;
        portapp.x = sysappdata.port_xpos[i];
        portapp.y = sysappdata.port_ypos[i];
        portapp.rot = sysappdata.port_angle[i];
        if( (portapp.rot == 0) || (portapp.rot == 180) )
        {
            portapp.direction = LEFTRIGHT;
        }
        else
        {
            portapp.direction = TOPBOTTOM;
        }
        //! @todo portdirection in portapperance should have an initial default value to avoid crash if not set when creating connector
        portapp.selectPortIcon("","",""); //!< @todo fix this

        portappmap->insert(sysappdata.portnames[i], portapp);
        qDebug() << sysappdata.portnames[i];
    }
    qDebug() << "Appearance set";

    //Load the contents of the subsystem from the external file
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.loadSystemFromFileCoreOnly(this->getName(), modelFileName);
    qDebug() << "Loaded in core";

    this->refreshAppearance();
    this->createPorts();
    this->refreshDisplayName();
    file.close();
}


int GUISystem::type() const
{
    return Type;
}


void GUISystem::deleteInHopsanCore()
{
    mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.removeSubComponent(this->getName(), true);
}

//! @todo Maybe should try to reduce multiple copys of same functions with other GUIObjects
void GUISystem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
        QMenu menu;

        QAction *groupAction;
        if (!mpParentGraphicsScene->selectedItems().empty())
            groupAction = menu.addAction(tr("Group components"));

        QAction *parameterAction = menu.addAction(tr("Change parameters"));
        //menu.insertSeparator(parameterAction);

        QAction *showNameAction = menu.addAction(tr("Show name"));
        showNameAction->setCheckable(true);
        showNameAction->setChecked(mpNameText->isVisible());

        QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
        if(!mModelFilePath.isEmpty()) loadAction->setDisabled(true);

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
            GUIGroup *pGroup = new GUIGroup(mpParentGraphicsScene->selectedItems(), appdata, mpParentGraphicsScene);
            mpParentGraphicsScene->addItem(pGroup);
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
            loadFromFile();
        }
    }



void GUISystem::openParameterDialog()
{
    ParameterDialog *dialog = new ParameterDialog(this, mpParentGraphicsView);
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
        QString nodeType = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getNodeType(this->getName(), it.key());
        QString portType = mpParentGraphicsView->mpParentProjectTab->mGUIRootSystem.getPortType(this->getName(), it.key());
        it.value().selectPortIcon(getTypeCQS(), portType, nodeType);

        qreal x = it.value().x;
        qreal y = it.value().y;

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

    if (!mModelFilePath.isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    rStream << addQuotes(mLoadType) << " " << addQuotes(getName()) << " " << addQuotes(getTypeCQS()) << " " << addQuotes(mModelFilePath) << " "
            << pos.x() << " " << pos.y() << " " << rotation() << " " << getNameTextPos() << " " << mpNameText->isVisible() << "\n";
}

void GUISystem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(mModelFilePath.isEmpty())
    {
        loadFromFile();
    }
    else
    {
        return;
    }
}
