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
#include "MessageWidget.h"

#include "MainWindow.h"
#include "GraphicsView.h"
#include "InitializationThread.h"
#include "SimulationThread.h"
#include "ProgressBarThread.h"
#include "Configuration.h"
#include "Utilities/XMLUtilities.h"
#include "GUIObjects/GUISystem.h"
#include "Widgets/LibraryWidget.h"
#include "version_gui.h"
#include "GUIConnector.h"

#include "SimulationHandler.h"

//! @class ProjectTab
//! @brief The ProjectTab class is a Widget to contain a simulation model
//!
//! ProjectTab contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTab::ProjectTab(ProjectTabWidget *parent)
    : QWidget(parent), mpSimulationHandler(0)
{
    mStartTime.setNum(0.0,'g',10);
    mStopTime.setNum(10.0,'g',10);

    mpAnimationWidget = 0;

    mEditingEnabled = true;
    this->setPalette(gConfig.getPalette());
    this->setMouseTracking(true);

    mpParentProjectTabWidget = parent;
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

    mpSystem = new SystemContainer(this, 0);

    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()), Qt::UniqueConnection);
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);
    connect(mpParentProjectTabWidget, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);

    emit checkMessages();

    mIsSaved = true;

    mpGraphicsView  = new GraphicsView(this);
    mpGraphicsView->setScene(mpSystem->getContainedScenePtr());

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


ProjectTab::~ProjectTab()
{
    //qDebug() << "projectTab destructor";
    //First make sure that we go to the top level system, we dont want to be inside a subsystem while it is beeing deleted
    this->mpQuickNavigationWidget->gotoContainerAndCloseSubcontainers(0);
    //Now delete the root system, all subcontents will be automatically removed by the mpSystem destructor
    delete mpSystem;
}

void ProjectTab::setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
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
        mpSystem->setTimeStep(mStopTime.toDouble() - mStartTime.toDouble()); //This line must come before the mainWindowSet
        gpMainWindow->setTimeStepInToolBar( mStopTime.toDouble() - mStartTime.toDouble() );
    }
    else
    {
        getTopLevelSystem()->setTimeStep(timeStep.toDouble());
    }

    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
}

//! @brief Help function to update the toolbar simulation time parameters from a tab
void ProjectTab::setToolBarSimulationTimeParametersFromTab()
{
    QString ts;
    ts.setNum(mpSystem->getTimeStep(),'g',10);
    gpMainWindow->displaySimulationTimeParameters(mStartTime, ts, mStopTime);
}

QString ProjectTab::getStartTime()
{
    return mStartTime;
}

QString ProjectTab::getTimeStep()
{
    QString num;
    num.setNum(getTopLevelSystem()->getTimeStep());
    return num;
}

QString ProjectTab::getStopTime()
{
    return mStopTime;
}


//! Should be called when a model has changed in some sense,
//! e.g. a component added or a connection has changed.
void ProjectTab::hasChanged()
{
    //qDebug() << "hasChanged()";
    if (mIsSaved)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());

        if(!tabName.endsWith("*"))
        {
            tabName.append("*");
        }
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);

        mIsSaved = false;
    }
}


//! @brief Returns a pointer to the system in the tab
SystemContainer *ProjectTab::getTopLevelSystem()
{
    return mpSystem;
}


//! @brief Returns a pointer to the graphics view displayed in the tab
GraphicsView *ProjectTab::getGraphicsView()
{
    return mpGraphicsView;
}


//! @brief Returns a pointer to the quick navigation widget
QuickNavigationWidget *ProjectTab::getQuickNavigationWidget()
{
    return mpQuickNavigationWidget;
}


//! @brief Sets last simulation time (only use this from project tab widget!)
void ProjectTab::setLastSimulationTime(int time)
{
    mLastSimulationTime = time;
}


//! @brief Returns last simulation time for tab
int ProjectTab::getLastSimulationTime()
{
    return mLastSimulationTime;
}


bool ProjectTab::isEditingEnabled()
{
    return mEditingEnabled;
}


//! @brief Returns whether or not the current project is saved
bool ProjectTab::isSaved()
{
    return mIsSaved;
}


