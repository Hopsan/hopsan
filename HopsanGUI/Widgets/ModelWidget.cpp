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
#include <QFileInfo>
#include <QDialogButtonBox>


//Hopsan includes
#include "global.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "GraphicsView.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
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
#include "LibraryHandler.h"
#include "UndoStack.h"

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
    connect(mpSimulationThreadHandler, SIGNAL(stepFinished()), this, SLOT(collectAndAppendPlotData()));
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

void ModelWidget::setModelType(ModelWidget::ModelType type)
{
    mModelType = type;
}

ModelWidget::ModelType ModelWidget::getModelType() const
{
    return mModelType;
}

void ModelWidget::setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime, UndoStatusEnumT undoSettings)
{
    if(undoSettings == Undo) {
        mpToplevelSystem->getUndoStackPtr()->newPost();
        mpToplevelSystem->getUndoStackPtr()->registerSimulationTimeChanged(mStartTime, QString::number(mpToplevelSystem->getTimeStep()), mStopTime, startTime, timeStep, stopTime);
    }

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
        mpToplevelSystem->setTimeStep(timeStep.toDouble());
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
SystemObject *ModelWidget::getTopLevelSystemContainer() const
{
    return mpToplevelSystem;
}

//! @brief Returns a pointer to the currently opened container in this model
SystemObject *ModelWidget::getViewContainerObject()
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
    SystemObject *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(sysname));
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
    SystemObject *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(sysname));
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
    SystemObject *pSystem=0;
    if (splitFullVariableName(rFullName,systems,c,p,d))
    {
        if (mpToplevelSystem)
        {
            pSystem = mpToplevelSystem;
            for (auto &sysname : systems)
            {
                pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(sysname));
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
        int nThreads = gpConfig->getIntegerSetting(cfg::numberofthreads);
        nThreads = qMax(nThreads, 1);

        QString serveraddress;
        if (useAddressServer)
        {
            SharedRemoteCoreAddressHandlerT pAddressHandler = getSharedRemoteCoreAddressHandler();

            // Check if we should change server address and reconnect
            if (!pAddressHandler.isNull() && (pAddressHandler->getAddressAndPort() != gpConfig->getStringSetting(cfg::remotehopsanaddresserveraddress)) )
            {
                pAddressHandler->disconnect();
                pAddressHandler->setHopsanAddressServer(gpConfig->getStringSetting(cfg::remotehopsanaddresserveraddress));
                pAddressHandler->connect();
                //! @todo what happens if it disconnects, then we would need to reconnect, we also need to keep the connection alive by polling
            }

            pAddressHandler->requestAvailableServers();

            serveraddress = pAddressHandler->getBestAvailableServer(nThreads);
        }
        else
        {
            serveraddress = gpConfig->getStringSetting(cfg::remotehopsanaddress);
        }

        if (!serveraddress.isEmpty())
        {
            // If we have found the best available server, then create a local remote simulation handler and connect it to the server
            mpLocalRemoteCoreSimulationHandler = SharedRemoteCoreSimulationHandlerT(new RemoteCoreSimulationHandler());
            mpLocalRemoteCoreSimulationHandler->setUserIdentification(gpConfig->getStringSetting(cfg::remotehopsanuseridentification));
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
    this->lockModelEditingFull(true);

    // Save backup copy (if needed)
    if (!isSaved() && gpConfig->getBoolSetting(cfg::autobackup))
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

        qDebug() << "Calling simulate_nonblocking()";
        if(gpConfig->getBoolSetting(cfg::logduringsimulation)) {
            prepareForLogDuringSimulation();
        }
        mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
        mpSimulationThreadHandler->initSimulateFinalize(mpToplevelSystem);
        if(gpConfig->getBoolSetting(cfg::logduringsimulation)) {
            cleanupAfterLogDuringSimulation();
        }
    }

    return true;
    //! @todo fix return code
}

bool ModelWidget::simulate_blocking()
{
    this->lockModelEditingFull(true);

    // Save backup copy
    if (!isSaved() && gpConfig->getBoolSetting(cfg::autobackup))
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

        mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
        mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
        QVector<SystemObject*> vec;
        vec.push_back(mpToplevelSystem);
        mpSimulationThreadHandler->initSimulateFinalize_blocking(vec);
    }

    return true;
    //! @todo fix return code

}

