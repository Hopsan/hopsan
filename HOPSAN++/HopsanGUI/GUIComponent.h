//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
//#include <QGraphicsSvgItem>
//#include <QGraphicsTextItem>
//#include <QWidget>
#include <QGraphicsView>
//#include "GUIConnector.h"
#include <vector>
//#include <QGraphicsItem>


class GUIConnector;
class QGraphicsSvgItem;
class GUIComponentTextItem;

class GUIComponent : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIComponent(const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIComponent();
    QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);

    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );


protected:
    //virtual void moveEvent(QGraphicsItem::GraphicsItemChange);
    virtual void moveEvent(QGraphicsSceneMoveEvent *event);

signals:
    void componentMoved();

public slots:
     void deleteComponent();
     void deselect();

private:
    QGraphicsSvgItem *icon;
    GUIComponentTextItem *text;
    QGraphicsView *mpParentView;
    QGraphicsRectItem *mpSelectionBox;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component

private slots:
    void fixTextPosition(QGraphicsSceneMouseEvent * event);

};


class GUIComponentTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    GUIComponentTextItem(const QString &text, QGraphicsItem *parent = 0);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

signals:
    void textMoved(QGraphicsSceneMouseEvent * event);

};

#endif // GUICOMPONENT_H
