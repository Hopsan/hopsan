//!
//! @file   ProjectTabWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-02-05
//!
//! @brief Contains classes for Project Tabs
//!
//$Id$

#ifndef PROJECTTABWIDGET_H
#define PROJECTTABWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTabWidget>
#include <map>

#include "MessageWidget.h"
#include "GUIComponent.h"


class GUIPort;
class GUIConnector;
class ProjectTab;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicsScene(ProjectTab *parent = 0);
    qreal TestVar;

    ProjectTab *mpParentProjectTab;
};


class HopsanEssentials; //Forward declaration
class ComponentSystem;
class GUIComponent;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(HopsanEssentials *hopsan, ComponentSystem *model, ProjectTab *parent = 0);
    ~GraphicsView();
    bool mIsCreatingConnector;
    GUIComponent *getComponent(QString name);
    GUIConnector *getTempConnector();

    ProjectTab *mpParentProjectTab;

signals:
    void draggingSomething();
    void keyPressDelete();
    void viewClicked();
    void checkMessages();

public slots:
    //void addComponent(QStringList parameterData, QPoint position);
    void addComponent(QString parameterType, QPoint position, QString name=QString());
    void deleteComponent(QString componentName);

    void addConnector(GUIPort *pPort);
    void removeConnection(GUIConnector* pConnector);

    //QByteArray *data;
    //QDataStream *stream;
    //QString *text;

protected:
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    GUIConnector *mpTempConnector;
    HopsanEssentials *mpHopsan;
    ComponentSystem *mpModel;
    //! @todo QMap no good means problem if we rename need to loop around the rename like in coore
    QMap<QString, GUIComponent *> mComponentMap;
};


class ProjectTabWidget; //Forward declaration

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(ProjectTabWidget *parent = 0);
    bool mIsSaved;
    ProjectTabWidget *mpParentProjectTabWidget;
    ComponentSystem *mpComponentSystem;

    GraphicsView *mpGraphicsView;
    GraphicsScene *mpGraphicsScene;

public slots:
    void hasChanged();

signals:
    void checkMessages();

};


class MainWindow;

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    HopsanEssentials *mpHopsan;

    size_t mNumberOfUntitledTabs;

public slots:
    void addProjectTab(ProjectTab *projectTab, QString tabName="Untitled");
    void addNewProjectTab(QString tabName="Untitled");
    void saveProjectTab();
    void saveProjectTab(int index);
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void simulateCurrent();
    void loadModel();

signals:
    void checkMessages();

};

#endif // PROJECTTABWIDGET_H
