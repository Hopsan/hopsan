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


class GUIPort;
class GUIConnector;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicsScene(QObject *parent = 0);
    qreal TestVar;

};


class HopsanEssentials; //Forward declaration
class ComponentSystem;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(HopsanEssentials *hopsan, ComponentSystem *model, QWidget *parent = 0);
    ~GraphicsView();
    bool creatingConnector;

signals:
    void draggingSomething();
    void keyPressDelete();
    void viewClicked();

public slots:
    void addComponent(QString iconDir, QString componentTypeName, QPoint position);
    void addConnector(GUIPort *rect);
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
};


//class Component : public QGraphicsWidget
//{
//    Q_OBJECT
//
//public:
//    Component(QString componentName, QGraphicsItem *parent = 0);
//
//};


class ProjectTabWidget; //Forward declaration

class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(QWidget *parent = 0);

    bool isSaved;

    ProjectTabWidget *mpTabContainer;

    ComponentSystem *mpModel;

public slots:
    void hasChanged();

};


class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(QWidget *parent = 0);

    HopsanEssentials *mpHopsan;

    size_t mNumberOfUntitledTabs;

public slots:
    void addProjectTab();
    void saveProjectTab();
    void saveProjectTab(int index);
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();
    void simulateCurrent();

};

#endif // PROJECTTABWIDGET_H
