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
//! @file   ProjectTabWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contain classes for Project Tabs
//!
//$Id$

#ifndef PROJECTTABWIDGET_H
#define PROJECTTABWIDGET_H

#include <QTabWidget>
#include "CoreAccess.h"
#include "QuickNavigationWidget.h"
#include "AnimationWidget.h"
#include "common.h"

//Forward declaration
class QGraphicsScene;
class GraphicsView;
class ProjectTab;
class SystemContainer;
class AnimationWidget;
class SimulationThreadHandler;


class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);
    ~ProjectTabWidget();
    ProjectTab *getCurrentTab();
    ProjectTab *getTab(int index);
    SystemContainer *getCurrentTopLevelSystem();
    ContainerObject *getContainer(int index);
    ContainerObject *getCurrentContainer();
    SystemContainer *getSystem(int index);

    //Public member variables

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void loadModel();
    void loadModel(QAction *action);
    void loadModel(QString modelFileName, bool ignoreAlreadyOpen=false);
    void tabChanged();
    void saveCurrentModelToWrappedCode();
    void createFMUFromCurrentModel();
    void createSimulinkWrapperFromCurrentModel();
    void showLosses(bool show);
    void measureSimulationTime();
    bool simulateAllOpenModels_nonblocking(bool modelsHaveNotChanged=false);
    bool simulateAllOpenModels_blocking(bool modelsHaveNotChanged=false);
    bool simulateAllOpenModels_old(bool modelsHaveNotChanged=false);
    void setCurrentTopLevelSimulationTimeParameters(const QString startTime, const QString timeStep, const QString stopTime);
    void openAnimation();

signals:
    void checkMessages();
    void newTabAdded();
    void simulationFinished();

private:
    void setToolBarSimulationTimeParametersFromSystem(SystemContainer *pSystem);
    size_t mNumberOfUntitledTabs;
    SimulationThreadHandler *mpSimulationThreadHandler;
};

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(ProjectTabWidget *parent = 0);
    ~ProjectTab();

    void setTopLevelSimulationTime(const QString startTime, const QString timeStep, const QString stopTime);
    void setToolBarSimulationTimeParametersFromTab();
    QString getStartTime();
    QString getTimeStep();
    QString getStopTime();
    bool isSaved();
    void setSaved(bool value);
    void hasChanged();
    SystemContainer *getTopLevelSystem();
    GraphicsView *getGraphicsView();
    QuickNavigationWidget *getQuickNavigationWidget();
    void setLastSimulationTime(int time);
    int getLastSimulationTime();
    bool isEditingEnabled();
    ProjectTabWidget *mpParentProjectTabWidget;
    GraphicsView *mpGraphicsView;
    AnimationWidget *mpAnimationWidget;

public slots:
    bool simulate_nonblocking();
    bool simulate_blocking();
    bool simulate_old();
    void save();
    void saveAs();
    void ExportModel();
    void setExternalSystem(bool value);
    void setEditingEnabled(bool value);
    void openAnimation();


private slots:
    void collectPlotData();
    void openCurrentContainerInNewTab();
    void closeAnimation();

signals:
    void checkMessages();
    void simulationFinished();

private:
    void saveModel(saveTarget saveAsFlag);

    QString mStartTime, mStopTime;
    SimulationThreadHandler *mpSimulationThreadHandler;

    bool mIsSaved;
    SystemContainer *mpSystem;
    QuickNavigationWidget *mpQuickNavigationWidget;
    QWidget *mpExternalSystemWidget;
    int mLastSimulationTime;
    bool mEditingEnabled;
};

#endif // PROJECTTABWIDGET_H