//! @brief Set function to tell the tab whether or not it is saved
void ProjectTab::setSaved(bool value)
{
    if(value)
    {
        QString tabName = mpParentProjectTabWidget->tabText(mpParentProjectTabWidget->currentIndex());
        if(tabName.endsWith("*"))
        {
            tabName.chop(1);
        }
        mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);
    }
    mIsSaved = value;
}


bool ProjectTab::simulate()
{
    //! @todo getnumsamp can be done inside
    mpSimulationHandler = new SimulationHandler(mpSystem, mStartTime.toDouble(), mStopTime.toDouble(), mpSystem->getNumberOfLogSamples());
    connect(mpSimulationHandler, SIGNAL(done(bool)), this, SLOT(simulationCompleted(bool)));
    return true;
    //! @todo fix return code
}

void ProjectTab::simulationCompleted(bool success)
{
    emit simulationFinished();
    delete mpSimulationHandler;
    mpSimulationHandler=0;
}

//! Simulates the model in the tab in a separate thread, the GUI runs a progressbar parallel to the simulation.
//! @deprecated
bool ProjectTab::simulate_old()
{
    MessageWidget *pMessageWidget = gpMainWindow->mpMessageWidget;

    // Setup simulation parameters
    double startTime = mStartTime.toDouble();
    double finishTime = mStopTime.toDouble();
    double dt = finishTime - startTime;
    size_t nSteps = dt/mpSystem->getTimeStep();
    size_t nSamples = mpSystem->getNumberOfLogSamples();

    if(!mpSystem->getCoreSystemAccessPtr()->isSimulationOk())
    {
        emit checkMessages();
        return false;
    }

    qDebug() << "Initializing simulation: " << startTime << nSteps << finishTime;

        //Ask core to initialize simulation
    InitializationThread actualInitialization(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, nSamples, this);
    actualInitialization.start(QThread::HighestPriority);

    ProgressBarThread progressThread(this);
    QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
    if(gConfig.getEnableProgressBar())
    {
        progressBar.setWindowModality(Qt::WindowModal);
        progressBar.setWindowTitle(tr("Simulate!"));
        size_t i=0;
        while (actualInitialization.isRunning())
        {
            progressThread.start();
            progressThread.setPriority(QThread::TimeCriticalPriority);//(QThread::LowestPriority);
            progressThread.wait();
            progressBar.setValue(i++);
            if (progressBar.wasCanceled())
            {
                mpSystem->getCoreSystemAccessPtr()->stop();
            }
        }
        progressBar.setValue(i);
    }

    actualInitialization.wait(); //Make sure actualSimulation does not go out of scope during simulation
    actualInitialization.quit();
    const bool initSuccess = actualInitialization.wasInitSuccessful();

    QTime simTimer;
    if (initSuccess)
    {
        //! @todo we should not start simulation if init was aborted/stoped from inside hopsan core, dont just look at the progress bar button
        //Ask core to execute (and finalize) simulation
        if (!progressBar.wasCanceled())
        {
            if(gConfig.getUseMulticore())
                gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting Multi Threaded Simulation");
            else
                gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting Single Threaded Simulation");

            simTimer.start();
            SimulationThread actualSimulation(mpSystem->getCoreSystemAccessPtr(), startTime, finishTime, this);
            actualSimulation.start();
            actualSimulation.setPriority(QThread::HighestPriority);
            //! @todo TimeCriticalPriority seem to work on dual core, is it a problem on single core machines only?
            //actualSimulation.setPriority(QThread::TimeCriticalPriority); //No bar appears in Windows with this prio

            if(gConfig.getEnableProgressBar())
            {
                progressBar.setLabelText(tr("Running simulation..."));
                progressBar.setCancelButtonText(tr("&Abort simulation"));
                progressBar.setMinimum(0);
                progressBar.setMaximum(nSteps);
                while (actualSimulation.isRunning())
                {
                    progressThread.start();
                    progressThread.setPriority(QThread::LowestPriority);
                    progressThread.wait();
                    progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
                    if (progressBar.wasCanceled())
                    {
                        mpSystem->getCoreSystemAccessPtr()->stop();
                    }
                }
                progressThread.quit();
                progressBar.setValue((size_t)(mpSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
            }

            actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
            actualSimulation.quit();
            //emit checkMessages();
        }
    }

    //! @todo we should be able to see if simulation was actaully completet successfully not only check the progress bar button
    QString timeString;
    timeString.setNum(simTimer.elapsed());
    mLastSimulationTime = simTimer.elapsed();
    if (progressBar.wasCanceled() || !initSuccess)
    {
        pMessageWidget->printGUIInfoMessage(QString(tr("Simulation of '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' was terminated!"))));
    }
    else
    {
        pMessageWidget->printGUIInfoMessage(QString(tr("Simulated '").append(mpSystem->getCoreSystemAccessPtr()->getRootSystemName()).append(tr("' successfully!  Simulation time: ").append(timeString).append(" ms"))));
        emit simulationFinished();
        //this->mpParentProjectTabWidget->mpParentMainWindow->mpPlotWidget->mpVariableList->updateList();
    }
    emit checkMessages();

    return (!progressBar.wasCanceled() && initSuccess);
}


//! Slot that saves current project to old file name if it exists.
//! @see saveProjectTab(int index)
void ProjectTab::save()
{
    saveModel(EXISTINGFILE);
}


//! Slot that saves current project to a new model file.
//! @see saveProjectTab(int index)
void ProjectTab::saveAs()
{
    saveModel(NEWFILE);
}


void ProjectTab::setExternalSystem(bool value)
{
    setEditingEnabled(!value);
    mpExternalSystemWidget->setVisible(value);
}


void ProjectTab::setEditingEnabled(bool value)
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
void ProjectTab::collectPlotData()
{
    //If we collect plot data, we can plot and calculate losses, so enable these buttons
    gpMainWindow->mpPlotAction->setEnabled(true);
    gpMainWindow->mpShowLossesAction->setEnabled(true);
    gpMainWindow->mpAnimateAction->setEnabled(true);

    //Tell container to do the job
    this->mpGraphicsView->getContainerPtr()->collectPlotData();
}


void ProjectTab::openAnimation()
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
        mpParentProjectTabWidget->hide();
    }
}


