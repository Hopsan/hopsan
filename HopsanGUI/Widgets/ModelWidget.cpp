/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   ModelWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-06-25
//!
//! @brief Contain the class for model widgets
//!
//$Id$

//Qt includes
#include <QGraphicsColorizeEffect>
#include <QSizePolicy>
#include <QHash>
#include <QFileDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>


//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "Utilities/GUIUtilities.h"
#include "InitializationThread.h"
#include "ModelHandler.h"
#include "ProjectTabWidget.h"
#include "SimulationThreadHandler.h"
#include "version_gui.h"
#include "Widgets/AnimationWidget.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/QuickNavigationWidget.h"
#include "SymHop.h"
#include "MessageHandler.h"
#include "LogDataHandler2.h"
#include "CoreAccess.h"
#include "GeneratorUtils.h"

//Needed for Modelica experiments, move later if necessary
#include "ModelicaLibrary.h"
#include "LibraryHandler.h"
#include "GUIPort.h"
#include "PlotWidget2.h"

//! @class ModelWidget
//! @brief The ModelWidget class is a Widget to contain a simulation model
//!
//! ModelWidget contains a drawing space to create models.
//!


//! Constructor.
//! @param parent defines a parent to the new instanced object.
ModelWidget::ModelWidget(ModelHandler *pModelHandler, CentralTabWidget *pParentTabWidget)
    : QWidget(pParentTabWidget)
{
    mpMessageHandler = 0;
    mpAnimationWidget = 0;

    mStartTime.setNum(0.0,'g',10);
    mStopTime.setNum(10.0,'g',10);

    this->setPalette(gpConfig->getPalette());
    this->setMouseTracking(true);

    mpParentModelHandler = pModelHandler;
    mpQuickNavigationWidget = new QuickNavigationWidget(this);

    mpExternalSystemWarningWidget = new QWidget(this);
    QLabel *pExternalSystemLabel = new QLabel("<font color='darkred'>External Subsystem (editing disabled)</font>", mpExternalSystemWarningWidget);
    QFont tempFont = pExternalSystemLabel->font();
    tempFont.setPixelSize(18);
    tempFont.setBold(true);
    pExternalSystemLabel->setFont(tempFont);
    pExternalSystemLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    QPushButton *pOpenExternalSystemButton = new QPushButton("Load As Internal System");
    pOpenExternalSystemButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(pOpenExternalSystemButton, SIGNAL(clicked()), this, SLOT(openCurrentContainerInNewTab()));
    QHBoxLayout *pExternalSystemLayout = new QHBoxLayout();
    pExternalSystemLayout->addWidget(pExternalSystemLabel);
    pExternalSystemLayout->addStretch(1);
    pExternalSystemLayout->addWidget(pOpenExternalSystemButton);
    mpExternalSystemWarningWidget->setLayout(pExternalSystemLayout);
    mpExternalSystemWarningWidget->hide();

    mpLockedSystemWarningWidget = new QLabel("<font color='darkred'>Locked Subsystem (editing disabled)</font>", this);
    mpLockedSystemWarningWidget->hide();

    createOrDestroyToplevelSystem(true);

    mpGraphicsView  = new GraphicsView(this);
    //mpGraphicsView->setScene(mpToplevelSystem->getContainedScenePtr());

    mpLogDataHandler = QSharedPointer<LogDataHandler2>(new LogDataHandler2(this), &QObject::deleteLater);

    mpSimulationThreadHandler = new SimulationThreadHandler();
    setMessageHandler(gpMessageHandler);

    connect(mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);
    connect(this, SIGNAL(simulationFinished()), this, SLOT(unlockSimulateMutex()));
    connect(this, SIGNAL(modelChanged(ModelWidget*)), mpParentModelHandler, SIGNAL(modelChanged(ModelWidget*)));

    emit checkMessages();

    mIsSaved = true;

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
    tabLayout->addWidget(mpExternalSystemWarningWidget,3,0);
    tabLayout->addWidget(mpLockedSystemWarningWidget,3,0);
    //this->setLayout(tabLayout);

    mpGraphicsView->centerView();

    mLastSimulationTime = 0;
}


ModelWidget::~ModelWidget()
{
    // This is a workaround hack to avoid notfying change on unlock caused by removal when tabwidget is destoryed
    // it would report back to the tab in the widget (which is being destoryed) this would cause crash
    mDoNotifyChangeToTabWidget=false;

    setUseRemoteSimulation(false, false);
    delete mpAnimationWidget;
    createOrDestroyToplevelSystem(false);
    mpSimulationThreadHandler->deleteLater();
    mpLogDataHandler->setParent(nullptr);
    mpLogDataHandler.clear();
}

void ModelWidget::setMessageHandler(GUIMessageHandler *pMessageHandler)
{
    // Disconnect old message handler
    if (mpMessageHandler)
    {
        disconnect(this, SIGNAL(checkMessages()), mpMessageHandler, SLOT(collectHopsanCoreMessages()));
    }

    // Assign new message handler
    mpMessageHandler = pMessageHandler;
    mpSimulationThreadHandler->setMessageHandler(pMessageHandler);
    connect(this, SIGNAL(checkMessages()), mpMessageHandler, SLOT(collectHopsanCoreMessages()), Qt::UniqueConnection);
}

void ModelWidget::setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
{
    mStartTime = startTime;
    mStopTime = stopTime;

    // First fix Stop time
    if (mStopTime.toDouble() < mStartTime.toDouble())
    {
        mStopTime = mStartTime;
    }

    //Then fix timestep
    if ( timeStep.toDouble() > (mStopTime.toDouble() - mStartTime.toDouble()) )
    {
        mpToplevelSystem->setTimeStep(mStopTime.toDouble() - mStartTime.toDouble());
    }
    else
    {
        getTopLevelSystemContainer()->setTimeStep(timeStep.toDouble());
    }

    // Notify about any changes made (this will go back up to the main window simtime edit
    emit simulationTimeChanged(getStartTime(), getTimeStep(), getStopTime());

    // Tag model as changed
    this->hasChanged();
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
}

QString ModelWidget::getStartTime()
{
    return mStartTime;
}

QString ModelWidget::getTimeStep()
{
    QString num;
    num.setNum(getTopLevelSystemContainer()->getTimeStep(), 'g', 10);
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
    if (mIsSaved && mDoNotifyChangeToTabWidget)
    {
        QString tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(this));

        if(!tabName.endsWith("*"))
        {
            tabName.append("*");
        }
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(this), tabName);

        mIsSaved = false;
    }
}


//! @brief Returns a pointer to the system in the tab
SystemContainer *ModelWidget::getTopLevelSystemContainer() const
{
    return mpToplevelSystem;
}

