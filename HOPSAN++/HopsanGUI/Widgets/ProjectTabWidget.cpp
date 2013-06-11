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
//! @file   ProjectTabWidget.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#include <QtGui>
#include <QSizePolicy>
#include <QHash>

#include "ProjectTabWidget.h"
#include "HcomWidget.h"

#include "MainWindow.h"
#include "GraphicsView.h"
#include "InitializationThread.h"
#include "ProgressBarThread.h"
#include "Configuration.h"
#include "Utilities/XMLUtilities.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "GUIObjects/GUIModelObject.h"
#include "Widgets/LibraryWidget.h"
#include "version_gui.h"
#include "GUIConnector.h"
#include "Widgets/HcomWidget.h"
#include "DesktopHandler.h"
#include "Widgets/DebuggerWidget.h"
#include "ModelHandler.h"

#include "SimulationThreadHandler.h"

//! @class ModelWidget
//! @brief The ModelWidget class is a Widget to contain a simulation model
//!
//! ModelWidget contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ModelWidget::ModelWidget(ModelHandler *modelHandler, CentralTabWidget *parent)
    : QWidget(parent)
{
    mStartTime.setNum(0.0,'g',10);
    mStopTime.setNum(10.0,'g',10);

    mpAnimationWidget = 0;

    mEditingEnabled = true;
    this->setPalette(gConfig.getPalette());
    this->setMouseTracking(true);

    mpParentModelHandler = modelHandler;
    mpQuickNavigationWidget = new QuickNavigationWidget(this);

    mpExternalSystemWidget = new QWidget(this);
    QLabel *pExternalSystemLabel = new QLabel("<font color='darkred'>External Subsystem (editing disabled)</font>", mpExternalSystemWidget);
    QFont tempFont = pExternalSystemLabel->font();
    tempFont.setPixelSize(18);
    tempFont.setBold(true);
    pExternalSystemLabel->setFont(tempFont);
    pExternalSystemLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QPushButton *pOpenExternalSystemButton = new QPushButton("Edit in new tab");
    pOpenExternalSystemButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(pOpenExternalSystemButton, SIGNAL(clicked()), this, SLOT(openCurrentContainerInNewTab()));
    QHBoxLayout *pExternalSystemLayout = new QHBoxLayout();
    pExternalSystemLayout->addWidget(pExternalSystemLabel);
    pExternalSystemLayout->addStretch(1);
    pExternalSystemLayout->addWidget(pOpenExternalSystemButton);
    mpExternalSystemWidget->setLayout(pExternalSystemLayout);
    mpExternalSystemWidget->hide();

    mpToplevelSystem = new SystemContainer(this, 0);
    mpSimulationThreadHandler = new SimulationThreadHandler();

    connect(mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));
    connect(this, SIGNAL(simulationFinished()), this, SLOT(unlockSimulateMutex()));
    connect(this, SIGNAL(checkMessages()), mpParentModelHandler, SIGNAL(checkMessages()), Qt::UniqueConnection);
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);
    connect(mpParentModelHandler->mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));

    emit checkMessages();

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpToplevelSystem->getContainedScenePtr());

//#ifdef XMAS
//    QLabel *pBalls = new QLabel(this);
//    QPixmap imageStars;
//    imageStars.load(QString(GRAPHICSPATH) + "balls.png");
//    pBalls->setPixmap(imageStars);
//    pBalls->setAlignment(Qt::AlignRight | Qt::AlignTop);
//    pBalls->setFixedWidth(200);
//    pBalls->setFixedHeight(217);
//    pBalls->setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    //mpCentralGridLayout->addWidget(pStars,0,0,1,1);
//#endif

    //QVBoxLayout *tabLayout = new QVBoxLayout(this);
    QGridLayout *tabLayout = new QGridLayout(this);
    tabLayout->setSpacing(0);
    tabLayout->addWidget(mpQuickNavigationWidget,0,0,1,2);
    tabLayout->addWidget(mpGraphicsView,1,0,2,2);