void ProjectTab::closeAnimation()
{
    gpMainWindow->mpCentralGridLayout->removeWidget(mpAnimationWidget);
    delete mpAnimationWidget;
    mpAnimationWidget = 0;
    mpParentProjectTabWidget->show();
}

//! @brief Opens current container in new tab
//! Used for opening external subsystems for editing. If current container is not external, it will
//! iterate through parent containers until it finds one that is.
void ProjectTab::openCurrentContainerInNewTab()
{
    ContainerObject *pContainer = mpGraphicsView->getContainerPtr();

    while(true)
    {
        if(pContainer == mpSystem)
        {
            break;
        }
        else if(!pContainer->isExternal())
        {
            pContainer = pContainer->getParentContainerObject();
        }
        else
        {
            mpParentProjectTabWidget->loadModel(pContainer->getModelFileInfo().filePath());
            break;
        }
    }
}


//! Saves the model and the viewport settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveProjectTab()
//! @see loadModel()
void ProjectTab::saveModel(saveTarget saveAsFlag)
{
    // Backup old save file before saving (if old file exists)
    if(saveAsFlag == EXISTINGFILE)
    {
        QFile backupFile(mpSystem->getModelFileInfo().filePath());
        QString fileNameWithoutHmf = mpSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        QString backupFilePath = QString(BACKUPPATH) + fileNameWithoutHmf + "_backup.hmf";
        if(QFile::exists(backupFilePath))
        {
            QFile::remove(backupFilePath);
        }
        backupFile.copy(backupFilePath);
    }

    //Get file name in case this is a save as operation
    if((mpSystem->getModelFileInfo().filePath().isEmpty()) || (saveAsFlag == NEWFILE))
    {
        QDir fileDialogSaveDir;
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     gConfig.getLoadModelDir(),
                                                     tr("Hopsan Model Files (*.hmf)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }
        mpSystem->setModelFile(modelFilePath);
        QFileInfo fileInfo = QFileInfo(modelFilePath);
        gConfig.setLoadModelDir(fileInfo.absolutePath());
    }

    QFile file(mpSystem->getModelFileInfo().filePath());   //Create a QFile object
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        return;
    }

        //Sets the model name (must set this name before saving or else systemports wont know the real name of their rootsystem parent)
    mpSystem->setName(mpSystem->getModelFileInfo().baseName());

        //Update the basepath for relative appearance data info
    mpSystem->setAppearanceDataBasePath(mpSystem->getModelFileInfo().absolutePath());

        //Save xml document
    QDomDocument domDocument;
    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, mpSystem->getCoreSystemAccessPtr()->getHopsanCoreVersion());

        // Save the required external lib names
    QVector<QString> extLibNames;
    CoreLibraryAccess coreLibAccess;
    coreLibAccess.getLoadedLibNames(extLibNames);

    //! @todo need HMF defines for hardcoded strings
    QDomElement reqDom = appendDomElement(hmfRoot, "requirements");
    for (int i=0; i<extLibNames.size(); ++i)
    {
        appendDomTextNode(reqDom, "componentlibrary", extLibNames[i]);
    }

        //Save the model component hierarcy
    //! @todo maybe use a saveload object instead of calling save imediately (only load object exist for now), or maybe this is fine
    mpSystem->saveToDomElement(hmfRoot);

        //Save to file
    const int IndentSize = 4;
    QFile xmlhmf(mpSystem->getModelFileInfo().filePath());
    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Could not save to file: " + mpSystem->getModelFileInfo().filePath());
        return;
    }
    QTextStream out(&xmlhmf);
    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
    domDocument.save(out, IndentSize);

    //Close the file
    xmlhmf.close();

        //Set the tab name to the model name, efectively removing *, also mark the tab as saved
    QString tabName = mpSystem->getModelFileInfo().baseName();
    mpParentProjectTabWidget->setTabText(mpParentProjectTabWidget->currentIndex(), tabName);
    gConfig.addRecentModel(mpSystem->getModelFileInfo().filePath());
    gpMainWindow->updateRecentList();
    this->setSaved(true);

    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Saved model: " + tabName);
}



