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
#include "GUIComponentSelectionBox.h"
#include "ProjectTabWidget.h"

class GraphicsScene;
class GraphicsView;
class GUIConnector;
class QGraphicsSvgItem;
class GUIComponentNameTextItem;
class HopsanEssentials;
class Component;
class GUIComponentSelectionBox;
class GUIPort;

class GUIComponent : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIComponent(HopsanEssentials *hopsan, QStringList parameterData, QPoint position, GraphicsScene *scene, QGraphicsItem *parent = 0);
    //GUIComponent(HopsanEssentials *hopsan, const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIComponent();
    //QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);
    void refreshName();
    void showPorts(bool visible);
    GUIPort *getPort(int number);

    GraphicsScene *mpParentGraphicsScene;
    GraphicsView *mpParentGraphicsView;

    //Core interaction
    Component *mpCoreComponent;
    //

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

signals:
    void componentMoved();
    void componentDeleted();

public slots:
     void deleteComponent();

private:
    QGraphicsSvgItem *mpIcon;
    GUIComponentNameTextItem *mpNameText;
    GUIComponentSelectionBox *mpSelectionBox;
    double mTextOffset;
    QGraphicsLineItem *mpTempLine;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component

    QList<GUIPort*> mPortListPtrs;

private slots:
    void fixTextPosition(QPointF pos);

};


class GUIComponentNameTextItem : public QGraphicsTextItem
{
    Q_OBJECT
private:
    Component* mpCoreComponent;

public:
    GUIComponentNameTextItem(Component* pCoreComponent, QGraphicsItem *parent = 0);
    void refreshName();

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusOutEvent(QFocusEvent *event);

signals:
    void textMoved(QPointF pos);

};

#endif // GUICOMPONENT_H