//! @brief Returns a pointer to the currently opened container in this model
ContainerObject *ModelWidget::getViewContainerObject()
{
    return mpGraphicsView->getContainerPtr();
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


SimulationThreadHandler *ModelWidget::getSimulationThreadHandler()
{
    return mpSimulationThreadHandler;
}

QSharedPointer<LogDataHandler2> ModelWidget::getLogDataHandler()
{
    return mpLogDataHandler;
}

void ModelWidget::setLogDataHandler(QSharedPointer<LogDataHandler2> pHandler)
{
    mpLogDataHandler->clear();
    mpLogDataHandler = pHandler;
    pHandler->setParentModel(this);
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
}


bool ModelWidget::isEditingFullyDisabled() const
{
    return mFullLockModelEditingCounter > 0;
}

bool ModelWidget::isEditingLimited() const
{
    return mLimitedLockModelEditingCounter > 0;
}

LocklevelEnumT ModelWidget::getCurrentLockLevel() const
{
    if (isEditingFullyDisabled())
    {
        return FullyLocked;
    }
    else if (isEditingLimited())
    {
        return LimitedLock;
    }
    else
    {
        return NotLocked;
    }
}

//! @brief Defines a new alias for specified variable (popup box)
//! @param[in] rFullName The Full name of the variable
bool ModelWidget::defineVariableAlias(const QString &rFullName, const QString &rAlias)
{
    QStringList systems;
    QString c,p,d;
    // Find the subsystem
    SystemContainer *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemContainer*>(pSystem->getModelObject(sysname));
                if (!pSystem)
                {
                    return false;
                }
            }
         }
    }

    // Now ask for alias and try to set it
    if (pSystem)
    {
        if (rAlias.isEmpty())
        {
            bool ok;
            QString alias = QInputDialog::getText(gpMainWindowWidget, gpMainWindowWidget->tr("Define Variable Alias"),
                                                  QString("Alias for: %1").arg(rFullName), QLineEdit::Normal, "", &ok);
            if(ok)
            {
                // Try to set the new alias, abort if it did not work
                return pSystem->setVariableAlias(makeFullVariableName(QStringList(),c,p,d),alias);
            }
        }
        else
        {
            // Try to set the new alias, abort if it did not work
            return pSystem->setVariableAlias(makeFullVariableName(QStringList(),c,p,d),rAlias);
        }
    }
    return false;
}

bool ModelWidget::undefineVariableAlias(const QString &rFullName)
{
    QStringList systems;
    QString c,p,d;
    // Find the subsystem
    SystemContainer *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemContainer*>(pSystem->getModelObject(sysname));
                if (!pSystem)
                {
                    return false;
                }
            }
         }
    }

    // Now ask for alias and try to set it
    if (pSystem)
    {
        // Try to set the new alias, abort if it did not work
        return pSystem->setVariableAlias(makeFullVariableName(QStringList(),c,p,d),"");
    }
    return false;
}

QString ModelWidget::getVariableAlias(const QString &rFullName)
{
    QStringList systems;
    QString c,p,d;
    // Find the subsystem
    SystemContainer *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemContainer*>(pSystem->getModelObject(sysname));
                if (!pSystem)
                {
                    return QString();
                }
            }
         }
    }

    // Now ask for alias and try to set it
    if (pSystem)
    {
        return pSystem->getVariableAlias(makeFullVariableName(QStringList(),c,p,d));
    }
    return QString();
}

void ModelWidget::setUseRemoteSimulation(bool useRemoteCore, bool useAddressServer)
{
#ifdef USEZMQ
    bool useRemoteCoreFailed=false;

    if (useRemoteCore)
    {
        int nThreads = gpConfig->getIntegerSetting(CFG_NUMBEROFTHREADS);
        nThreads = qMax(nThreads, 1);

        QString serveraddress;
        if (useAddressServer)
        {
            SharedRemoteCoreAddressHandlerT pAddressHandler = getSharedRemoteCoreAddressHandler();

            // Check if we should change server address and reconnect
            if (!pAddressHandler.isNull() && (pAddressHandler->getAddressAndPort() != gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS)) )
            {
                pAddressHandler->disconnect();
                pAddressHandler->setHopsanAddressServer(gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESSSERVERADDRESS));
                pAddressHandler->connect();
                //! @todo what happens if it disconnects, then we would need to reconnect, we also need to keep the connection alive by polling
            }

            pAddressHandler->requestAvailableServers();

            serveraddress = pAddressHandler->getBestAvailableServer(nThreads);
        }
        else
        {
            serveraddress = gpConfig->getStringSetting(CFG_REMOTEHOPSANADDRESS);
        }

        if (!serveraddress.isEmpty())
        {
            // If we have found the best available server, then create a local remote simulation handler and connect it to the server
            mpLocalRemoteCoreSimulationHandler = SharedRemoteCoreSimulationHandlerT(new RemoteCoreSimulationHandler());
            mpLocalRemoteCoreSimulationHandler->setUserIdentification(gpConfig->getStringSetting(CFG_REMOTEHOPSANUSERIDENTIFICATION));
            mpLocalRemoteCoreSimulationHandler->setNumThreads(nThreads);
            if (useAddressServer)
            {
                mpLocalRemoteCoreSimulationHandler->setAddressServer(getSharedRemoteCoreAddressHandler()->getAddressAndPort());
            }
            mpLocalRemoteCoreSimulationHandler->setHopsanServer(serveraddress);

            // Connect and attempt to load the model remotely
            bool rc = mpLocalRemoteCoreSimulationHandler->connect();
            if (rc)
            {
                rc = loadModelRemote();
                if (!rc)
                {
                    mpMessageHandler->addErrorMessage(QString("Could not load model in remote server: %1").arg(mpLocalRemoteCoreSimulationHandler->getLastError()));
                    useRemoteCoreFailed = true;
                }
            }
            else
            {
                mpMessageHandler->addErrorMessage(QString("Could not connect to remote server: %1").arg(mpLocalRemoteCoreSimulationHandler->getLastError()));
                useRemoteCoreFailed = true;
            }
        }
        else
        {
            mpMessageHandler->addErrorMessage(QString("Could not find an availible server matching your requirements; nThreads: %1").arg(nThreads));
            useRemoteCoreFailed = true;
        }
    }

    // If use remote core is false and a core simulation handler is set, then delete it
    // This should trigger when we deactivate remote simulation or if connect or load model failed
    if ( (!useRemoteCore || useRemoteCoreFailed) && mpLocalRemoteCoreSimulationHandler)
    {
        mpLocalRemoteCoreSimulationHandler->disconnect(); //! @todo maybe should not disconnect here should wait for destructor when all refs gone, but that never seem to happen
        mpLocalRemoteCoreSimulationHandler.clear();
    }
#endif
}

#ifdef USEZMQ
void ModelWidget::setExternalRemoteCoreSimulationHandler(SharedRemoteCoreSimulationHandlerT pRSCH)
{
    mpExternalRemoteCoreSimulationHandler = pRSCH;
}