//#ifdef XMAS
//    tabLayout->addWidget(pBalls, 1,1);
//#endif
    tabLayout->addWidget(mpExternalSystemWidget,3,0);
    //this->setLayout(tabLayout);

    mpGraphicsView->centerView();

    mLastSimulationTime = 0;
}


ModelWidget::~ModelWidget()
{
    //First make sure that we go to the top level system, we dont want to be inside a subsystem while it is beeing deleted
    this->mpQuickNavigationWidget->gotoContainerAndCloseSubcontainers(0);
    //Now delete the root system, first remove in core (will also trigger delete for all sub modelobjects)
    mpToplevelSystem->deleteInHopsanCore();
    mpToplevelSystem->deleteLater();
    mpSimulationThreadHandler->deleteLater();
}

void ModelWidget::setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
{
    mStartTime = startTime;
    mStopTime = stopTime;

    // First fix Stop time
    if (mStopTime.toDouble() < mStartTime.toDouble())
    {
        mStopTime = mStartTime; //This line must come before the mainWindowSet
        gpMainWindow->setStopTimeInToolBar(mStartTime.toDouble());
    }

    //Then fix timestep
    if ( timeStep.toDouble() > (mStopTime.toDouble() - mStartTime.toDouble()) )
    {
        mpToplevelSystem->setTimeStep(mStopTime.toDouble() - mStartTime.toDouble()); //This line must come before the mainWindowSet
        gpMainWindow->setTimeStepInToolBar( mStopTime.toDouble() - mStartTime.toDouble() );
    }
    else
    {
        getTopLevelSystem()->setTimeStep(timeStep.toDouble());
    }

    this->hasChanged();
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
}

//! @brief Help function to update the toolbar simulation time parameters from a tab
void ModelWidget::setToolBarSimulationTimeParametersFromTab()
{
    QString ts;
    ts.setNum(mpToplevelSystem->getTimeStep(),'g',10);
    gpMainWindow->displaySimulationTimeParameters(mStartTime, ts, mStopTime);
}

QString ModelWidget::getStartTime()
{
    return mStartTime;
}

QString ModelWidget::getTimeStep()
{
    QString num;
    num.setNum(getTopLevelSystem()->getTimeStep());
    return num;
}

QString ModelWidget::getStopTime()
{
    return mStopTime;
}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ModelWidget::hasChanged()
{
    //qDebug() << "hasChanged()";
    if (mIsSaved)
    {
        QString tabName = gpMainWindow->mpCentralTabs->tabText(gpMainWindow->mpCentralTabs->indexOf(mpParentModelHandler->getCurrentModel()));

        if(!tabName.endsWith("*"))
        {
            tabName.append("*");
        }
        gpMainWindow->mpCentralTabs->setTabText(gpMainWindow->mpCentralTabs->indexOf(this), tabName);

        mIsSaved = false;
    }
}


//! @brief Returns a pointer to the system in the tab
SystemContainer *ModelWidget::getTopLevelSystem()
{
    return mpToplevelSystem;
}


//! @brief Returns a pointer to the graphics view displayed in the tab
GraphicsView *ModelWidget::getGraphicsView()
{
    return mpGraphicsView;
}


//! @brief Returns a pointer to the quick navigation widget
QuickNavigationWidget *ModelWidget::getQuickNavigationWidget()
{
    return mpQuickNavigationWidget;
}


//! @brief Sets last simulation time (only use this from project tab widget!)
void ModelWidget::setLastSimulationTime(int time)
{
    mLastSimulationTime = time;
}


//! @brief Returns last simulation time for tab
int ModelWidget::getLastSimulationTime()
{
    return mpSimulationThreadHandler->getLastSimulationTime();
   // return mLastSimulationTime;
}


bool ModelWidget::isEditingEnabled()
{
    return mEditingEnabled;
}