bool ModelWidget::startRealtimeSimulation(const double realtimeFactor)
{
    if(!mSimulateMutex.tryLock()) {
        gpMessageHandler->addErrorMessage("Simulation mutex is locked. Aborting.");
        return false;
    }
    CoreSimulationHandler mCoreSimulationHandler;
    return mCoreSimulationHandler.startRealtimeSimulation(mpToplevelSystem->getCoreSystemAccessPtr(), realtimeFactor);
}

void ModelWidget::stopRealtimeSimulation()
{
    CoreSimulationHandler mCoreSimulationHandler;
    mCoreSimulationHandler.stopRealtimeSimulation(mpToplevelSystem->getCoreSystemAccessPtr());
    mCoreSimulationHandler.finalize(mpToplevelSystem->getCoreSystemAccessPtr());
    mSimulateMutex.unlock();
}

bool ModelWidget::simulateDcpServer()
{
    // Save backup copy
    if (!isSaved() && gpConfig->getBoolSetting(cfg::autobackup))
    {
        //! @todo this should be a help function, also we may not want to call it every time when we run optimization (not sure if that is done now but probably)
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    QDialog */*pDcpServerDialog*/pDialog = new QDialog(gpMainWindowWidget);
    QGridLayout */*pDcpServerDialogLayout*/pLayout = new QGridLayout(pDialog);
    QLineEdit *pHostLineEdit = new QLineEdit("127.0.0.1",pDialog);

    QSpinBox *pPortSpinBox = new QSpinBox(pDialog);
    pPortSpinBox->setMaximum(1000000);
    pPortSpinBox->setValue(8080);
    pPortSpinBox->setSingleStep(1);

    QLineEdit *pCommunicationStepLineEdit = new QLineEdit(getTimeStep(), pDialog);
    pCommunicationStepLineEdit->setValidator(new QDoubleValidator());
    QLineEdit *pTargetFileLineEdit = new QLineEdit(gpDesktopHandler->getDocumentsPath()+"/"+getTopLevelSystemContainer()->getModelFileInfo().baseName()+".dcp");
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDialog);
    QPushButton *pOkButton = pButtonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *pCancelButton = pButtonBox->addButton(QDialogButtonBox::Cancel);
    connect(pOkButton, SIGNAL(clicked()), pDialog, SLOT(accept()));
    connect(pCancelButton, SIGNAL(clicked()), pDialog, SLOT(reject()));
    pLayout->addWidget(new QLabel("Host address:",pDialog),0,0);
    pLayout->addWidget(pHostLineEdit,0,1,1,2);
    pLayout->addWidget(new QLabel("Port:",pDialog),1,0);
    pLayout->addWidget(pPortSpinBox,1,1,1,2);
    pLayout->addWidget(new QLabel("Communication step:", pDialog),2,0);
    pLayout->addWidget(pCommunicationStepLineEdit,2,1,1,2);
    pLayout->addWidget(new QLabel("XML output file:",pDialog),3,0);
    pLayout->addWidget(pTargetFileLineEdit,3,1,1,2);
    pLayout->addWidget(pButtonBox, 4,1,1,3);

    if(pDialog->exec() == QDialog::Rejected) {
        return false;
    }

    if(!mSimulateMutex.tryLock())
    {
        gpMessageHandler->addErrorMessage("Simulation mutex is locked. Aborting.");
        return false;
    }

    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), mpToplevelSystem->getNumberOfLogSamples());
    mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
    mSimulationProgress=0;
    mpSimulationThreadHandler->initSimulateFinalizeDcpServer(mpToplevelSystem, pHostLineEdit->text(), pPortSpinBox->value(), pCommunicationStepLineEdit->text().toDouble(), pTargetFileLineEdit->text());

    return true;
    //! @todo fix return code
}