double ModelWidget::getSimulationProgress() const
{
    return mSimulationProgress;
}
#endif

bool ModelWidget::getUseRemoteSimulationCore() const
{
#ifdef USEZMQ
    SharedRemoteCoreSimulationHandlerT pRSH = chooseRemoteCoreSimulationHandler();
    return !pRSH.isNull();
#else
    return false;
#endif
}

bool ModelWidget::isRemoteCoreConnected() const
{
#ifdef USEZMQ
    //! @todo should check is connected also
    SharedRemoteCoreSimulationHandlerT pRSH = chooseRemoteCoreSimulationHandler();
    return !pRSH.isNull();
#else
    return false;
#endif
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
        QString tabName = gpCentralTabWidget->tabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()));
        if(tabName.endsWith("*"))
        {
            tabName.chop(1);
        }
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
    }
    mIsSaved = value;
}

bool ModelWidget::simulate_nonblocking()
{
    // Save backup copy (if needed)
    if (!isSaved() && gpConfig->getBoolSetting(CFG_AUTOBACKUP))
    {
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    // Remote Core Simulation
    if (getUseRemoteSimulationCore())
    {
        if (isRemoteCoreConnected())
        {
            // If model contains at least one Modelica component, the special code for simulating models with Modelica components must be used
            foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
            {
                if(comp->getTypeName() == MODELICATYPENAME)
                {
                    gpMessageHandler->addErrorMessage("You cant simulate Modelica models remotly right now");
                    return false;
                }
            }

            if(!mSimulateMutex.tryLock())
            {
                gpMessageHandler->addErrorMessage("mSimulateMutex is locked!! Aborting");
                return false;
            }
#ifdef USEZMQ
            mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
            mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
            mSimulationProgress=0; // Set this to zero here since it may take some time before launched threads will update this value (we do not want the previous value to remain)
            mpSimulationThreadHandler->initSimulateFinalizeRemote(chooseRemoteCoreSimulationHandler(), &mRemoteResultVariables, &mSimulationProgress);
#endif
        }
        else
        {
            mpMessageHandler->addErrorMessage(QString("Remote core is not connected"));
            return false;
        }
    }
    // Local core simulation
    else
    {
        if(!mSimulateMutex.tryLock()) return false;

        //If model contains at least one modelica component, the special code for simulating models with Modelica components must be used
        foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
        {
            if(comp->getTypeName() == MODELICATYPENAME)
            {
                simulateModelica();
                unlockSimulateMutex();
                return true;        //! @todo Should use return value from simulateModelica() function instead
            }
        }

        qDebug() << "Calling simulate_nonblocking()";
        mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
        mpSimulationThreadHandler->initSimulateFinalize(mpToplevelSystem);
    }

    return true;
    //! @todo fix return code
}

bool ModelWidget::simulate_blocking()
{
    // Save backup copy
    if (!isSaved() && gpConfig->getBoolSetting(CFG_AUTOBACKUP))
    {
        //! @todo this should be a help function, also we may not want to call it every time when we run optimization (not sure if that is done now but probably)
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    // Remote Core Simulation
    if (getUseRemoteSimulationCore())
    {
        if (isRemoteCoreConnected())
        {
            // If model contains at least one Modelica component, the special code for simulating models with Modelica components must be used
            foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
            {
                if(comp->getTypeName() == MODELICATYPENAME)
                {
                    gpMessageHandler->addErrorMessage("You cant simulate Modelica models remotly right now");
                    return false;
                }
            }

            if(!mSimulateMutex.tryLock())
            {
                gpMessageHandler->addErrorMessage("mSimulateMutex is locked!! Aborting");
                return false;
            }

#ifdef USEZMQ
            mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
            mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
            mSimulationProgress=0; // Set this to zero here since it may take some time before launched threads will update this value (we do not want the previous value to remain)
            mpSimulationThreadHandler->initSimulateFinalizeRemote(chooseRemoteCoreSimulationHandler(), &mRemoteResultVariables, &mSimulationProgress);
            //! @todo is this really blocking hmm
#endif
        }
        else
        {
            mpMessageHandler->addErrorMessage(QString("Remote core is not connected"));
            return false;
        }
    }
    // Local core simulation
    else
    {
        if(!mSimulateMutex.tryLock()) return false;

        //If model contains at least one Modelica component, the special code for simulating models with Modelica components must be used
        foreach(const ModelObject *comp, mpToplevelSystem->getModelObjects())
        {
            if(comp->getTypeName() == MODELICATYPENAME)
            {
                simulateModelica();
                return true;        //! @todo Should use return value from simulateModelica() function instead
            }
        }

        mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
        mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
        QVector<SystemContainer*> vec;
        vec.push_back(mpToplevelSystem);
        mpSimulationThreadHandler->initSimulateFinalize_blocking(vec);
    }

    return true;
    //! @todo fix return code

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
}

void ModelWidget::importModelParameters(QString parameterFile)
{
    mpToplevelSystem->loadParameterValuesFromFile(parameterFile);
}

void ModelWidget::exportSimulationStates()
{
    QString fileName = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Simulation State to File"), "", tr("Binary file (*.dat)"));
    mpToplevelSystem->getCoreSystemAccessPtr()->saveSimulationState(fileName.toStdString().c_str());
}

//! @todo this should not be in the model widget, it should be in the container
void ModelWidget::handleSystemLock(bool isExternal, bool hasLocalLock)
{
    // If this is an external system and we have not already fully locked editing, then lock it
    if ((isExternal || hasLocalLock) && !isEditingFullyDisabled())
    {
        lockModelEditingFull(true);
    }
    // Else if this is not an external system but we have already locked it, then unlock
    else if (!(isExternal && hasLocalLock) && isEditingFullyDisabled())
    {
        lockModelEditingFull(false);
    }

    mpExternalSystemWarningWidget->setVisible(isExternal);
    mpLockedSystemWarningWidget->setVisible(!isExternal && hasLocalLock);
}


void ModelWidget::lockModelEditingFull(bool lock)
{
    if (lock)
    {
        ++mFullLockModelEditingCounter;
    }
    else
    {
        mFullLockModelEditingCounter = qMax(mFullLockModelEditingCounter-1, 0);
    }
    lockModelEditingLimited(lock);
}

void ModelWidget::lockModelEditingLimited(bool lock)
{
    if (lock)
    {
        ++mLimitedLockModelEditingCounter;
    }
    else
    {
        mLimitedLockModelEditingCounter = qMax(mLimitedLockModelEditingCounter-1, 0);
    }
//#define USEDISABLEGRAYEFFECT
    if( isEditingLimited() )
    {
        QList<ModelObject*> objects =  mpGraphicsView->getContainerPtr()->getModelObjects();
        for (ModelObject* pObj : objects)
        {
            //pObj->setFlag(QGraphicsItem::ItemIsMovable, false);
            //pObj->setFlag(QGraphicsItem::ItemIsSelectable, false);
#ifdef USEDISABLEGRAYEFFECT
            QGraphicsColorizeEffect *pGrayEffect = new QGraphicsColorizeEffect();
            pGrayEffect->setColor(QColor("gray"));
            pObj->setGraphicsEffect(pGrayEffect);

            QList<Connector*> connectors = pObj->getConnectorPtrs();
            for(Connector* pCon : connectors)
            {
                pGrayEffect = new QGraphicsColorizeEffect();
                pGrayEffect->setColor(QColor("gray"));
                pCon->setGraphicsEffect(pGrayEffect);
            }
#endif
        }

#ifdef USEDISABLEGRAYEFFECT
        QList<Widget*> widgets = mpGraphicsView->getContainerPtr()->getWidgets();
        for(Widget* pWidget : widgets)
        {
            QGraphicsColorizeEffect *grayEffect = new QGraphicsColorizeEffect();
            grayEffect->setColor(QColor("gray"));
            pWidget->setGraphicsEffect(grayEffect);
        }
#endif
    }
    else
    {
        QList<ModelObject*> objects =  mpGraphicsView->getContainerPtr()->getModelObjects();
        for (ModelObject* pObj : objects)
        {
            //pObj->setFlag(QGraphicsItem::ItemIsMovable, true);
            //pObj->setFlag(QGraphicsItem::ItemIsSelectable, true);

            if (pObj->graphicsEffect() && !pObj->isDisabled())
            {
                pObj->graphicsEffect()->setEnabled(false);
                //pObj->graphicsEffect()->deleteLater();
                pObj->setGraphicsEffect(nullptr);
            }

            QList<Connector*> connectors = pObj->getConnectorPtrs();
            for(Connector* pCon : connectors)
            {
                if (pCon->graphicsEffect())
                {
                    pCon->graphicsEffect()->setEnabled(false);
                    //pCon->graphicsEffect()->deleteLater();
                    pCon->setGraphicsEffect(nullptr);
                }
            }
        }

        QList<Widget*> widgets = mpGraphicsView->getContainerPtr()->getWidgets();
        for(Widget* pWidget : widgets)
        {
            if (pWidget->graphicsEffect())
            {
                pWidget->graphicsEffect()->setEnabled(false);
                //pWidget->graphicsEffect()->deleteLater();
                pWidget->setGraphicsEffect(nullptr);
            }
        }
    }
}


//! @brief Slot that tells the current system to collect plot data from core
void ModelWidget::collectPlotData(bool overWriteGeneration)
{
    //If we collect plot data, we can plot and calculate losses, so enable these buttons
    //gpMainWindow->mpPlotAction->setEnabled(true);
    //gpMainWindow->mpShowLossesAction->setEnabled(true); //! @todo Can this be done without including main window?
   // gpMainWindow->mpAnimateAction->setEnabled(true);


    if (mRemoteResultVariables.empty())
    {
        // Collect local data
        mpLogDataHandler->collectLogDataFromModel(overWriteGeneration);
    }
    else
    {
        // Collect remote data instead
        mpLogDataHandler->collectLogDataFromRemoteModel(mRemoteResultVariables);
        // Clear now obsolete data
        mRemoteResultVariables.clear();
    }
}

void ModelWidget::setUseRemoteSimulation(bool tf)
{
    setUseRemoteSimulation(tf, gpConfig->getBoolSetting(CFG_USEREMOTEADDRESSSERVER));
}

void ModelWidget::revertModel()
{
    QFileInfo modelfileinfo = mpToplevelSystem->getModelFileInfo();

    // This will remove the current model and create a new empty one
    createOrDestroyToplevelSystem(true);

    // Now reload the model
    QFile modelFile(modelfileinfo.absoluteFilePath());
    if(!modelFile.exists())
    {
        gpMessageHandler->addErrorMessage("File not found: " + modelfileinfo.absoluteFilePath());
    }
    loadModel(modelFile);
    mpGraphicsView->setContainerPtr(mpToplevelSystem);
    emit modelChanged(this);
}


void ModelWidget::openAnimation()
{
    //Generate animation dialog
    if(mpAnimationWidget !=0)
    {
        delete mpAnimationWidget;
        mpAnimationWidget = 0;
    }
    if(!getTopLevelSystemContainer()->getModelObjectNames().isEmpty())   //Animation widget cannot be created with no objects
    {
        mpAnimationWidget = new AnimationWidget(this);
        gpCentralGridLayout->addWidget(mpAnimationWidget, 0, 0, 4, 4);  //! @todo Can this be done without including main window?
        mpAnimationWidget->show();
        gpCentralTabWidget->hide();
    }
}

void ModelWidget::simulateModelica()
{
    QString modelName = mpToplevelSystem->getName();
    if(modelName.isEmpty())
    {
        modelName = "Unsaved";
    }
    QString modelicaCode = "model "+modelName+"\n";

    foreach(ModelObject* object, mpToplevelSystem->getModelObjects())
    {
        if(object->getTypeName() == MODELICATYPENAME)
        {
            QString model = object->getParameterValue("model");
            modelicaCode.append("    "+model+" "+object->getName()+"(");
            //Add type name for modelica component
        }
        else
        {
            modelicaCode.append("    TLM_"+object->getTypeName()+" "+object->getName()+"(");
        }
        foreach(const QString &parName, object->getParameterNames())
        {
            modelicaCode.append(parName+"="+object->getParameterValue(parName)+",");
        }
        modelicaCode.chop(1);
        modelicaCode.append(");\n");
    }

    modelicaCode.append("equation\n");

    QList<Connector*> connectorPtrs;
    foreach(ModelObject* object, mpToplevelSystem->getModelObjects())
    {
        foreach(Connector* connector, object->getConnectorPtrs())
        {
            if(!connectorPtrs.contains(connector))
            {
                connectorPtrs.append(connector);
            }
        }
    }

    foreach(const Connector* connector, connectorPtrs)
    {
        modelicaCode.append("    connect("+connector->getStartComponentName()+"."+connector->getStartPortName()+
                      ","+connector->getEndComponentName()+"."+connector->getEndPortName()+");\n");
    }

    modelicaCode.append("end "+modelName);

    qDebug() << modelicaCode;






    ModelicaModel mainModel(modelicaCode);

    QList<ModelicaVariable> variables = mainModel.getVariables();

    //Add each component to one group each (except TLM components)
    QList<QList<QPair<QString, QString> > > groups;
    foreach(const ModelicaVariable &variable, variables)
    {
        QString type = variable.getType();
        QString name = variable.getName();
        if(type.startsWith("TLM_"))
        {
            variables.removeAll(variable);
        }
        else
        {
            groups.append(QList<QPair<QString, QString> >());
            groups.last().append(QPair<QString, QString>(name, type));
        }
    }

    //Merge all groups of connected components
    QStringList equations;
    mainModel.getEquations(equations);
    foreach(const QString &equation, equations)
    {
        if(equation.startsWith("connect("))
        {
            QString comp1 = equation.section("connect(",1,1).section(".",0,0);
            QString comp2 = equation.section(",",1,1).section(".",0,0).trimmed();
            bool found1 = false;
            bool isTlm1 = false;
            bool found2 = false;
            bool isTlm2 = false;
            foreach(const ModelicaVariable &variable, variables)
            {
                if(variable.getName() == comp1)
                {
                    found1 = true;
                    isTlm1 = variable.getType().startsWith("TLM_");
                }
                else if(variable.getName() == comp2)
                {
                    found2 = true;
                    isTlm2 = variable.getType().startsWith("TLM_");
                }
            }
            if(!found1 || !found2 || isTlm1 || isTlm2)
            {
                continue;       //Ignore connections with TLM components
            }
            int id1, id2;
            for(int i=0; i<groups.size(); ++i)
            {
                for(int j=0; j<groups[i].size(); ++j)
                {
                    if(groups[i][j].first == comp1)
                    {
                        id1 = i;
                    }
                    else if(groups[i][j].first == comp2)
                    {
                        id2 = i;
                    }
                }
            }

            if(id1 != id2)
            {
                groups[qMin(id1,id2)].append(groups[qMax(id1, id2)]);
                groups.removeAt(qMax(id1,id2));
            }
        }
    }

    gpMessageHandler->addInfoMessage("Found "+QString::number(groups.size())+" independent groups of Modelica components.");

    QList<QStringList> groupNames;
    for(int i=0; i<groups.size(); ++i)
    {
        groupNames.append(QStringList());
        for(int j=0; j<groups.at(i).size(); ++j)
        {
            groupNames[i].append(groups.at(i).at(j).first);
        }
    }

    QMap<QString, QList<QStringList> > modelicaModels;
    QStringList loadedLibs;

    gpMessageHandler->addInfoMessage("Compiling Modelica code...");

    for(int i=0; i<groupNames.size(); ++i)
    {
        qDebug() << "Group " << i << ":";
        QStringList flatInitAlgorithms;
        QStringList flatPreAlgorithms;
        QStringList flatEquations;
        QMap<QString,QString> localVars;
        mainModel.toFlatEquations(flatInitAlgorithms, flatPreAlgorithms, flatEquations, localVars, "", groupNames[i]);

        //Ugly hack, because HopsanGenerator assumes port variables to use point notation (e.g. P1.q) while points are replaced with "__" above
        foreach(const QString &var, localVars.keys())
        {
            if(localVars.find(var).value() == "NodeSignalIn")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__y", var+".y");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__y", var+".y");
                }
            }
            else if(localVars.find(var).value() == "NodeHydraulic")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__p", var+".p");
                    flatInitAlgorithms[i].replace(var+"__q", var+".q");
                    flatInitAlgorithms[i].replace(var+"__c", var+".c");
                    flatInitAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatPreAlgorithms.size(); ++i)
                {
                    flatPreAlgorithms[i].replace(var+"__p", var+".p");
                    flatPreAlgorithms[i].replace(var+"__q", var+".q");
                    flatPreAlgorithms[i].replace(var+"__c", var+".c");
                    flatPreAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__p", var+".p");
                    flatEquations[i].replace(var+"__q", var+".q");
                    flatEquations[i].replace(var+"__c", var+".c");
                    flatEquations[i].replace(var+"__Zc", var+".Zc");
                }
            }
            else if(localVars.find(var).value() == "NodeMechanic")
            {
                for(int i=0; i<flatInitAlgorithms.size(); ++i)
                {
                    flatInitAlgorithms[i].replace(var+"__f", var+".f");
                    flatInitAlgorithms[i].replace(var+"__x", var+".x");
                    flatInitAlgorithms[i].replace(var+"__v", var+".v");
                    flatInitAlgorithms[i].replace(var+"__c", var+".c");
                    flatInitAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatPreAlgorithms.size(); ++i)
                {
                    flatPreAlgorithms[i].replace(var+"__f", var+".f");
                    flatPreAlgorithms[i].replace(var+"__x", var+".x");
                    flatPreAlgorithms[i].replace(var+"__v", var+".v");
                    flatPreAlgorithms[i].replace(var+"__c", var+".c");
                    flatPreAlgorithms[i].replace(var+"__Zc", var+".Zc");
                }
                for(int i=0; i<flatEquations.size(); ++i)
                {
                    flatEquations[i].replace(var+"__f", var+".f");
                    flatEquations[i].replace(var+"__x", var+".x");
                    flatEquations[i].replace(var+"__v", var+".v");
                    flatEquations[i].replace(var+"__c", var+".c");
                    flatEquations[i].replace(var+"__Zc", var+".Zc");
                }
            }
        }

        //Generate file name for temporary modelica component
        QString name = "ModelicaTemp";
        int uid = int(rand() / (double)RAND_MAX * 1000000); //! @todo Verify that component with same uid does not exist!
        name.append(QString::number(uid));

        QString flatModel;
        flatModel.append("model "+name+"\n");
        flatModel.append("    annotation(hopsanCqsType = \"Q\");\n\n");
        foreach(const QString &var, localVars.keys())
        {
            flatModel.append("    "+localVars.find(var).value()+" "+var+";\n");
        }
        flatModel.append("initial algorithm\n");
        foreach(const QString &algorithm, flatInitAlgorithms)
        {
            flatModel.append("    "+algorithm+";\n");
        }
        flatModel.append("algorithm\n");
        foreach(const QString &algorithm, flatPreAlgorithms)
        {
            flatModel.append("    "+algorithm+";\n");
        }
        flatModel.append("equation\n");
        foreach(const QString &equation, flatEquations)
        {
            flatModel.append("    "+equation+";\n");
        }
        flatModel.append("end "+name+";\n");

        //Create folder for temporary modelica component
        QString path = gpDesktopHandler->getDataPath()+"ModelicaTempComponents/"+name;
        QDir().mkpath(path);

        QFile moFile(path+"/"+name+".mo");
        moFile.open(QFile::WriteOnly | QFile::Text);
        moFile.write(flatModel.toUtf8());
        moFile.close();

        auto spGenerator = createDefaultImportGenerator();
        if (!spGenerator->generateFromModelica(QFileInfo(moFile).absoluteFilePath(), 0, HopsanGeneratorGUI::CompileT::DoCompile))
        {
            gpMessageHandler->addErrorMessage("Modelica import generator failed");
        }
        qDebug() << flatModel;

        gpLibraryHandler->loadLibrary(path+"/"+name+"._lib.xml");
        if(!gpLibraryHandler->getLoadedTypeNames().contains(name))
        {
            gpMessageHandler->addErrorMessage("Could not load temporary Modelica library. Aborting.");
        }
        loadedLibs.append(name);

        QList<QStringList> connections;
        foreach(const QString &comp, groupNames[i])
        {
            foreach(const Connector *pConnector, mpToplevelSystem->getModelObject(comp)->getConnectorPtrs())
            {
                QString startComp = pConnector->getStartComponentName();
                QString endComp = pConnector->getEndComponentName();
                QString startPort = pConnector->getStartPortName();
                QString endPort = pConnector->getEndPortName();
                QString startType = mpToplevelSystem->getModelObject(startComp)->getTypeName();
                QString endType = mpToplevelSystem->getModelObject(endComp)->getTypeName();

                if(startType != MODELICATYPENAME)
                {
                    connections.append(QStringList());
                    connections.last().append(startComp);
                    connections.last().append(startPort);
                    connections.last().append(name);
                    connections.last().append(startComp+"__"+startPort);
                }
                else if(endType != MODELICATYPENAME)
                {
                    connections.append(QStringList());
                    connections.last().append(name);
                    connections.last().append(endComp+"__"+endPort);
                    connections.last().append(endComp);
                    connections.last().append(endPort);
                }
            }
        }

        modelicaModels.insert(name, connections);
    }

    gpMessageHandler->addInfoMessage("Creating temporary simulation model...");

    //Create new hidden temporary model
    ModelWidget *pTempModel = mpParentModelHandler->addNewModel("TempModel", ModelHandler::Detatched);
    SystemContainer *pTempSys = pTempModel->getTopLevelSystemContainer();

    //Add all non-Modelica components
    for(int i=0; i<mpToplevelSystem->getModelObjectNames().size(); ++i)
    {
        QString name = mpToplevelSystem->getModelObjectNames().at(i);
        QString type = mpToplevelSystem->getModelObjects().at(i)->getTypeName();
        if(type != MODELICATYPENAME)
        {
            ModelObject *pModelObject = pTempSys->addModelObject(type, QPointF(0,0));
            if(!pModelObject)
            {
                gpMessageHandler->addErrorMessage("Failed to add model object of type: "+type+" to temporary model. Aborting.");
                return;
            }
            pTempSys->renameModelObject(pModelObject->getName(), name);

            //Set parameters in added component
            foreach(const QString par, pModelObject->getParameterNames())
            {
                pModelObject->setParameterValue(par, mpToplevelSystem->getModelObject(name)->getParameterValue(par));
            }
        }
    }

    //Add all connections between non-Modelica components
    foreach(const Connector *pConnector, mpToplevelSystem->getSubConnectorPtrs())
    {
        QString startComp = pConnector->getStartComponentName();
        QString endComp = pConnector->getEndComponentName();
        QString startPort = pConnector->getStartPortName();
        QString endPort = pConnector->getEndPortName();

        if(pTempSys->hasModelObject(startComp) && pTempSys->getModelObject(startComp)->getPort(startPort) &&
           pTempSys->hasModelObject(endComp) && pTempSys->getModelObject(endComp)->getPort(endPort))
        {
            pTempSys->createConnector(pTempSys->getModelObject(startComp)->getPort(startPort), pTempSys->getModelObject(endComp)->getPort(endPort));
        }
    }

    //Add and connect the Modelica components
    QMapIterator<QString, QList<QStringList> > it(modelicaModels);
    while(it.hasNext())
    {
        it.next();
        if(!pTempSys->addModelObject(it.key(), QPointF(0,0)))
        {
            gpMessageHandler->addErrorMessage("Failed to add temporary Modelica component. Aborting.");
            return;
        }

        //Make sure all output variable ports are enabled
        //for(int p=0; p<pTempSys->getModelObject(it.key())->getPortListPtrs().size(); ++p)
        QVector<CoreVariameterDescription> variameters;
        pTempSys->getModelObject(it.key())->getVariameterDescriptions(variameters);
        foreach(CoreVariameterDescription variameter, variameters)
        {
            if(variameter.mVariameterType == 1) //1 = output variable
            {
                pTempSys->getModelObject(it.key())->createRefreshExternalPort(variameter.mPortName);
                pTempSys->getModelObject(it.key())->getPort(variameter.mPortName)->setEnable(true);
            }
        }

        foreach(const QStringList &connection, it.value())
        {
            if(connection.size() == 4)      //Should always be 4
            {
                if(!pTempSys->hasModelObject(connection[0]) || !pTempSys->getModelObject(connection[0])->getPort(connection[1]) ||
                   !pTempSys->hasModelObject(connection[2]) || !pTempSys->getModelObject(connection[2])->getPort(connection[3]))
                {
                    gpMessageHandler->addErrorMessage("Failed to create connector, one object or port does not exist. Aborting.");
                    return;
                }
                pTempSys->createConnector(pTempSys->getModelObject(connection[0])->getPort(connection[1]), pTempSys->getModelObject(connection[2])->getPort(connection[3]));
            }
        }


        //Set parameter values for Modelica components
        pTempSys->getModelObject(it.key())->setParameterValue("solverType", "4", true);
        pTempSys->getModelObject(it.key())->setParameterValue("nIter","2", true);
        foreach(const QString parameter, pTempSys->getModelObject(it.key())->getParameterNames())
        {
            if(parameter.contains("__"))
            {
                //! @todo Check that original value exists first!

               QString orgComp = parameter.section("__",0,0);
                QString orgPar = parameter.section("__",1,1);
                if(mpToplevelSystem->hasModelObject(orgComp) &&
                   mpToplevelSystem->getModelObject(orgComp)->hasParameter(orgPar))
                {
                    QString orgValue = mpToplevelSystem->getModelObject(orgComp)->getParameterValue(orgPar);
                    pTempSys->getModelObject(it.key())->setParameterValue(parameter, orgValue);
                }
            }
        }
    }

    pTempModel->setTopLevelSimulationTime(mStartTime, QString::number(this->getTopLevelSystemContainer()->getTimeStep()), mStopTime);
    pTempModel->getTopLevelSystemContainer()->setNumberOfLogSamples(mpToplevelSystem->getNumberOfLogSamples());
    pTempModel->simulate_blocking();

    //Move all data from temporary model to actual model
    mpToplevelSystem->getLogDataHandler()->takeOwnershipOfData(pTempSys->getLogDataHandler().data());

    //Rename variables so that their component name and port names are converted for actual model
    QMapIterator<QString, QList<QStringList> > it2(modelicaModels);
    QStringList varsToRemove;
    while(it2.hasNext())
    {
        it2.next();
        for(int i=0; i<mpToplevelSystem->getLogDataHandler()->getAllVariablesAtGeneration(-1).size(); ++i)
        {
            SharedVectorVariableT pVar = mpToplevelSystem->getLogDataHandler()->getAllVariablesAtGeneration(-1).at(i);
            if(pVar->getSmartName().startsWith(it2.key()+"#"))
            {
                QString newName = pVar->getSmartName();
                newName.remove(it2.key()+"#");
                newName.remove("#Value");
                if(!newName.startsWith("_"))    //Ignore all internal variables (will be removed)
                {
                    SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription());
                    pVarDesc->mComponentName = newName.section("__",0,0);
                    pVarDesc->mPortName = newName.section("__",1,1);
                    pVarDesc->mDataName = newName.section("__",2,2);
                    if(pVarDesc->mDataName.isEmpty())
                        pVarDesc->mDataName = "Value";
                    SharedVectorVariableT pNewVar = createFreeVariable(pVar->getVariableType(), pVarDesc);
                    pNewVar->assignFrom(pVar);
                    mpToplevelSystem->getLogDataHandler()->insertNewVectorVariable(pNewVar);
                }
                varsToRemove.append(pVar->getSmartName());
            }
        }
    }

    gpPlotWidget->updateList();

    //! @todo FIXA /Peter
