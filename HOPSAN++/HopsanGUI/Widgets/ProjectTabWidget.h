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
#include "../CoreAccess.h"
#include "QuickNavigationWidget.h"

//Forward declaration
class QGraphicsScene;
class GraphicsView;
class ProjectTab;
class GUISystem;

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);
    ProjectTab *getCurrentTab();
    ProjectTab *getTab(int index);
    GUISystem *getCurrentTopLevelSystem();
    GUIContainerObject *getContainer(int index);
    GUIContainerObject *getCurrentContainer();
    GUISystem *getSystem(int index);

    //Public member variables

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void loadModel();
    void loadModel(QAction *action);
    void loadModel(QString modelFileName);
    void tabChanged();
    void saveCurrentModelToWrappedCode();
    void createSimulinkWrapperFromCurrentModel();

signals:
    void checkMessages();
    void newTabAdded();

private:
    size_t mNumberOfUntitledTabs;
};

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(ProjectTabWidget *parent = 0);
    ~ProjectTab();

    bool isSaved();
    void setSaved(bool value);
    void hasChanged();

        //Public member variables
    //! @todo these should not be public
    GUISystem *mpSystem;
    ProjectTabWidget *mpParentProjectTabWidget;
    GraphicsView *mpGraphicsView;
    QuickNavigationWidget *mpQuickNavigationWidget;

    int mLastSimulationTime;

public slots:
    bool simulate();
    void save();
    void saveAs();

private slots:
    void collectPlotData();

signals:
    void checkMessages();
    void simulationFinished();

private:
    bool mIsSaved;
    void saveModel(saveTarget saveAsFlag);
};

#endif // PROJECTTABWIDGET_H
