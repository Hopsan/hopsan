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
//! @file   ModelWidget.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-06-25
//!
//! @brief Contain the class for model widgets
//!
//$Id$

#ifndef MODELWIDGET_H
#define MODELWIDGET_H

//Qt includes
#include <QString>
#include <QWidget>
#include <QMutex>
#include <QMap>
#include <QDomDocument>
#include <QFile>

//Hopsan includes
#include "common.h"

//Forward declarations
class SystemObject;
class GraphicsView;
class SystemObject;
class QuickNavigationWidget;
class ModelHandler;
class CentralTabWidget;
class AnimationWidget;
class SimulationThreadHandler;
class GUIMessageHandler;
class LogDataHandler2;

#include "CoreAccess.h"
#include "RemoteCoreAccess.h"

class ModelWidget : public QWidget
{
    Q_OBJECT

    friend class ModelHandler;

public:
    ModelWidget(ModelHandler *pModelHandler, CentralTabWidget *pParentTabWidget = nullptr);
    ~ModelWidget();

    void setMessageHandler(GUIMessageHandler *pMessageHandler);

    QString getStartTime();
    QString getTimeStep();
    QString getStopTime();
    int getLastSimulationTime();
    void setLastSimulationTime(int time);

    bool saveTo(const QString &path, SaveContentsEnumT contents=FullModel);
    QDomDocument saveToDom(SaveContentsEnumT contents=FullModel);
    bool isSaved();
    void setSaved(bool value);
    void hasChanged();

    bool isEditingFullyDisabled() const;
    bool isEditingLimited() const;
    LocklevelEnumT getCurrentLockLevel() const;

    bool defineVariableAlias(const QString &rFullName, const QString &rAlias="");
    bool undefineVariableAlias(const QString &rFullName);
    QString getVariableAlias(const QString &rFullName);

    void setUseRemoteSimulation(bool useRemoteCore, bool useAddressServer);
#ifdef USEZMQ
    void setExternalRemoteCoreSimulationHandler(SharedRemoteCoreSimulationHandlerT pRSCH);
    double getSimulationProgress() const;
#endif
    bool getUseRemoteSimulationCore() const;
    bool isRemoteCoreConnected() const;

    bool loadModelRemote();
    bool loadModel(QFile &rModelFile);

    SystemObject *getTopLevelSystemContainer() const;
    SystemObject *getViewContainerObject();
    GraphicsView *getGraphicsView();
    QuickNavigationWidget *getQuickNavigationWidget();
    SimulationThreadHandler *getSimulationThreadHandler();
    QSharedPointer<LogDataHandler2> getLogDataHandler();
    void setLogDataHandler(QSharedPointer<LogDataHandler2> pHandler);

    ModelHandler *mpParentModelHandler;
    GraphicsView *mpGraphicsView;
    AnimationWidget *mpAnimationWidget;

public slots:
    void setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime);
    bool simulate_nonblocking();
    bool simulate_blocking();
    bool startRealtimeSimulation(const double realtimeFactor);
    void stopRealtimeSimulation();
    void save();
    void saveAs();
    void exportModelParametersToHpf();
    void exportModelParametersToSsv();
    void importModelParametersFromHpf(QString parameterFile="");
    void importModelParametersFromSsv();
    void exportSimulationStates();
    void handleSystemLock(bool isExternal, bool hasLocalLock);
    void lockModelEditingFull(bool lock);
    void lockModelEditingLimited(bool lock);
    void openAnimation();
    void prepareForLogDuringSimulation();
    void cleanupAfterLogDuringSimulation();
    void collectAndAppendPlotData();
    void collectPlotData(bool overWriteGeneration=false);
    void setUseRemoteSimulation(bool tf);
    void revertModel();

private slots:
    void openCurrentContainerInNewTab();
    void closeAnimation();
    void unlockSimulateMutex();

signals:
    void simulationTimeChanged(QString start, QString ts, QString stop);
    void checkMessages();
    void simulationFinished();
    void simulationStepFinished();
    void modelSaved(ModelWidget*);
    void aliasChanged(QString fullName, QString alias);
    void quantityChanged(QString fullName, QString quantity);
    void modelChanged(ModelWidget *);

private:
    void saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents=FullModel);
    void createOrDestroyToplevelSystem(bool recreate);

    QString mStartTime, mStopTime;
    int mLastSimulationTime;

    bool mIsSaved;
    bool mDoNotifyChangeToTabWidget=true;
    int mLimitedLockModelEditingCounter=0;
    int mFullLockModelEditingCounter=0;

    QuickNavigationWidget *mpQuickNavigationWidget;
    QWidget *mpExternalSystemWarningWidget;
    QWidget *mpLockedSystemWarningWidget;

    SystemObject *mpToplevelSystem=nullptr;
    QSharedPointer<LogDataHandler2> mpLogDataHandler;
    GUIMessageHandler *mpMessageHandler;
    SimulationThreadHandler *mpSimulationThreadHandler;
#ifdef USEZMQ
    SharedRemoteCoreSimulationHandlerT mpLocalRemoteCoreSimulationHandler;
    SharedRemoteCoreSimulationHandlerT mpExternalRemoteCoreSimulationHandler;
    SharedRemoteCoreSimulationHandlerT chooseRemoteCoreSimulationHandler() const;
    double mSimulationProgress;
#endif
    // Remote collected data
    QVector<RemoteResultVariable> mRemoteResultVariables;
    QMutex mSimulateMutex;
};


#endif // MODELWIDGET_H