//    foreach(const QString &var, varsToRemove)
//    {
//        mpToplevelSystem->getLogDataHandler()->deleteVariableContainer(var);
//    }


    //Cleanup
    pTempModel->close();
    delete(pTempModel);

    mpParentModelHandler->setCurrentModel(this);

//    foreach(const QString &lib, loadedLibs)
//    {
//        gpLibraryHandler->unloadLibrary(lib);
//        removeDir(gpDesktopHandler->getDataPath()+"/ModelicaTempComponents/"+lib);
//    }

    //! @todo We should not recompile every time, only if model structure has changed
}


void ModelWidget::closeAnimation()
{
    gpCentralGridLayout->removeWidget(mpAnimationWidget); //! @todo Can this be done without including main window?
    delete mpAnimationWidget;
    mpAnimationWidget = 0;
    gpCentralTabWidget->show();
}


void ModelWidget::unlockSimulateMutex()
{
//    Debug code
//    bool didLock = mSimulateMutex.tryLock();
//    qDebug() << "Simulation mutex locked: " << !didLock;
//    if (didLock)
//    {
//        mSimulateMutex.unlock();
//    }

    // In case we had a relay connection from our parent handlers simulation thread handler, the make sure that we remove it here
    // It must be re-established EVERY time. so that we do not have lingering connections that may trigger this slot accidentally (very bad)
    disconnect(mpParentModelHandler->mpSimulationThreadHandler, SIGNAL(done(bool)), this, SIGNAL(simulationFinished()));
    qDebug() << "unlock simulation mutex";
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
            //mpParentModelHandler->loadModel(pContainer->getModelFileInfo().filePath());
            pContainer->setModelFile("");
            lockModelEditingFull(true);
            mpExternalSystemWarningWidget->setVisible(false);
            break;
        }
    }


}


