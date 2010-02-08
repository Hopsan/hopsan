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

//#include <QGraphicsView>
//#include <QGraphicsScene>
//#include <QDragMoveEvent>
#include <QWidget>
#include <QGraphicsWidget>
#include <QTabWidget>

//class GraphicsScene : public QGraphicsScene
//{
//    Q_OBJECT
//
//public:
//    GraphicsScene();
//};
//
//
//class GraphicsView : public QGraphicsView
//{
//    Q_OBJECT
//
//public:
//    GraphicsView(QWidget *parent = 0);
//
//    void dragMoveEvent(QDragMoveEvent *event);
//    void dropEvent(QDropEvent *event);
//};
//
//
//class Component : public QGraphicsWidget
//{
//    Q_OBJECT
//
//public:
//    Component(QString componentName, QGraphicsItem *parent = 0);
//
//};


class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(QWidget *parent = 0);

    bool isSaved;

};


class ProjectTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    ProjectTabWidget(QWidget *parent = 0);

    size_t numberOfUntitledTabs;

public slots:
    void addProjectTab();
    void saveProjectTab();
    void closeProjectTab(int index);

};

#endif // PROJECTTABWIDGET_H
