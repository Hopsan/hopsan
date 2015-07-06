/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

//Hopsan includes
#include "common.h"

//Forward declarations
class SystemContainer;
class GraphicsView;
class ContainerObject;
class QuickNavigationWidget;
class ModelHandler;
class CentralTabWidget;
class AnimationWidget;
class SimulationThreadHandler;
class GUIMessageHandler;
class LogDataHandler2;

#include "CoreAccess.h"
#ifdef USEZMQ
#include "RemoteCoreAccess.h"
#endif

class ModelWidget : public QWidget
{
    Q_OBJECT

public:
    ModelWidget(ModelHandler *pModelHandler, CentralTabWidget *pParentTabWidget = nullptr);
    ~ModelWidget();

    void setMessageHandler(GUIMessageHandler *pMessageHandler);

    QString getStartTime();
    QString getTimeStep();
    QString getStopTime();
    int getLastSimulationTime();
    void setLastSimulationTime(int time);

    bool saveTo(QString path, SaveContentsEnumT contents=FullModel);
    QDomDocument saveToDom(SaveContentsEnumT contents=FullModel);
    bool isSaved();
    void setSaved(bool value);
    void hasChanged();

    bool isEditingFullyDisabled();
    bool isEditingLimited();

    bool defineVariableAlias(const QString &rFullName, const QString &rAlias="");
    bool undefineVariableAlias(const QString &rFullName);
    QString getVariableAlias(const QString &rFullName);

    void setUseRemoteSimulationCore(bool tf, bool useDispatch);
#ifdef USEZMQ
    void setUseRemoteSimulationCore(SharedRemoteCoreSimulationHandlerT pRSCH);
    double getSimulationProgress() const;
#endif
    bool getUseRemoteSimulationCore() const;
    bool isRemoteCoreConnected() const;
    bool isExternalRemoteCoreConnected() const;
    bool loadModelRemote();

    SystemContainer *getTopLevelSystemContainer() const;
    ContainerObject *getViewContainerObject();
    GraphicsView *getGraphicsView();
    QuickNavigationWidget *getQuickNavigationWidget();
    SimulationThreadHandler *getSimulationThreadHandler();
    LogDataHandler2 *getLogDataHandler();

    ModelHandler *mpParentModelHandler;
    GraphicsView *mpGraphicsView;
    AnimationWidget *mpAnimationWidget;

public slots:
    void setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime);
    bool simulate_nonblocking();
    bool simulate_blocking();
    void startCoSimulation();
    void save();
    void saveAs();
    void exportModelParameters();
    void setExternalSystem(bool value);
    void lockModelEditingFull(bool lock);
    void lockModelEditingLimited(bool lock);
    void openAnimation();
    void simulateModelica();
    void collectPlotData(bool overWriteGeneration=false);
    void setUseRemoteSimulationCore(bool tf);

private slots:
    void openCurrentContainerInNewTab();
    void closeAnimation();
    void unlockSimulateMutex();

signals:
    void simulationTimeChanged(QString start, QString ts, QString stop);
    void checkMessages();
    void simulationFinished();
    void modelSaved(ModelWidget*);
    void aliasChanged(QString fullName, QString alias);

private:
    void saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents=FullModel);

    QString mStartTime, mStopTime;
    int mLastSimulationTime;

    bool mIsSaved;
    int mLimitedLockModelEditingCounter=0;
    int mFullLockModelEditingCounter=0;
    bool mUseRemoteCore=false;
    bool mUseRemoteCoreAddressServer=false;

    QuickNavigationWidget *mpQuickNavigationWidget;
    QWidget *mpExternalSystemWarningWidget;

    SystemContainer *mpToplevelSystem;
    LogDataHandler2 *mpLogDataHandler;
    GUIMessageHandler *mpMessageHandler;
    SimulationThreadHandler *mpSimulationThreadHandler;
#ifdef USEZMQ
    SharedRemoteCoreSimulationHandlerT mpRemoteCoreSimulationHandler;
    SharedRemoteCoreSimulationHandlerT mpExternalRemoteCoreSimulationHandler;
    double mSimulationProgress;
#endif
    QMutex mSimulateMutex;

    // Remote collected data
    std::vector<std::string> mRemoteLogNames; std::vector<double> mRemoteLogData;
};


#endif // MODELWIDGET_H
