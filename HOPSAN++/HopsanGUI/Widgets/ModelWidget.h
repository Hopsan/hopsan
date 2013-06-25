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


class ModelWidget : public QWidget
{
    Q_OBJECT

public:
    ModelWidget(ModelHandler *modelHandler, CentralTabWidget *parent = 0);
    ~ModelWidget();

    void setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime);
    void setToolBarSimulationTimeParametersFromTab();
    QString getStartTime();
    QString getTimeStep();
    QString getStopTime();
    bool isSaved();
    void setSaved(bool value);
    void hasChanged();
    SystemContainer *getTopLevelSystemContainer();
    ContainerObject *getViewContainerObject();
    GraphicsView *getGraphicsView();
    QuickNavigationWidget *getQuickNavigationWidget();
    void setLastSimulationTime(int time);
    int getLastSimulationTime();
    bool isEditingEnabled();
    ModelHandler *mpParentModelHandler;
    GraphicsView *mpGraphicsView;
    AnimationWidget *mpAnimationWidget;
    SimulationThreadHandler *mpSimulationThreadHandler;

public slots:
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


private slots:
    void collectPlotData();
    void openCurrentContainerInNewTab();
    void closeAnimation();
    void unlockSimulateMutex();

signals:
    void checkMessages();
    void simulationFinished();

private:
    void saveModel(SaveTargetEnumT saveAsFlag, SaveContentsEnumT contents=FullModel);

    QString mStartTime, mStopTime;

    bool mIsSaved;
    SystemContainer *mpToplevelSystem;
    QuickNavigationWidget *mpQuickNavigationWidget;
    QWidget *mpExternalSystemWidget;
    int mLastSimulationTime;
    bool mEditingEnabled;

    QMutex mSimulateMutex;
};

#endif // MODELWIDGET_H
