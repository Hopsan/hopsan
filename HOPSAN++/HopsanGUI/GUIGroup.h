#ifndef GUIGROUP_H
#define GUIGROUP_H

//#include <QGraphicsWidget>
//#include <QObject>
//#include <QGraphicsSvgItem>
//#include <QPen>
//#include <QtXml>

#include "common.h"

#include "GUIModelObjectAppearance.h"
#include <assert.h>
#include "GUIContainerObject.h"

class ProjectTabWidget;
class GraphicsScene;
class GraphicsView;
class GUIConnector;
//class GUIModelObjectDisplayName;
//class Component;
//class GUIObjectSelectionBox;
class GUIPort;
class GUISystem;
class GUIComponent;
//class GUIContainerObject;
//! @todo clean up these includes and forward declarations

class GUIGroup : public GUIContainerObject
{
    Q_OBJECT
public:
    GUIGroup(QList<QGraphicsItem*> compList, GUIModelObjectAppearance* pAppearanceData, GUISystem *system, QGraphicsItem *parent = 0);
    ~GUIGroup();
//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);

    enum { Type = GUIGROUP };
    int type() const;

    QString getTypeName();

protected:
    GraphicsScene *mpParentScene;
    GraphicsScene *mpGroupScene;

    QList<GUIModelObject*> mGUICompList;
    QList<GUIConnector*> mGUIConnList;
    QList<GUIConnector*> mGUITransitConnList;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void showParent();

private:
//    GUIPort *mpGuiPort;

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void openParameterDialog();
//
//    QString mComponentTypeName;
//
//    GraphicsScene *mpGroupScene;
//
//public slots:
//     void deleteMe();
};


class GUIGroupPort : public GUIModelObject
{
    Q_OBJECT
public:
    GUIGroupPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, GUISystem *system, QGraphicsItem *parent = 0);
    QString getTypeName();
    void setName(QString newName);

    void setOuterGuiPort(GUIPort *pPort);

    enum { Type = GUIGROUPPORT };
    int type() const;

private:
    GUIPort *mpGuiPort;
    GUIPort *mpOuterGuiPort;
};

#endif // GUIGROUP_H
