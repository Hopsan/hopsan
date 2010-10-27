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
#include "CoreSystemAccess.h"
#include "GUIObject.h"

//Forward declaration
class GraphicsScene;
class GraphicsView;
class MainWindow;
class ProjectTab;
class GUISystem;

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);
    MainWindow *mpParentMainWindow;
    ProjectTab *getCurrentTab();
    ProjectTab *getTab(int index);
    GUISystem *getCurrentSystem();
    GUISystem *getSystem(int index);
    QString *mpCopyData;

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void loadModel();
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
    GUISystem *mpSystem;
    ProjectTabWidget *mpParentProjectTabWidget;
    GraphicsView *mpGraphicsView;

    bool isSaved();
    void setSaved(bool value);
    void hasChanged();

public slots:
    bool simulate();
    void save();
    void saveAs();

signals:
    void checkMessages();
    void simulationFinished();

private:
    bool mIsSaved;
    void saveModel(saveTarget saveAsFlag);
};

#endif // PROJECTTABWIDGET_H