//! Saves the model and the view port settings in the tab to a model file.
//! @param saveAsFlag tells whether or not an already existing file name shall be used
//! @see saveModel()
//! @see loadModel()
void ModelWidget::saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents)
{
    // Backup old save file before saving (if old file exists)
    if(saveAsFlag == ExistingFile && gpConfig->getBoolSetting(CFG_AUTOBACKUP))
    {
        QFile backupFile(mpToplevelSystem->getModelFileInfo().filePath());
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        QString backupFilePath = gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_save_backup.hmf";
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

        QString modelPath = getTopLevelSystemContainer()->getModelFileInfo().absolutePath();
        if(modelPath.isEmpty())
        {
            modelPath = gpConfig->getStringSetting(CFG_LOADMODELDIR);
        }

        modelFilePathToSave = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                     modelPath,
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
        gpConfig->setStringSetting(CFG_LOADMODELDIR, fileInfo.absolutePath());
    }

    bool success = saveTo(modelFilePathToSave, contents);


    if(success)
    {
            //Set the tab name to the model name, effectively removing *, also mark the tab as saved
        //! @todo this should not happen when saving parameters, This needs to be rewritten in a smarter way so that we do not need a special argument and lots of ifs to do special saving of parameters, actually parameters should be saved using the CLI method (and that code should be in a shared utility library)
        QString tabName = mpToplevelSystem->getModelFileInfo().baseName();
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
        if(contents == FullModel)
        {
            gpConfig->addRecentModel(mpToplevelSystem->getModelFileInfo().filePath());
            this->setSaved(true);
        }

        mpMessageHandler->addInfoMessage("Saved model: " + modelFilePathToSave);

        mpToplevelSystem->getCoreSystemAccessPtr()->addSearchPath(mpToplevelSystem->getModelFileInfo().absolutePath());

        emit modelSaved(this);

        // Reload the model on the remote server when we save
#ifdef USEZMQ
        if (isRemoteCoreConnected())
        {
            loadModelRemote();
        }
#endif
    }
}