//! @class ProjectTabWidget
//! @brief The ProjectTabWidget class is a container class for ProjectTab class
//!
//! ProjectTabWidget contains ProjectTabWidget widgets.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ProjectTabWidget::ProjectTabWidget(MainWindow *parent)
        :   QTabWidget(parent)
{
    this->setPalette(gConfig.getPalette());

    connect(this, SIGNAL(checkMessages()), gpMainWindow->mpMessageWidget, SLOT(checkMessages()), Qt::UniqueConnection);
    connect(this, SIGNAL(currentChanged(int)), gpMainWindow, SLOT(updateToolBarsToNewTab()), Qt::UniqueConnection);
    connect(this, SIGNAL(currentChanged(int)), gpMainWindow, SLOT(refreshUndoWidgetList()), Qt::UniqueConnection);

    setTabsClosable(true);
    mNumberOfUntitledTabs = 0;

    connect(this,SIGNAL(currentChanged(int)),SLOT(tabChanged()), Qt::UniqueConnection);
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(closeProjectTab(int)), Qt::UniqueConnection);
    connect(this,SIGNAL(tabCloseRequested(int)),SLOT(tabChanged()), Qt::UniqueConnection);

    this->hide();
}


//!  Tells current tab to export itself to PDF. This is needed because a direct connection to current tab would be too complicated.


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getCurrentTab()
{
    return qobject_cast<ProjectTab *>(currentWidget());
}


//! Returns a pointer to the currently active project tab - be sure to check that the number of tabs is not zero before calling this
ProjectTab *ProjectTabWidget::getTab(int index)
{
    return qobject_cast<ProjectTab *>(widget(index));
}


//! @brief Returns a pointer to the current top level system model
//! Be sure to check that the number of tabs is not zero before calling this.
SystemContainer *ProjectTabWidget::getCurrentTopLevelSystem()
{
    return getCurrentTab()->getTopLevelSystem();
}


//! @brief Returns a pointer to the currently open container object in current tab
ContainerObject *ProjectTabWidget::getCurrentContainer()
{
    return getCurrentTab()->getGraphicsView()->getContainerPtr();
}


//! @brief Returns a pointer to the currently open container object in specified tab
ContainerObject *ProjectTabWidget::getContainer(int index)
{
    return getTab(index)->getGraphicsView()->getContainerPtr();
}


//! @brief Returns a pointer to the top level system model at specified tab
//! Be sure to check that the tab exist before calling this.
SystemContainer *ProjectTabWidget::getSystem(int index)
{
    return getTab(index)->getTopLevelSystem();
}