bool ModelWidget::simulateDcpMaster()
{
    // Save backup copy
    if (!isSaved() && gpConfig->getBoolSetting(cfg::autobackup))
    {
        //! @todo this should be a help function, also we may not want to call it every time when we run optimization (not sure if that is done now but probably)
        QString fileNameWithoutHmf = mpToplevelSystem->getModelFileInfo().fileName();
        fileNameWithoutHmf.chop(4);
        saveTo(gpDesktopHandler->getBackupPath() + fileNameWithoutHmf + "_sim_backup.hmf");
    }

    QDialog *pDcpSettingsDialog = new QDialog(gpMainWindowWidget);
    QGridLayout *pDialogLayout = new QGridLayout(pDcpSettingsDialog);
    QLineEdit *pHostLineEdit = new QLineEdit("127.0.0.1",pDcpSettingsDialog);
    QSpinBox *pPortSpinBox = new QSpinBox(pDcpSettingsDialog);
    pPortSpinBox->setMaximum(1000000);
    pPortSpinBox->setValue(8180);
    pPortSpinBox->setSingleStep(1);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDcpSettingsDialog);
    QPushButton *pOkButton = pButtonBox->addButton(QDialogButtonBox::Ok);
    QPushButton *pCancelButton = pButtonBox->addButton(QDialogButtonBox::Cancel);
    QCheckBox *pRealTimeCheckBox = new QCheckBox("Realtime", pDcpSettingsDialog);
    pRealTimeCheckBox->setChecked(false);
    connect(pOkButton, SIGNAL(clicked()), pDcpSettingsDialog, SLOT(accept()));
    connect(pCancelButton, SIGNAL(clicked()), pDcpSettingsDialog, SLOT(reject()));
    pDialogLayout->addWidget(new QLabel("Host address:",pDcpSettingsDialog),0,0);
    pDialogLayout->addWidget(pHostLineEdit,0,1,1,2);
    pDialogLayout->addWidget(new QLabel("Port:",pDcpSettingsDialog),1,0);
    pDialogLayout->addWidget(pPortSpinBox,1,1,1,2);
    pDialogLayout->addWidget(pRealTimeCheckBox);
    pDialogLayout->addWidget(pButtonBox, 3,1,1,3);

    if(pDcpSettingsDialog->exec() == QDialog::Rejected) {
        return false;
    }

    if(!mSimulateMutex.tryLock())
    {
        gpMessageHandler->addErrorMessage("Simulation mutex is locked. Aborting.");
        return false;
    }

    mpSimulationThreadHandler->setSimulationTimeVariables(mStartTime.toDouble(), mStopTime.toDouble(), mpToplevelSystem->getLogStartTime(), uint(mpToplevelSystem->getNumberOfLogSamples()));
    mpSimulationThreadHandler->setProgressDilaogBehaviour(true, false);
    mSimulationProgress=0;
    mpSimulationThreadHandler->initSimulateFinalizeDcpMaster(mpToplevelSystem, pHostLineEdit->text(), pPortSpinBox->value(), pRealTimeCheckBox->isChecked());

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

void ModelWidget::exportModelParametersToHpf()
{
    saveModel(NewFile, ParametersOnly);
}