//! @brief (Re)Create or Destroy the top-level system
void ModelWidget::createOrDestroyToplevelSystem(bool recreate)
{
    if (recreate)
    {
        // Delete old
        if (mpToplevelSystem)
        {
            createOrDestroyToplevelSystem(false);
        }

        // Create new
        mpToplevelSystem = new SystemContainer(this, 0);
    }
    // Destroy
    else
    {
        // First make sure that we go to the top level system, we don't want to be inside a subsystem while it is being deleted
        mpQuickNavigationWidget->gotoContainerAndCloseSubcontainers(0);

        // Unmake any main window connections
        mpToplevelSystem->unmakeMainWindowConnectionsAndRefresh();
        // Deactivate Undo to prevent each component from registering it being deleted in the undo stack (waste of time)
        mpToplevelSystem->setUndoEnabled(false, true);

        // Now delete the root system, first remove in core (will also trigger delete for all sub modelobjects)
        mpToplevelSystem->deleteInHopsanCore();
        mpToplevelSystem->deleteLater();
    }
}

#ifdef USEZMQ
SharedRemoteCoreSimulationHandlerT ModelWidget::chooseRemoteCoreSimulationHandler() const
{
    // Prefere external remote simulator, if present
    if (mpExternalRemoteCoreSimulationHandler)
    {
        return mpExternalRemoteCoreSimulationHandler;
    }
    // otherwise choose the local one, if present
    else if (mpLocalRemoteCoreSimulationHandler)
    {
        return mpLocalRemoteCoreSimulationHandler;
    }
    else
    {
        return SharedRemoteCoreSimulationHandlerT();
    }
}
#endif