//! @brief Adds an existing ProjectTab object to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addProjectTab(ProjectTab *projectTab, QString tabName)
{
    projectTab->setParent(this);

    addTab(projectTab, tabName);
    setCurrentWidget(projectTab);

    projectTab->setToolBarSimulationTimeParametersFromTab();

    emit newTabAdded();
}


//! @brief Adds a ProjectTab object (a new tab) to itself.
//! @see closeProjectTab(int index)
void ProjectTabWidget::addNewProjectTab(QString tabName)
{
    tabName.append(QString::number(mNumberOfUntitledTabs));

    ProjectTab *newTab = new ProjectTab(this);
    newTab->getTopLevelSystem()->setName(tabName);

    this->addTab(newTab, tabName);
    this->setCurrentWidget(newTab);

    newTab->setToolBarSimulationTimeParametersFromTab();

    newTab->setSaved(true);

    mNumberOfUntitledTabs += 1;
}


//! @brief Closes current project.
//! @param index defines which project to close.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeAllProjectTabs()
bool ProjectTabWidget::closeProjectTab(int index)
{
    if (!(getTab(index)->isSaved()))
    {
        QString modelName;
        modelName = tabText(index);
        modelName.chop(1);
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("The model '").append(modelName).append("'").append(QString(" is not saved.")));
        msgBox.setInformativeText("Do you want to save your changes before closing?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Save:
            // Save was clicked
            getTab(index)->save();
            break;
        case QMessageBox::Discard:
            // Don't Save was clicked
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            return false;
        default:
            // should never be reached
            return false;
        }
    }


    if (getTab(index)->getTopLevelSystem()->getPlotDataPtr()->hasOpenPlotCurves())
    {
        QMessageBox msgBox;
        msgBox.setWindowIcon(gpMainWindow->windowIcon());
        msgBox.setText(QString("All open plot curves from this model will be lost."));
        msgBox.setInformativeText("Are you sure you want to close?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int answer = msgBox.exec();

        switch (answer)
        {
        case QMessageBox::Ok:
            // Ok was clicked
            break;
        case QMessageBox::Cancel:
            // Cancel was clicked
            return false;
        default:
            // should never be reached
            return false;
        }
    }

    //Disconnect signals
    //std::cout << "ProjectTabWidget: " << "Closing project: " << qPrintable(tabText(index)) << std::endl;
    //statusBar->showMessage(QString("Closing project: ").append(tabText(index)));
    disconnect(gpMainWindow->mpResetZoomAction,       SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(resetZoom()));
    disconnect(gpMainWindow->mpZoomInAction,          SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(zoomIn()));
    disconnect(gpMainWindow->mpZoomOutAction,         SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(zoomOut()));
    disconnect(gpMainWindow->mpExportPDFAction,       SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(exportToPDF()));
    disconnect(gpMainWindow->mpCenterViewAction,      SIGNAL(triggered()),    getTab(index)->getGraphicsView(),   SLOT(centerView()));

    disconnect(gpMainWindow,                 SIGNAL(simulateKeyPressed()),    getTab(index),                      SLOT(simulate()));
    disconnect(gpMainWindow->mpSaveAction,            SIGNAL(triggered()),    getTab(index),                      SLOT(save()));
    disconnect(gpMainWindow->mpSaveAsAction,          SIGNAL(triggered()),    getTab(index),                      SLOT(saveAs()));

    getContainer(index)->disconnectMainWindowActions();

    getCurrentContainer()->setUndoEnabled(false, true);  //This is necessary to prevent each component from registering it being deleted in the undo stack

    //Delete project tab, We dont need to call removeTab here, this seems to be handled automatically
    delete widget(index);

    return true;
}


