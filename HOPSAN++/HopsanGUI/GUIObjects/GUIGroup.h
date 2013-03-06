/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   GUIGroup.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI Group class
//!
//$Id$

#ifndef GUIGROUP_H
#define GUIGROUP_H

#include<QGraphicsScene>

#include "GUIContainerObject.h"
#include "GUIPort.h"
#include "common.h"
#include <assert.h>

class ProjectTabWidget;
class GraphicsView;
class Connector;
class Port;
class Component;

class GroupContainer : public ContainerObject
{
    Q_OBJECT
public:
    GroupContainer(QPointF position, qreal rotation, const ModelObjectAppearance *pAppearanceData, ContainerObject *pParentContainer);
    ~GroupContainer();

//    void setContents(CopyStack *pCopyStack);
//    QString getName();

    void setName(QString newName);

    enum { Type = GroupContainerType };
    int type() const;

    QString getTypeName();

    CoreSystemAccess* getCoreSystemAccessPtr();

protected:
    void addExternalContainerPortObject(ModelObject *pModelObject);

    QList<ModelObject*> mGUICompList;
    QList<Connector*> mGUIConnList;
    QList<Connector*> mGUITransitConnList;

    //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

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


//class GUIGroupPort : public GUIModelObject
//{
//    Q_OBJECT
//public:
//    GUIGroupPort(ModelObjectAppearance* pAppearanceData, QPoint position, GUIContainerObject *system, QGraphicsItem *parent = 0);
//    QString getTypeName();
//    void setName(QString newName);

//    void setOuterGuiPort(GUIPort *pPort);

//    enum { Type = GUIGROUPPORT };
//    int type() const;

//private:
//    GUIPort *mpGuiPort;
//    GUIPort *mpOuterGuiPort;
//};

#endif // GUIGROUP_H