bool ModelWidget::loadModelRemote()
{
#ifdef USEZMQ
    SharedRemoteCoreSimulationHandlerT pRSH = chooseRemoteCoreSimulationHandler();
    if (pRSH)
    {
        QStringList paths = mpToplevelSystem->getCoreSystemAccessPtr()->getSearchPaths();
        QStringList assets = mpToplevelSystem->getCoreSystemAccessPtr()->getModelAssets();
        for (QString &rAsset : assets)
        {
            // Try all paths (for relative assets), and use the one that first matches
            QFileInfo fileInfo(rAsset);
            if (!fileInfo.exists())
            {
                for (QString &path : paths)
                {

                    fileInfo.setFile(path+"/"+rAsset);
                    if (fileInfo.exists())
                    {
                        break;
                    }
                }
            }

            if (fileInfo.exists())
            {
                double dummy;
                mpMessageHandler->addInfoMessage("Sending asset: "+rAsset);
                pRSH->sendAsset(fileInfo.absoluteFilePath(), rAsset, &dummy);
            }
            else
            {
                mpMessageHandler->addErrorMessage("Could not find asset: "+rAsset);
            }
        }

        QDomDocument doc = saveToDom();
        bool rc = pRSH->loadModelStr(doc.toString(-1));
        if (!rc)
        {
            mpMessageHandler->addErrorMessage(QString("Could not load model in remote server: %1").arg(pRSH->getLastError()));
        }


        QVector<QString> types,tags,messages;
        bool rc2 = pRSH->getCoreMessages(types, tags, messages);
        for (int i=0; i<messages.size(); ++i)
        {
            mpMessageHandler->addMessageFromCore(types[i], tags[i], messages[i]);
        }
        return rc && rc2;
    }
#endif
    return false;
}