//! @brief Closes all opened projects.
//! @return true if closing went ok. false if the user canceled the operation.
//! @see closeProjectTab(int index)
//! @see saveProjectTab()
bool ProjectTabWidget::closeAllProjectTabs()
{
    gConfig.clearLastSessionModels();

    while(count() > 0)
    {
        setCurrentIndex(count()-1);
        gConfig.addLastSessionModel(getCurrentTopLevelSystem()->getModelFileInfo().filePath());
        if (!closeProjectTab(count()-1))
        {
            return false;
        }
    }
    return true;
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @see loadModel(QString modelFileName)
//! @see Model(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Model File"),
                                                         gConfig.getLoadModelDir(),
                                                         tr("Hopsan Model Files (*.hmf)"));
    if(!modelFileName.isEmpty())
    {
        loadModel(modelFileName);
        QFileInfo fileInfo = QFileInfo(modelFileName);
        gConfig.setLoadModelDir(fileInfo.absolutePath());
    }
}


//! @brief Help function that loads a model from the text in a QAction object.
//! Used to facilitate recent models function.
void ProjectTabWidget::loadModel(QAction *action)
{
    loadModel(action->text());
}


//! @brief Loads a model from a file and opens it in a new project tab.
//! @param modelFileName is the path to the loaded file
//! @see loadModel()
//! @see saveModel(saveTarget saveAsFlag)
void ProjectTabWidget::loadModel(QString modelFileName, bool ignoreAlreadyOpen)
{
    //! @todo maybe  write utility function that opens filel checks existance and sets fileinfo
    QFile file(modelFileName);   //Create a QFile object
    if(!file.exists())
    {
        qDebug() << "File not found: " + file.fileName();
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("File not found: " + file.fileName());
        return;
    }
    QFileInfo fileInfo(file);

    //Make sure file not already open
    if(!ignoreAlreadyOpen)
    {
        for(int t=0; t!=this->count(); ++t)
        {
            if(this->getSystem(t)->getModelFileInfo().filePath() == fileInfo.filePath())
            {
                QMessageBox::information(this, tr("Error"), tr("Unable to load model. File is already open."));
                return;
            }
        }
    }

    gpMainWindow->registerRecentModel(fileInfo);

    this->addProjectTab(new ProjectTab(this), fileInfo.baseName());
    ProjectTab *pCurrentTab = this->getCurrentTab();
    pCurrentTab->getTopLevelSystem()->setUndoEnabled(false, true);

    //Check if this is an expected hmf xml file
    //! @todo maybe write helpfunction that does this directly in system (or container)
    QDomDocument domDocument;
    QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
    if (!hmfRoot.isNull())
    {
        //! @todo check if we could load else give error message and dont attempt to load
        QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
        pCurrentTab->getTopLevelSystem()->setModelFileInfo(file); //Remember info about the file from which the data was loaded
        pCurrentTab->getTopLevelSystem()->setAppearanceDataBasePath(pCurrentTab->getTopLevelSystem()->getModelFileInfo().absolutePath());
        pCurrentTab->getTopLevelSystem()->loadFromDomElement(systemElement);

        //! @todo not hardcoded strings
        //! @todo in the future not only debug message but an actual check that libs are present
        QDomElement reqDom = hmfRoot.firstChildElement("requirements");
        QDomElement compLib = reqDom.firstChildElement("componentlibrary");
        while (!compLib.isNull())
        {
            gpMainWindow->mpMessageWidget->printGUIDebugMessage("This model MIGHT require Lib: " + compLib.text());
            compLib = compLib.nextSiblingElement("componentlibrary");
        }
    }
    else
    {
        //! @todo give some cool error message
    }
    pCurrentTab->setSaved(true);

    pCurrentTab->getTopLevelSystem()->setUndoEnabled(true, true);

    emit newTabAdded();
}


