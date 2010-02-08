#ifndef PROJECTTAB_H
#define PROJECTTAB_H

#include <QGraphicsView>
#include <QDragMoveEvent>
#include <QtGui/QWidget>
#include <QGraphicsScene>
#include <QGraphicsWidget>

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    GraphicsScene();
};


class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    GraphicsView(QWidget *parent = 0);

    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
};


class Component : public QGraphicsWidget
{
    Q_OBJECT

public:
    Component(QString componentName, QGraphicsItem *parent = 0);

};


class ProjectTab : public QWidget
{
    Q_OBJECT

public:
    ProjectTab(QWidget *parent = 0);

    bool isSaved;

};

#endif // PROJECTTAB_H