//! @brief Returns whether or not the current project is saved
bool ModelWidget::isSaved()
{
    return mIsSaved;
}


//! @brief Set function to tell the tab whether or not it is saved
void ModelWidget::setSaved(bool value)
{
    if(value)
    {
        QString tabName = gpMainWindow->mpCentralTabs->tabText(gpMainWindow->mpCentralTabs->indexOf(mpParentModelHandler->getCurrentModel()));
        if(tabName.endsWith("*"))
        {
            tabName.chop(1);
        }
        gpMainWindow->mpCentralTabs->setTabText(gpMainWindow->mpCentralTabs->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
    }
    mIsSaved = value;
}

//! @note this is experimental code to replace madness simulation code in the future
bool ModelWidget::simulate_nonblocking()
{
    if(!mSimulateMutex.tryLock()) return false;

    qDebug() << "Calling simulate_nonblocking()";
    //QVector<SystemContainer*> vec;
    //vec.push_back(mpSystem);
    //mSimulationHandler.initSimulateFinalize( vec, mStartTime.toDouble(), mStopTime.toDouble(), mpSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->initSimulateFinalize(mpToplevelSystem);

    return true;
    //! @todo fix return code
}

bool ModelWidget::simulate_blocking()
{
    if(!mSimulateMutex.tryLock()) return false;

    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
    QVector<SystemContainer*> vec;
    vec.push_back(mpToplevelSystem);
    mpSimulationThreadHandler->initSimulateFinalize_blocking(vec);

    return true;
    //! @todo fix return code
}

void ModelWidget::startCoSimulation()
{
    CoreSimulationHandler *pHandler = new CoreSimulationHandler();
    pHandler->runCoSimulation(mpToplevelSystem->getCoreSystemAccessPtr());
    delete(pHandler);

    emit checkMessages();
}


//! Slot that saves current project to old file name if it exists.
//! @see saveModel(int index)
void ModelWidget::save()
{
    saveModel(ExistingFile);
}


//! Slot that saves current project to a new model file.
//! @see saveModel(int index)
void ModelWidget::saveAs()
{
    saveModel(NewFile);
}

void ModelWidget::exportModelParameters()
{
    saveModel(NewFile, ParametersOnly);


//    //saveModel(NEWFILE);

//    QDir fileDialogSaveDir;
//    QString modelFilePath;
//    modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
//                                                 gConfig.getLoadModelDir(),
//                                                 tr("Hopsan Parameter File (*.hmf)"));

//    if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
//    {
//        return;
//    }

//    mpSystem->setModelFile(modelFilePath);
//    QFileInfo fileInfo = QFileInfo(modelFilePath);
//    gConfig.setLoadModelDir(fileInfo.absolutePath());

//    QFile file(mpSystem->getModelFileInfo().filePath());   //Create a QFile object
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        return;
//    }

//    //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
//    mpSystem->setName(mpSystem->getModelFileInfo().baseName());

//    //Update the basepath for relative appearance data info
//    mpSystem->setAppearanceDataBasePath(mpSystem->getModelFileInfo().absolutePath());

//    //Save xml document
//    QDomDocument domDocument;
//    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, mpSystem->getCoreSystemAccessPtr()->getHopsanCoreVersion());

//    // Save the required external lib names
//    QVector<QString> extLibNames;
//    CoreParameterData coreParaAccess;
//    //coreParaAccess.getLoadedLibNames(extLibNames);


////    //! @todo need HMF defines for hardcoded strings
////    QDomElement reqDom = appendDomElement(hmfRoot, "requirements");
////    for (int i=0; i<coreParaAccess.size(); ++i)
////    {
////        appendDomTextNode(reqDom, "Parameters", extLibNames[i]);
////    }

////    QDomElement XMLparameters = appendDomElement(hmfRoot, "parameters");
////    for(int i = 0; i < gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.size(); ++i)
////    {
////        QDomElement XMLparameter = appendDomElement(XMLparameters, "parameter");
////        appendDomTextNode(XMLparameter, "componentname", gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mComponentName);
////        appendDomTextNode(XMLparameter, "parametername", gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mParameterName);
////        appendDomValueNode2(XMLparameter, "minmax", gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mMin, gpMainWindow->mpModelHandler->getCurrentTopLevelSystem()->mOptSettings.mParamters.at(i).mMax);
////    }

////    //Save the model component hierarcy
////    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
//    mpSystem->saveToDomElement(hmfRoot);

//    //Save to file
//    const int IndentSize = 4;
//    QFile xmlhmf(mpSystem->getModelFileInfo().filePath());
//    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not save to file: " + mpSystem->getModelFileInfo().filePath());
//        return;
//    }
//    QTextStream out(&xmlhmf);
//    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//    domDocument.save(out, IndentSize);

//    //Close the file
//    xmlhmf.close();

//    //Set the tab name to the model name, efectively removing *, also mark the tab as saved
//    QString tabName = mpSystem->getModelFileInfo().baseName();
//    mpParentCentralTabWidget->setTabText(mpParentCentralTabWidget->currentIndex(), tabName);
//    gConfig.addRecentModel(mpSystem->getModelFileInfo().filePath());
//    gpMainWindow->updateRecentList();
//    this->setSaved(true);

//    gpMainWindow->mpTerminalWidget->mpConsole->printInfoMessage("Saved model: " + tabName);

}


void ModelWidget::setExternalSystem(bool value)
{
    setEditingEnabled(!value);
    mpExternalSystemWidget->setVisible(value);
}


void ModelWidget::setEditingEnabled(bool value)
{
    mEditingEnabled = value;

    if(!mEditingEnabled)
    {
        QStringList objects = mpGraphicsView->getContainerPtr()->getModelObjectNames();
        for(int i=0; i<objects.size(); ++i)
        {
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsMovable, false);
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsSelectable, false);

            QGraphicsColorizeEffect *grayEffect = new QGraphicsColorizeEffect();
            grayEffect->setColor(QColor("gray"));
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setGraphicsEffect(grayEffect);

            QList<Connector*> connectors = mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->getConnectorPtrs();
            for(int j=0; j<connectors.size(); ++j)
            {
                QGraphicsColorizeEffect *grayEffect2 = new QGraphicsColorizeEffect();
                grayEffect2->setColor(QColor("gray"));
                connectors.at(j)->setGraphicsEffect(grayEffect2);
            }
        }

        QList<Widget*> widgetList = mpGraphicsView->getContainerPtr()->getWidgets();
        for(int w=0; w<widgetList.size(); ++w)
        {
            QGraphicsColorizeEffect *grayEffect = new QGraphicsColorizeEffect();
            grayEffect->setColor(QColor("gray"));
            widgetList.at(w)->setGraphicsEffect(grayEffect);
        }
    }
    else
    {
        QStringList objects = mpGraphicsView->getContainerPtr()->getModelObjectNames();
        for(int i=0; i<objects.size(); ++i)
        {
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsMovable, true);
            mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->setFlag(QGraphicsItem::ItemIsSelectable, true);

            if(mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->graphicsEffect())
                mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->graphicsEffect()->setEnabled(false);

            QList<Connector*> connectors = mpGraphicsView->getContainerPtr()->getModelObject(objects.at(i))->getConnectorPtrs();
            for(int j=0; j<connectors.size(); ++j)
            {
                if(connectors.at(j)->graphicsEffect())
                    connectors.at(j)->graphicsEffect()->setEnabled(false);
            }
        }

        QList<Widget*> widgetList = mpGraphicsView->getContainerPtr()->getWidgets();
        for(int w=0; w<widgetList.size(); ++w)
        {
            if(widgetList.at(w)->graphicsEffect())
                widgetList.at(w)->graphicsEffect()->setEnabled(false);
        }
    }
}


