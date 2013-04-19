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
//! @file   GUIObject.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIObject class (The baseclass for all objects)
//!
//$Id$

#ifndef GUIOBJECT_H
#define GUIOBJECT_H

#include <QGraphicsWidget>
#include <QObject>
#include <QPen>

#include "common.h"

class QDomElement;
class WorkspaceObjectSelectionBox;
class ContainerObject;

enum GUIObjectEnumT {WorkspaceObjectType=QGraphicsItem::UserType+1, ModelObjectType, ContainerObjectType, SystemContainerType, ComponentType, ScopeComponentType, ContainerPortType, GroupContainerType, AnimatedObjectType};

class WorkspaceObject : public QGraphicsWidget
{
    Q_OBJECT

public:
    WorkspaceObject(QPointF pos, qreal rot, SelectionStatusEnumT=Deselected, ContainerObject *pParentContainer=0, QGraphicsItem *pParent=0);

    virtual void setParentContainerObject(ContainerObject *pParentContainer);
    virtual ContainerObject *getParentContainerObject();

    //Position methods
    QPointF getCenterPos();
    void setCenterPos(const QPointF cpos);

    //Load and save methods
    virtual void saveToDomElement(QDomElement &rDomElement);
    virtual void loadFromHMF(QString modelFilePath=QString());

    //Other methods
    bool isFlipped();

    enum { Type = WorkspaceObjectType };
    int type() const;

public slots:
    virtual void flipVertical(UndoStatusEnumT /*undoSettings = UNDO*/){} //!< @todo nothing for now
    virtual void flipHorizontal(UndoStatusEnumT /*undoSettings = UNDO*/){}  //!< @todo nothing for now
    virtual void rotate(qreal angle, UndoStatusEnumT undoSettings=Undo);
    void rotate90cw(UndoStatusEnumT undoSettings=Undo);
    void rotate90ccw(UndoStatusEnumT undoSettings=Undo);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void deselect();
    void select();

    virtual void deleteMe();

    void updateOldPos();

signals:
    void objectMoved();
    void objectDeleted();
    void objectSelected();

protected:
    //Reimplemented Qt methods
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    //Protected members
    ContainerObject *mpParentContainerObject;
    QString mHmfTagName;
    bool mIsFlipped;
    WorkspaceObjectSelectionBox *mpSelectionBox;
    QPointF mOldPos;
};



class WorkspaceObjectSelectionBox : public QGraphicsItemGroup
{
public:
    WorkspaceObjectSelectionBox(qreal x1, qreal y1, qreal x2, qreal y2, QPen activePen, QPen hoverPen, WorkspaceObject *parent = 0);
    void setSize(qreal x1, qreal y1, qreal x2, qreal y2);

    //Selection/active methods
    void setActive();
    void setPassive();
    void setHovered();

protected:
    //Protected members
    QVector<QGraphicsLineItem*> mLines;

    QPen mActivePen;
    QPen mHoverPen;
};



#endif // GUIOBJECT_H
