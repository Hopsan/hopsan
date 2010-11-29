//$Id$

#ifndef GUIGROUP_H
#define GUIGROUP_H

#include<QGraphicsScene>

#include "GUIContainerObject.h"
#include "../common.h"
#include <assert.h>

class ProjectTabWidget;
class GraphicsView;
class GUIConnector;
class GUIPort;
class GUIComponent;

class GUIGroup : public GUIContainerObject
{
    Q_OBJECT
public:
    GUIGroup(QPoint position, qreal rotation, const GUIModelObjectAppearance *pAppearanceData, GUIContainerObject *system, QGraphicsItem *parent = 0);
    ~GUIGroup();

    void setContents(CopyStack *pCopyStack);
//    QString getName();
//    void setName(QString name, bool doOnlyLocalRename=false);
    void setName(QString newName);

    //void enterContainer();


    enum { Type = GUIGROUP };
    int type() const;

    QString getTypeName();

    CoreSystemAccess* getCoreSystemAccessPtr();

protected:
    //QGraphicsScene *mpParentScene;
    //QGraphicsScene *mpGroupScene;

    QList<GUIModelObject*> mGUICompList;
    QList<GUIConnector*> mGUIConnList;
    QList<GUIConnector*> mGUITransitConnList;

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

public slots:
//    void showParent();
//    void exitContainer();

private:
//    GUIPort *mpGuiPort;

//    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
//    void openComponentPropertiesDialog();
//
//    QString mComponentTypeName;
//
//    QGraphicsScene *mpGroupScene;
//
//public slots:
//     void deleteMe();
};


class GUIGroupPort : public GUIModelObject
{
    Q_OBJECT
public:
    GUIGroupPort(GUIModelObjectAppearance* pAppearanceData, QPoint position, GUIContainerObject *system, QGraphicsItem *parent = 0);
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