void ModelWidget::exportModelParametersToSsv()
{
    QStringList names, values, dataTypes, quantities, units;
    getTopLevelSystemContainer()->getAllParametersAndValuesRecursively("", names, values, dataTypes, quantities, units);

    QString saveDirectory = gpConfig->getStringSetting(cfg::dir::parameterexport);
    QString ssvFile = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Parameters to SSV"),
                                                     saveDirectory,
                                                     tr("System Structure Parameter Values (*.ssv)"));
    if(ssvFile.isEmpty()) {
        return;
    }
    gpConfig->setStringSetting(cfg::dir::parameterexport, QFileInfo(ssvFile).absolutePath());

    //! @todo Don't assume all parameters to be of real type

    QDomDocument ssvDocument;
    appendRootXMLProcessingInstruction(ssvDocument);
    QDomElement ssvParameterSetElement = ssvDocument.createElement(ssv::parameterSet);
    ssvDocument.appendChild(ssvParameterSetElement);
    ssvParameterSetElement.setAttribute(ssv::attr::version, "1.0");
    ssvParameterSetElement.setAttribute(ssv::attr::name, this->getTopLevelSystemContainer()->getName());
    ssvParameterSetElement.setAttribute(ssv::attr::xmlns, ssv::url);
    QDomElement ssvParametersElement = appendDomElement(ssvParameterSetElement, ssv::parameters);
    for(int i=0; i<names.size(); ++i) {
        QDomElement ssvParameterElement = appendDomElement(ssvParametersElement, ssv::parameter);
        ssvParameterElement.setAttribute(ssv::attr::name, names[i]);
        QDomElement ssvRealElement = appendDomElement(ssvParameterElement,ssv::dataTypeTranslator[dataTypes[i]]);
        ssvRealElement.setAttribute(ssv::attr::value, values[i]);
        ssvRealElement.setAttribute(ssv::attr::unit, units[i]);
    }

    QDomElement ssvUnitsElement = appendDomElement(ssvParameterSetElement, ssv::units);
    QList<QPair<QString, QString> > usedUnits;
    for(int i=0; i<units.size(); ++i) {
        QString quantity = quantities[i];
        if(quantity.isEmpty()) {
            QStringList unitQuantities = gpConfig->getQuantitiesForUnit(units[i]);
            if(!unitQuantities.isEmpty()) {
                quantity = unitQuantities.first();  //Picking the first quantity is the best we can do without more knowledge
            }
        }
        if(usedUnits.contains(qMakePair(units[i], quantity)) || quantity.isEmpty()) {
            continue;
        }
        usedUnits.append(qMakePair(units[i], quantity));
        UnitConverter converter;
        gpConfig->getUnitScale(quantity, units[i], converter);
        double scale = converter.scaleToDouble();
        double offset = converter.offsetToDouble();
        QDomElement ssvUnitElement = appendDomElement(ssvUnitsElement, ssv::unit);
        ssvUnitElement.setAttribute(ssv::attr::name, units[i]);

        QDomElement ssvBaseUnitElement = appendDomElement(ssvUnitElement, ssv::baseUnit);
        ssvBaseUnitElement.setAttribute(ssv::attr::factor, scale);
        ssvBaseUnitElement.setAttribute(ssv::attr::offset, offset);

        //! @todo We should maybe derive base units in this way everywhere to conform with e.g. FMI and SSP standards, instead of hard-coding it here
        int kg, m, s, A, K, mol, cd, rad;
        gpConfig->getBaseUnitSIExponents(quantity, kg, m, s, A, K, mol, cd, rad);
        ssvBaseUnitElement.setAttribute(ssv::attr::kg,  kg);
        ssvBaseUnitElement.setAttribute(ssv::attr::m,   m);
        ssvBaseUnitElement.setAttribute(ssv::attr::s,   s);
        ssvBaseUnitElement.setAttribute(ssv::attr::A,   A);
        ssvBaseUnitElement.setAttribute(ssv::attr::K,   K);
        ssvBaseUnitElement.setAttribute(ssv::attr::mol, mol);
        ssvBaseUnitElement.setAttribute(ssv::attr::cd,  cd);
        ssvBaseUnitElement.setAttribute(ssv::attr::rad, rad);
    }

    saveXmlFile(ssvFile, gpMessageHandler, [&](){return ssvDocument;});
}

void ModelWidget::importModelParametersFromHpf(QString parameterFile)
{
    mpToplevelSystem->loadParameterValuesFromFile(parameterFile);
}

