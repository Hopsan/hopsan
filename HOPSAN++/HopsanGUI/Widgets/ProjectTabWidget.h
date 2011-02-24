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
