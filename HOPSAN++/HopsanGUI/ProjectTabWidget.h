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
#include <QGraphicsWidget>
#include <QTabWidget>
#include <QStringList>
#include <QGraphicsTextItem>
#include "graphicsrectitem.h"
#include "GUIConnector.h"


class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicsScene(QObject *parent = 0);
    qreal TestVar;
};


class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(QWidget *parent = 0);
    ~GraphicsView();
    bool creatingConnector;
    GUIConnector *line;

public slots:
    void addConnector(GraphicsRectItem *rect);

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

    ProjectTabWidget *pTabContainer;

public slots:
    void hasChanged();

};


class HopsanEssentials; //Forward declaration

class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(QWidget *parent = 0);

    HopsanEssentials *Hopsan;

    size_t numberOfUntitledTabs;

public slots:
    void addProjectTab();
    void saveProjectTab();
    void saveProjectTab(int index);
    bool closeProjectTab(int index);
    bool closeAllProjectTabs();

};

#endif // PROJECTTABWIDGET_H