//! @brief Slot that tells the current system to collect plot data from core
void ModelWidget::collectPlotData()
{
    //If we collect plot data, we can plot and calculate losses, so enable these buttons
    gpMainWindow->mpPlotAction->setEnabled(true);
    gpMainWindow->mpShowLossesAction->setEnabled(true);
   // gpMainWindow->mpAnimateAction->setEnabled(true);

    // Tell container to do the job
    mpToplevelSystem->collectPlotData();
}


void ModelWidget::openAnimation()
{
    //Generate animation dialog
    if(mpAnimationWidget !=0)
    {
        delete mpAnimationWidget;
        mpAnimationWidget = 0;
    }
    if(!getTopLevelSystem()->getModelObjectNames().isEmpty())   //Animation widget cannot be created with no objects
    {
        mpAnimationWidget = new AnimationWidget(gpMainWindow);
        gpMainWindow->mpCentralGridLayout->addWidget(mpAnimationWidget, 0, 0, 4, 4);
        mpAnimationWidget->show();
        gpMainWindow->mpCentralTabs->hide();
    }
}

void ModelWidget::lockTab(bool locked)
{
    setDisabled(locked);
}


void ModelWidget::closeAnimation()
{
    gpMainWindow->mpCentralGridLayout->removeWidget(mpAnimationWidget);
    delete mpAnimationWidget;
    mpAnimationWidget = 0;
    gpMainWindow->mpCentralTabs->show();
}