bool ModelWidget::loadModel(QFile &rModelFile)
{
    QFileInfo modelFileInfo(rModelFile);

    mpToplevelSystem->getCoreSystemAccessPtr()->addSearchPath(modelFileInfo.absoluteDir().absolutePath());
    mpToplevelSystem->setUndoEnabled(false, true);

    // Check if this is an expected hmf xml file
    QDomDocument domDocument;
    QDomElement hmfRoot = loadXMLDomDocument(rModelFile, domDocument, HMF_ROOTTAG);
    if (!hmfRoot.isNull())
    {
        //! @todo check if we could load else give error message and don't attempt to load
        QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);

        // Check if Format version OK
        QString hmfFormatVersion = hmfRoot.attribute(HMF_VERSIONTAG, "0");
        if (!verifyHmfFormatVersion(hmfFormatVersion))
        {
            gpMessageHandler->addErrorMessage("Model file format: "+hmfFormatVersion+", is to old. Try to update (resave) the model in an previous version of Hopsan");
            return false;
        }
        else if (hmfFormatVersion < HMF_VERSIONNUM)
        {
            gpMessageHandler->addWarningMessage("Model file is saved with an older version of Hopsan, but versions should be compatible.");
        }

        mpToplevelSystem->setModelFileInfo(rModelFile); //Remember info about the file from which the data was loaded
        mpToplevelSystem->setAppearanceDataBasePath(modelFileInfo.absolutePath());
        mpToplevelSystem->loadFromDomElement(systemElement);

        // Check for required libraries and show warning if they do not seem to be loaded
        QStringList loadedLibraryNames = gpLibraryHandler->getLoadedLibraryNames();
        //! @todo not hardcoded strings
        QDomElement compLib = hmfRoot.firstChildElement("requirements").firstChildElement("componentlibrary");
        while (!compLib.isNull())
        {
            QString requiredLibID = compLib.firstChildElement("id").text();
            QString requiredLibName = compLib.firstChildElement("name").text();
            auto pRequiredLib = gpLibraryHandler->getLibrary(requiredLibID);

            // Print debug for "old" required data as that data was incorrect
            if (hmfRoot.attribute(HMF_HOPSANGUIVERSIONTAG, "0") < "2.9.0.20180816.1300")
            {
                // Override name due to old xml format
                requiredLibName = compLib.text();
                if (!loadedLibraryNames.contains(requiredLibName))
                {
                    gpMessageHandler->addDebugMessage(QString("If you got load errors, the model '%1' may require library '%2' which does not seem to be loaded")
                                                      .arg(rModelFile.fileName()).arg(requiredLibName));
                }
            }
            else if (pRequiredLib.isNull())
            {
                bool hasRequiredLibName = loadedLibraryNames.contains(requiredLibName);
                if (hasRequiredLibName)
                {
                    gpMessageHandler->addWarningMessage(QString("The model '%1' requires library '%2' with ID '%3'. However, a different library with the same "
                                                                "name is currently loaded. (This might be OK if the library has been re-generated or re-imported.")
                                                        .arg(rModelFile.fileName()).arg(requiredLibName).arg(requiredLibID));
                }
                else
                {
                    gpMessageHandler->addWarningMessage(QString("The model '%1' requires library '%2' with id: '%3' which does not seem to be loaded")
                                                        .arg(rModelFile.fileName()).arg(requiredLibName).arg(requiredLibID));
                }
            }

            compLib = compLib.nextSiblingElement("componentlibrary");
        }

        // Upconvert adding self. to parameter names
        if (hmfRoot.attribute(HMF_HOPSANGUIVERSIONTAG, "0") < "2.14.0") {
            prependSelfToParameterExpressions(mpToplevelSystem);
            QString neededChanges = checkPrependSelfToEmbeddedScripts(mpToplevelSystem);
            if (!neededChanges.isEmpty()) {
                mpMessageHandler->addWarningMessage(neededChanges);
                QMessageBox msgBox;
                msgBox.setWindowTitle("Model updates needed");
                msgBox.setText(neededChanges);
                msgBox.exec();
            }
        }


        setSaved(true);
        mpToplevelSystem->setUndoEnabled(true, true);
        return true;
    }
    else
    {
        gpMessageHandler->addErrorMessage(QString("Model does not contain a HMF root tag: ")+HMF_ROOTTAG);
        return false;
    }
}

QDomDocument ModelWidget::saveToDom(SaveContentsEnumT contents)
{
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
        // Save the required external library names
        QStringList requiredLibraries = mpToplevelSystem->getRequiredComponentLibraries();
        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (const auto& libID : requiredLibraries)
        {
            auto pLibrary = gpLibraryHandler->getLibrary(libID);
            if (pLibrary) {
                auto libdom = appendDomElement(reqDom, "componentlibrary");
                appendDomTextNode(libdom, "id", libID);
                appendDomTextNode(libdom, "name", pLibrary->name);
            }
        }
    }

    // Before we save, make sure that view port is updated in current model (as we only update it when entering exiting systems)
    if (getViewContainerObject() && getGraphicsView())
    {
        getViewContainerObject()->setGraphicsViewport(getGraphicsView()->getViewPort());
    }
    // Save the model component hierarchy
    mpToplevelSystem->saveToDomElement(rootElement, contents);

    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line

    return domDocument;
}

//! @brief Save model to file
//! @param[in] path File path
//! @param[in] contents What should be saved
bool ModelWidget::saveTo(const QString& path, SaveContentsEnumT contents)
{
    auto saveFunction = [this, contents]() -> QDomDocument {
        return this->saveToDom(contents);
    };

    return saveXmlFile(path, mpMessageHandler, saveFunction);
}