void ModelWidget::importModelParametersFromSsv()
{
    //Let user choose SSV file
    QString openLocation = gpConfig->getStringSetting(cfg::dir::parameterimport);
    if (openLocation.isEmpty()) {
        openLocation = gpConfig->getStringSetting(cfg::dir::loadmodel);
    }
    QString parameterFile = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Parameters from SSV File"),
                                                 openLocation,
                                                 tr("System Structure Parameter Values (*.ssv)"));
    if(!parameterFile.isEmpty()) {
        gpConfig->setStringSetting(cfg::dir::parameterimport,  QFileInfo(parameterFile).absolutePath());
    }

    QList<SsvParameter> ssvParameters;
    readFromSsv(parameterFile, ssvParameters);

    for(const auto &ssvParameter : ssvParameters) {
        //Now find corresponding parameter in model
        //ssvName = system1|system2|component.parameter (component parameters)
        //ssvName = system1|system2|parameter           (system paramters)
        QStringList systemHierarchy;
        QString componentName, parameterName;
        systemHierarchy = ssvParameter.name.split("|");
        if(systemHierarchy.last().contains(".")) {
            componentName = systemHierarchy.last().split(".")[0];  //Component parameter
            parameterName = systemHierarchy.last().split(".")[1];
        }
        else {
            parameterName = systemHierarchy.last();                 //System parameter
        }
        systemHierarchy.removeLast();
        SystemObject *pSystem = getTopLevelSystemContainer();
        for(int s=0; s<systemHierarchy.size(); ++s) {               //Recurse subsystems
            pSystem = qobject_cast<SystemObject*>(pSystem->getModelObject(systemHierarchy[s]));
            if(pSystem == nullptr) {
                gpMessageHandler->addWarningMessage("Could not find subsystem: "+systemHierarchy[s]+", ignoring all its parameters.");
                continue;
            }
        }
        if(componentName.isEmpty()) {                               //Set system parameter
            CoreParameterData parameter;
            pSystem->getParameter(parameterName, parameter);
            if(ssv::dataTypeTranslator[parameter.mType] != ssvParameter.dataType) {
                gpMessageHandler->addWarningMessage("Wrong data type for unit: \""+ssvParameter.name+"\", ignoring value. ("+ssv::dataTypeTranslator[parameter.mType]+" != "+ssvParameter.dataType+")");
            }
            else if(parameter.mType == ssv::datatype::real) {    //Handle units for "Real" type parameters only
                if(parameter.mUnit == ssvParameter.unit) {
                    pSystem->setParameterValue(parameterName, ssvParameter.value);
                }
                else {
                    QStringList quantities = gpConfig->getQuantitiesForUnit(ssvParameter.unit);
                    if(quantities.isEmpty()) {
                        gpMessageHandler->addWarningMessage("Unit is not supported by Hopsan: "+ssvParameter.unit+". Setting parameter without unit conversion.");
                        pSystem->setParameterValue(parameterName, ssvParameter.value);
                    }
                    else {
                        QString ssvQuantity, hopsanUnit, hopsanQuantity;
                        ssvQuantity = quantities[0];    //Guessing for first quantity is the best we can do
                        hopsanUnit = parameter.mUnit;
                        QStringList hopsanQuantities = gpConfig->getQuantitiesForUnit(parameter.mUnit);
                        if(parameter.mQuantity.isEmpty() && !hopsanQuantities.isEmpty()) {
                            hopsanQuantity = hopsanQuantities[0];
                        }
                        else {
                            hopsanQuantity = parameter.mQuantity;
                        }
                        UnitConverter uc1 = gpConfig->getUnitScaleUC(ssvQuantity, ssvParameter.unit);         //Converts between SSV unit and base unit
                        UnitConverter uc2 = gpConfig->getUnitScaleUC(hopsanQuantity, parameter.mUnit); //Converts between Hopsan unit and base unit
                        pSystem->setParameterValue(parameterName, uc2.convertFromBase(uc1.convertToBase(ssvParameter.value)));
                    }
                }
            }
            else {
                pSystem->setParameterValue(parameterName, ssvParameter.value);   //Ignore units for non-Real parameters
            }
        }
        else {                               //Set component parameter
            ModelObject *pComponent = pSystem->getModelObject(componentName);
            if(pComponent == nullptr) {
                gpMessageHandler->addWarningMessage("Could not find component: "+componentName+", ignoring parameter "+parameterName);
                continue;
            }
            CoreParameterData parameter;
            pComponent->getParameter(parameterName, parameter);
            if(ssv::dataTypeTranslator[parameter.mType] != ssvParameter.dataType) {
                gpMessageHandler->addWarningMessage("Wrong data type for unit: \""+ssvParameter.name+"\", ignoring value.");
            }
            else if(ssv::dataTypeTranslator[parameter.mType] == ssv::datatype::real) {
                if(parameter.mUnit == ssvParameter.unit) {
                    pComponent->setParameterValue(parameterName, ssvParameter.value);
                }
                else {
                    QStringList quantities = gpConfig->getQuantitiesForUnit(ssvParameter.unit);
                    if(quantities.isEmpty()) {
                        gpMessageHandler->addWarningMessage("Unit is not supported by Hopsan: "+ssvParameter.unit+". Setting parameter without unit conversion.");
                        pComponent->setParameterValue(parameterName, ssvParameter.value);
                    }
                    else {
                        QString ssvQuantity, hopsanQuantity;
                        ssvQuantity = quantities[0];    //Guessing for first quantity is the best we can do
                        QStringList hopsanQuantities = gpConfig->getQuantitiesForUnit(parameter.mUnit);
                        if(parameter.mQuantity.isEmpty() && !hopsanQuantities.isEmpty()) {
                            hopsanQuantity = hopsanQuantities[0];
                        }
                        else {
                            hopsanQuantity = parameter.mQuantity;
                        }
                        UnitConverter uc1 = gpConfig->getUnitScaleUC(ssvQuantity, ssvParameter.unit);         //Converts between SSV unit and base unit
                        UnitConverter uc2 = gpConfig->getUnitScaleUC(hopsanQuantity, parameter.mUnit); //Converts between Hopsan unit and base unit
                        pComponent->setParameterValue(parameterName, uc2.convertFromBase(uc1.convertToBase(ssvParameter.value)));
                    }
                }
            }
            else {
                pComponent->setParameterValue(parameterName, ssvParameter.value);   //Ignore units for non-Real parameters
            }
        }
    }
    emit getTopLevelSystemContainer()->systemParametersChanged();
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