void ModelWidget::unlockSimulateMutex()
{
    mSimulateMutex.unlock();
}


//! @brief Opens current container in new tab
//! Used for opening external subsystems for editing. If current container is not external, it will
//! iterate through parent containers until it finds one that is.
void ModelWidget::openCurrentContainerInNewTab()
{
    ContainerObject *pContainer = mpGraphicsView->getContainerPtr();

    while(true)
    {
        if(pContainer == mpToplevelSystem)
        {
            break;
        }
        else if(!pContainer->isExternal())
        {
            pContainer = pContainer->getParentContainerObject();
        }
        else
        {
            mpParentModelHandler->loadModel(pContainer->getModelFileInfo().filePath());
            break;
        }
    }
}


//! Saves the model and the viewport settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveModel()
//! @see loadModel()
void ModelWidget::saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents)
{
    // Backup old save file before saving (if old file exists)
    if(saveAsFlag == ExistingFile)
    {
        QFile backupFile(mpToplevelSystem->getModelFileInfo().filePath());
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        QString backupFilePath = gDesktopHandler.getBackupPath() + fileNameWithoutHmf + "_backup.hmf";
        if(QFile::exists(backupFilePath))
        {
            QFile::remove(backupFilePath);
        }
        backupFile.copy(backupFilePath);
    }

    //Get file name in case this is a save as operation
    QString modelFilePathToSave = mpToplevelSystem->getModelFileInfo().filePath();
    if((modelFilePathToSave.isEmpty()) || (saveAsFlag == NewFile))
    {
        QString filter;
        if(contents==FullModel)
        {
            filter = tr("Hopsan Model Files (*.hmf)");
        }
        else if(contents==ParametersOnly)
        {
            filter = tr("Hopsan Parameter Files (*.hpf)");
        }


        modelFilePathToSave = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     gConfig.getLoadModelDir(),
                                                     filter);

        if(modelFilePathToSave.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        if(contents==FullModel)
        {
            mpToplevelSystem->setModelFile(modelFilePathToSave);
        }
        QFileInfo fileInfo = QFileInfo(modelFilePathToSave);
        gConfig.setLoadModelDir(fileInfo.absolutePath());
    }

    QFile file(modelFilePathToSave);   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
        return;
    }

    if(contents==FullModel)
    {
            //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
        mpToplevelSystem->setName(mpToplevelSystem->getModelFileInfo().baseName());

            //Update the basepath for relative appearance data info
        mpToplevelSystem->setAppearanceDataBasePath(mpToplevelSystem->getModelFileInfo().absolutePath());
    }

        //Save xml document
    QDomDocument domDocument;
    QDomElement rootElement;
    if(contents==FullModel)
    {
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
    }
    else
    {
        rootElement = domDocument.createElement(HPF_ROOTTAG);
        domDocument.appendChild(rootElement);
    }

    if(contents==FullModel)
    {
            // Save the required external lib names
        QVector<QString> extLibNames;
        CoreLibraryAccess coreLibAccess;
        coreLibAccess.getLoadedLibNames(extLibNames);


        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (int i=0; i<extLibNames.size(); ++i)
        {
            appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
        }
    }
        //Save the model component hierarcy
    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
    mpToplevelSystem->saveToDomElement(rootElement, contents);

        //Save to file
    QFile xmlFile(modelFilePathToSave);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        gpMainWindow->mpTerminalWidget->mpConsole->printErrorMessage("Could not save to file: " + modelFilePathToSave);
        return;
    }
    QTextStream out(&xmlFile);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, XMLINDENTATION);

    //Close the file
    xmlFile.close();

        //Set the tab name to the model name, efectively removing *, also mark the tab as saved
    //! @todo this should not happen when saving parameters, This needs to be rewritten in a smarter way so that we do not need a special argument and lots of ifs to do special saving of parameters, actually parameters should be saved using the CLI method (and that code should be in a shared utility library)
    QString tabName = mpToplevelSystem->getModelFileInfo().baseName();
    gpMainWindow->mpCentralTabs->setTabText(gpMainWindow->mpCentralTabs->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
    if(contents == FullModel)
    {
        gConfig.addRecentModel(mpToplevelSystem->getModelFileInfo().filePath());
        gpMainWindow->updateRecentList();
        this->setSaved(true);
    }

    gpMainWindow->mpTerminalWidget->mpConsole->printInfoMessage("Saved model: " + modelFilePathToSave);

    mpToplevelSystem->getCoreSystemAccessPtr()->addSearchPath(mpToplevelSystem->getModelFileInfo().absolutePath());
}



