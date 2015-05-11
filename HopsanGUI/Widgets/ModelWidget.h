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
    bool isSaved();
    void setSaved(bool value);
    void hasChanged();

    bool isEditingEnabled();

    bool defineVariableAlias(const QString &rFullName, const QString &rAlias="");
    bool undefineVariableAlias(const QString &rFullName);
    QString getVariableAlias(const QString &rFullName);

    void setUseRemoteSimulationCore(bool tf, bool useDispatch);
#ifdef USEZMQ
    void setUseRemoteSimulationCore(SharedRemoteCoreSimulationHandlerT pRSCH);
#endif
    bool getUseRemoteSimulationCore() const;
    bool isRemoteCoreConnected() const;
    bool isExternalRemoteCoreConnected() const;
    bool loadModelRemote();

    SystemContainer *getTopLevelSystemContainer();
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
    void setEditingEnabled(bool value);
    void openAnimation();
    void lockTab(bool locked);
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
    QDomDocument saveToDom(SaveContentsEnumT contents=FullModel);
    void saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents=FullModel);

    QString mStartTime, mStopTime;
    int mLastSimulationTime;

    bool mIsSaved;
    bool mEditingEnabled;
    bool mUseRemoteCore=false;
    bool mUseRemoteCoreAddressServer=false;

    QuickNavigationWidget *mpQuickNavigationWidget;
    QWidget *mpExternalSystemWidget;

    SystemContainer *mpToplevelSystem;
    LogDataHandler2 *mpLogDataHandler;
    GUIMessageHandler *mpMessageHandler;
    SimulationThreadHandler *mpSimulationThreadHandler;
#ifdef USEZMQ
    SharedRemoteCoreSimulationHandlerT mpRemoteCoreSimulationHandler;
    SharedRemoteCoreSimulationHandlerT mpExternalRemoteCoreSimulationHandler;
#endif
    QMutex mSimulateMutex;

    // Remote collected data
    std::vector<std::string> mRemoteLogNames; std::vector<double> mRemoteLogData;
};


#endif // MODELWIDGET_H