void ProjectTabWidget::tabChanged()
{
    if(count() > 0) { this->show(); }
    else { this->hide(); }

    for(int i=0; i<count(); ++i)
    {
        //If you add a disconnect here, remember to also add it to the close tab function!
        //! @todo  Are these connections such connection that are supposed to be permanent conections? otherwise they should be in the disconnectMainWindowActions function
        disconnect(gpMainWindow->mpResetZoomAction,       SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(resetZoom()));
        disconnect(gpMainWindow->mpZoomInAction,          SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(zoomIn()));
        disconnect(gpMainWindow->mpZoomOutAction,         SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(zoomOut()));
        disconnect(gpMainWindow->mpExportPDFAction,       SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(exportToPDF()));
        disconnect(gpMainWindow->mpCenterViewAction,      SIGNAL(triggered()),        getTab(i)->getGraphicsView(),  SLOT(centerView()));

        getContainer(i)->disconnectMainWindowActions();

        disconnect(gpMainWindow,                    SIGNAL(simulateKeyPressed()),   getTab(i),  SLOT(simulate()));
        disconnect(gpMainWindow->mpSaveAction,      SIGNAL(triggered()),            getTab(i),  SLOT(save()));
        disconnect(gpMainWindow->mpSaveAsAction,    SIGNAL(triggered()),            getTab(i),  SLOT(saveAs()));
    }
    if(this->count() != 0)
    {
        connect(gpMainWindow,                       SIGNAL(simulateKeyPressed()),   getCurrentTab(),        SLOT(simulate()), Qt::UniqueConnection);
        connect(gpMainWindow->mpSaveAction,         SIGNAL(triggered()),            getCurrentTab(),        SLOT(save()), Qt::UniqueConnection);
        connect(gpMainWindow->mpSaveAsAction,       SIGNAL(triggered()),            getCurrentTab(),        SLOT(saveAs()), Qt::UniqueConnection);

        connect(gpMainWindow->mpResetZoomAction,    SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(resetZoom()), Qt::UniqueConnection);
        connect(gpMainWindow->mpZoomInAction,       SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(zoomIn()), Qt::UniqueConnection);
        connect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(zoomOut()), Qt::UniqueConnection);
        connect(gpMainWindow->mpExportPDFAction,    SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(exportToPDF()), Qt::UniqueConnection);
        connect(gpMainWindow->mpCenterViewAction,   SIGNAL(triggered()),        getCurrentTab()->getGraphicsView(),    SLOT(centerView()), Qt::UniqueConnection);

        getCurrentContainer()->connectMainWindowActions();

        getCurrentContainer()->updateMainWindowButtons();
        getCurrentTab()->setToolBarSimulationTimeParametersFromTab();


        if(gpMainWindow->mpLibrary->mGfxType != getCurrentTab()->getTopLevelSystem()->getGfxType())
        {
            gpMainWindow->mpLibrary->setGfxType(getCurrentTab()->getTopLevelSystem()->getGfxType());
        }

        gpMainWindow->mpToggleNamesAction->setChecked(!getCurrentContainer()->areSubComponentNamesHidden());
        gpMainWindow->mpTogglePortsAction->setChecked(!getCurrentContainer()->areSubComponentPortsHidden());
        gpMainWindow->mpShowLossesAction->setChecked(getCurrentContainer()->areLossesVisible());
    }
}

void ProjectTabWidget::saveCurrentModelToWrappedCode()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->saveToWrappedCode();
}


void ProjectTabWidget::createFMUFromCurrentModel()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->createFMUSourceFiles();
}


void ProjectTabWidget::createSimulinkWrapperFromCurrentModel()
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->createSimulinkSourceFiles();
}


void ProjectTabWidget::showLosses(bool show)
{
    qobject_cast<SystemContainer*>(getCurrentContainer())->showLosses(show);
}