//! @class CentralTabWidget
//! @brief The CentralTabWidget class is the central tab widget in the main window

//! Constructor.
//! @param parent defines a parent to the new instanced object.
CentralTabWidget::CentralTabWidget(MainWindow *pParentMainWindow)
        :   QTabWidget(pParentMainWindow)
{
    this->setPalette(gConfig.getPalette());

    connect(this, SIGNAL(currentChanged(int)),  pParentMainWindow,                      SLOT(updateToolBarsToNewTab()), Qt::UniqueConnection);
    connect(this, SIGNAL(currentChanged(int)),  pParentMainWindow,                      SLOT(refreshUndoWidgetList()), Qt::UniqueConnection);

    setTabsClosable(true);

    connect(this,   SIGNAL(currentChanged(int)),    gpMainWindow->mpModelHandler, SLOT(modelChanged()),         Qt::UniqueConnection);
    connect(this,   SIGNAL(tabCloseRequested(int)), gpMainWindow->mpModelHandler, SLOT(closeModelByTabIndex(int)), Qt::UniqueConnection);
    connect(this,   SIGNAL(tabCloseRequested(int)), gpMainWindow->mpModelHandler, SLOT(modelChanged()),         Qt::UniqueConnection);

    //this->hide();
}

void CentralTabWidget::setTabNotClosable(int index)
{
    tabBar()->tabButton(index, QTabBar::RightSide)->resize(0, 0);
}

void CentralTabWidget::tabInserted(int index)
{
    tabBar()->setVisible(this->count() > 1);
    QTabWidget::tabInserted(index);
}

void CentralTabWidget::tabRemoved(int index)
{
    tabBar()->setVisible(this->count() > 1);
    QTabWidget::tabRemoved(index);
}



//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.