void ModelWidget::prepareForLogDuringSimulation()
{
    mpLogDataHandler->createEmptyGeneration();
    disconnect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()));
}

void ModelWidget::cleanupAfterLogDuringSimulation()
{
    connect(this, SIGNAL(simulationFinished()), this, SLOT(collectPlotData()), Qt::UniqueConnection);
}

void ModelWidget::collectAndAppendPlotData()
{
    collectPlotData(true);
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
    setUseRemoteSimulation(tf, gpConfig->getBoolSetting(cfg::useremoteaddresserver));
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
    this->lockModelEditingFull(false);
}


//! @brief Opens current container in new tab
//! Used for opening external subsystems for editing. If current container is not external, it will
//! iterate through parent containers until it finds one that is.
void ModelWidget::openCurrentContainerInNewTab()
{
    SystemObject *pContainer = mpGraphicsView->getContainerPtr();

    while(true)
    {
        if(pContainer == mpToplevelSystem)
        {
            break;
        }
        else if(!pContainer->isExternal())
        {
            pContainer = pContainer->getParentSystemObject();
        }
        else
        {
            //mpParentModelHandler->loadModel(pContainer->getModelFileInfo().filePath());
            pContainer->setModelFile("");
            lockModelEditingFull(true);
            mpExternalSystemWarningWidget->setVisible(false);
            pContainer->setSubTypeName("");
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
    if(saveAsFlag == ExistingFile && gpConfig->getBoolSetting(cfg::autobackup))
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
    const QString filePathToModel = mpToplevelSystem->getModelFileInfo().filePath();
    QString filePathToDestinationFile = filePathToModel;
    if((filePathToModel.isEmpty()) || (saveAsFlag == NewFile))
    {
        QString filter;
        QString dialogTitle;
        QString saveDirectory;

        const QString modelDirPath = getTopLevelSystemContainer()->getModelFileInfo().absolutePath();

        if(contents==FullModel)
        {
            filter = tr("Hopsan Model Files (*.hmf)");
            dialogTitle = tr("Save Model File");
            saveDirectory = modelDirPath;
            if(saveDirectory.isEmpty()) {
                saveDirectory = gpConfig->getStringSetting(cfg::dir::loadmodel);
            }
        }
        else if(contents==ParametersOnly)
        {
            filter = tr("Hopsan Parameter Files (*.hpf)");
            dialogTitle = tr("Save Parameter File");
            saveDirectory = gpConfig->getStringSetting(cfg::dir::parameterexport);
            if(saveDirectory.isEmpty()) {
                saveDirectory = modelDirPath;
            }
        }

        filePathToDestinationFile = QFileDialog::getSaveFileName(this, dialogTitle, saveDirectory, filter);
        // Don't save anything if user presses cancel
        if(filePathToDestinationFile.isEmpty()) {
            return;
        }
    }

    bool success = saveTo(filePathToDestinationFile, contents);
    if((contents==FullModel) && success)
    {
        mpToplevelSystem->setModelFile(filePathToDestinationFile);

        // Set the tab name to the model name, effectively removing *, also mark the tab as saved
        //! @todo This needs to be rewritten in a smarter way so that we do not need a special argument and lots of ifs to do special saving of parameters, actually parameters should be saved using the CLI method (and that code should be in a shared utility library)
        QString tabName = mpToplevelSystem->getModelFileInfo().baseName();
        gpCentralTabWidget->setTabText(gpCentralTabWidget->indexOf(mpParentModelHandler->getCurrentModel()), tabName);
        gpConfig->addRecentModel(mpToplevelSystem->getModelFileInfo().filePath());
        this->setSaved(true);

        mpToplevelSystem->getCoreSystemAccessPtr()->addSearchPath(mpToplevelSystem->getModelFileInfo().absolutePath());

        gpConfig->setStringSetting(cfg::dir::loadmodel, QFileInfo(filePathToDestinationFile).absolutePath());
        mpMessageHandler->addInfoMessage("Saved model: " + filePathToDestinationFile);

        emit modelSaved(this);

        // Reload the model on the remote server when we save
#ifdef USEZMQ
        if (isRemoteCoreConnected())
        {
            loadModelRemote();
        }
#endif
    }
    else if ((contents==ParametersOnly) && success)
    {
        gpConfig->setStringSetting(cfg::dir::parameterexport, QFileInfo(filePathToDestinationFile).absolutePath());
        mpMessageHandler->addInfoMessage("Exported model parameters to: " + filePathToDestinationFile);
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
        mpToplevelSystem = new SystemObject(this, 0);
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
    QDomElement hmfRoot = loadXMLDomDocument(rModelFile, domDocument, hmf::root);
    if(hmfRoot.attribute(hmf::customtype) == "dcp") {
        mModelType = DcpModel;
    }
    if (!hmfRoot.isNull())
    {
        //! @todo check if we could load else give error message and don't attempt to load
        QDomElement systemElement = hmfRoot.firstChildElement(hmf::system);

        // Check if Format version OK
        QString hmfFormatVersion = hmfRoot.attribute(hmf::version::hmf, "0");
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
            if (hmfRoot.attribute(hmf::version::hopsangui, "0") < "2.9.0.20180816.1300")
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
        if (hmfRoot.attribute(hmf::version::hopsangui, "0") < "2.14.0") {
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
        gpMessageHandler->addErrorMessage(QString("Model does not contain a HMF root tag: ")+hmf::root);
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
        QString customType;
        if(mModelType == DcpModel) {
            customType = "dcp";
        }
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion(), customType);
    }
    else
    {
        rootElement = domDocument.createElement(hpf::root);
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