bool ProjectTabWidget::simulateAllOpenModels(bool modelsHaveNotChanged)
{
    qDebug() << "simulateAllOpenModels()";

    if(count() > 0)
    {
        SystemContainer *pMainSystem = getCurrentTopLevelSystem();     //All systems will use start time, stop time and time step from this system

        MessageWidget *pMessageWidget = gpMainWindow->mpMessageWidget;

            //Setup simulation parameters
        double startTime = getCurrentTab()->getStartTime().toDouble();
        double finishTime = getCurrentTab()->getStopTime().toDouble();
        double dt = finishTime - startTime;
        size_t nSteps = dt/pMainSystem->getTimeStep();
        size_t nSamples = pMainSystem->getNumberOfLogSamples();

        for(int i=0; i<count(); ++i)
        {
            if(!getSystem(i)->getCoreSystemAccessPtr()->isSimulationOk())
            {
                emit checkMessages();
                return false;
            }
        }

        qDebug() << "Initializing simulation!";

            //Ask core to initialize simulation
        QVector<CoreSystemAccess*> coreAccessVector;
        for(int i=0; i<count(); ++i)
        {
            coreAccessVector.append(getSystem(i)->getCoreSystemAccessPtr());
        }
        MultipleInitializationThread actualInitialization(coreAccessVector, startTime, finishTime, nSamples);
        actualInitialization.start(QThread::HighestPriority);

        ProgressBarThread progressThread(this);
        QProgressDialog progressBar(tr("Initializing simulation..."), tr("&Abort initialization"), 0, 0, this);
        if(gConfig.getEnableProgressBar())
        {
            progressBar.setWindowModality(Qt::WindowModal);
            progressBar.setWindowTitle(tr("Simulate!"));
            size_t i=0;
            while (actualInitialization.isRunning())
            {
                progressThread.start();
                progressThread.setPriority(QThread::TimeCriticalPriority);//(QThread::LowestPriority);
                progressThread.wait();
                progressBar.setValue(i++);
                if (progressBar.wasCanceled())
                {
                    for(int i=0; i<count(); ++i)
                    {
                        getSystem(i)->getCoreSystemAccessPtr()->stop();
                    }
                }
            }
            progressBar.setValue(i);
        }

        actualInitialization.wait(); //Make sure actualSimulation does not go out of scope during simulation
        actualInitialization.quit();
        const bool initSuccess = actualInitialization.wasInitSuccessful();

        qDebug() << "Starting simulation!";

        QTime simTimer;
        if (initSuccess)
        {
            //! @todo we should not start simulation if init was aborted/stoped from inside hopsan core, dont just look at the progress bar button
            //Ask core to execute (and finalize) simulation
            if (!progressBar.wasCanceled())
            {
                if(gConfig.getUseMulticore())
                    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting multi-threaded simulation of all models");
                else
                    gpMainWindow->mpMessageWidget->printGUIInfoMessage("Starting single-threaded simulation of all models");

                simTimer.start();
                MultipleSimulationThread actualSimulation(coreAccessVector, startTime, finishTime, modelsHaveNotChanged, this);
                actualSimulation.start();
                actualSimulation.setPriority(QThread::HighestPriority);

                if(gConfig.getEnableProgressBar())
                {
                    progressBar.setLabelText(tr("Running simulation..."));
                    progressBar.setCancelButtonText(tr("&Abort simulation"));
                    progressBar.setMinimum(0);
                    progressBar.setMaximum(nSteps);
                    while (actualSimulation.isRunning())
                    {
                        progressThread.start();
                        progressThread.setPriority(QThread::LowestPriority);
                        progressThread.wait();
                        progressBar.setValue((size_t)(pMainSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
                        if (progressBar.wasCanceled())
                        {
                            for(int i=0; i<count(); ++i)
                            {
                                getSystem(i)->getCoreSystemAccessPtr()->stop();
                            }
                        }
                    }
                    progressThread.quit();
                    progressBar.setValue((size_t)(pMainSystem->getCoreSystemAccessPtr()->getCurrentTime()/dt * nSteps));
                }

                actualSimulation.wait(); //Make sure actualSimulation do not goes out of scope during simulation
                actualSimulation.quit();
                //emit checkMessages();
            }
        }

        //! @todo we should be able to see if simulation was actaully completet successfully not only check the progress bar button
        QString timeString;
        timeString.setNum(simTimer.elapsed());
        for(int i=0; i<count(); ++i)
        {
            getTab(i)->setLastSimulationTime(simTimer.elapsed());
        }
        if (progressBar.wasCanceled() || !initSuccess)
        {
            pMessageWidget->printGUIInfoMessage(QString(tr("Simulation of all systems was terminated!")));
        }
        else
        {
            pMessageWidget->printGUIInfoMessage(QString(tr("Simulated all systems successfully!  Simulation time: ").append(timeString).append(" ms")));
            emit simulationFinished();
            //this->mpParentProjectTabWidget->mpParentMainWindow->mpPlotWidget->mpVariableList->updateList();
        }
        emit checkMessages();

        return (!progressBar.wasCanceled() && initSuccess);


        //getSystem(i)->getCoreSystemAccessPtr()->simlateAllOpenModels();
    }

    return false;       //No tabs open
}

void ProjectTabWidget::setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime)
{
    if (count() > 0)
    {
        getCurrentTab()->setTopLevelSimulationTime(startTime, timeStep, stopTime);
    }
}


void ProjectTabWidget::openAnimation()
{
    if(count() > 0)
    {
        getCurrentTab()->openAnimation();
    }
}
